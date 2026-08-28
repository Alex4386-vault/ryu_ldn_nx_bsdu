#include "lsp_capture.hpp"

#include <stratosphere.hpp>

#include <array>
#include <unordered_map>

namespace lsp_capture {

namespace detail {

constexpr const char *kCapturePath = "sdmc:/lsp_mitm/capture.bin";
constexpr const char *kCaptureDir  = "sdmc:/lsp_mitm";

constexpr uint16_t kVideoBase = 5016;
constexpr uint16_t kCtrlBase  = 5032;
constexpr uint16_t kTelemBase = 5116;
constexpr uint16_t kSlotMax   = 8;

constexpr size_t kMaxFdMap = 256;

ams::os::SdkMutex g_mu;
ams::fs::FileHandle g_file{};
bool g_open = false;
u64 g_file_off = 0;

std::unordered_map<s32, uint16_t> g_fd_port;
std::unordered_map<s32, Proto> g_fd_proto;

bool PortInRange(uint16_t port) {
    if (port == 5201 || port == 5202) {
        return true;
    }
    for (uint16_t slot = 0; slot <= kSlotMax; ++slot) {
        if (port == kVideoBase + slot || port == kCtrlBase + slot ||
            port == kTelemBase + slot) {
            return true;
        }
    }
    return false;
}

bool EnsureOpen() {
    if (g_open) {
        return true;
    }

    if (R_FAILED(ams::fs::CreateDirectory(kCaptureDir))) {
        // ok if exists
    }

    if (R_FAILED(ams::fs::CreateFile(kCapturePath, 0))) {
        // ok if exists
    }

    if (R_FAILED(ams::fs::OpenFile(std::addressof(g_file), kCapturePath,
                                   ams::fs::OpenMode_Write | ams::fs::OpenMode_AllowAppend))) {
        return false;
    }

    s64 sz = 0;
    if (R_SUCCEEDED(ams::fs::GetFileSize(&sz, g_file))) {
        g_file_off = static_cast<u64>(sz);
    }

    g_open = true;
    return true;
}

void WriteRecord(Dir dir, Proto proto, uint16_t port, const void *data, size_t len) {
    if (len == 0 || data == nullptr || !PortInRange(port)) {
        return;
    }
    if (len > 65535) {
        len = 65535;
    }

    std::scoped_lock lk(g_mu);
    if (!EnsureOpen()) {
        return;
    }

    std::array<u8, 12> record_hdr{};
    record_hdr[0] = static_cast<u8>(dir);
    record_hdr[1] = static_cast<u8>(proto);
    record_hdr[2] = static_cast<u8>(port >> 8);
    record_hdr[3] = static_cast<u8>(port);
    record_hdr[4] = static_cast<u8>((len >> 24) & 0xff);
    record_hdr[5] = static_cast<u8>((len >> 16) & 0xff);
    record_hdr[6] = static_cast<u8>((len >> 8) & 0xff);
    record_hdr[7] = static_cast<u8>(len);

    if (R_FAILED(ams::fs::WriteFile(g_file, static_cast<s64>(g_file_off), record_hdr.data(), record_hdr.size(),
                                    ams::fs::WriteOption::None))) {
        return;
    }
    g_file_off += record_hdr.size();

    if (R_FAILED(ams::fs::WriteFile(g_file, static_cast<s64>(g_file_off), data, len,
                                    ams::fs::WriteOption::Flush))) {
        return;
    }
    g_file_off += len;
}

uint16_t LookupPort(s32 fd) {
    auto it = g_fd_port.find(fd);
    if (it == g_fd_port.end()) {
        return 0;
    }
    return it->second;
}

void TrackFd(s32 fd, uint16_t port, Proto proto) {
    if (fd < 0) {
        return;
    }
    if (g_fd_port.size() >= kMaxFdMap) {
        g_fd_port.clear();
        g_fd_proto.clear();
    }
    g_fd_port[fd] = port;
    g_fd_proto[fd] = proto;
}

} // namespace detail

bool IsFujiPort(uint16_t port) {
    return detail::PortInRange(port);
}

void Init() {
    std::scoped_lock lk(detail::g_mu);
    detail::EnsureOpen();
}

void OnBind(s32 fd, uint16_t local_port) {
    if (!detail::PortInRange(local_port)) {
        return;
    }
    Proto proto = Proto::Udp;
    if (local_port == 5201 || local_port == 5202 ||
        (local_port >= detail::kCtrlBase && local_port <= detail::kCtrlBase + detail::kSlotMax)) {
        proto = Proto::Tcp;
    }
    std::scoped_lock lk(detail::g_mu);
    detail::TrackFd(fd, local_port, proto);
}

void OnAccept(s32 listen_fd, s32 conn_fd) {
    std::scoped_lock lk(detail::g_mu);
    const uint16_t port = detail::LookupPort(listen_fd);
    if (port == 0) {
        return;
    }
    detail::TrackFd(conn_fd, port, Proto::Tcp);
}

void LogTcp(s32 fd, Dir dir, const void *data, size_t len) {
    const uint16_t port = detail::LookupPort(fd);
    if (port == 0) {
        return;
    }
    detail::WriteRecord(dir, Proto::Tcp, port, data, len);
}

void LogUdp(s32 fd, Dir dir, const void *data, size_t len, uint16_t remote_port) {
    AMS_UNUSED(remote_port);
    const uint16_t port = detail::LookupPort(fd);
    if (port == 0) {
        return;
    }
    detail::WriteRecord(dir, Proto::Udp, port, data, len);
}

} // namespace lsp_capture
