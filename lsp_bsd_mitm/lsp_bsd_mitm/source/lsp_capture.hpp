#pragma once

#include <cstddef>
#include <cstdint>
#include <switch/types.h>

namespace lsp_capture {

enum class Dir : uint8_t { Tx = 0, Rx = 1 };
enum class Proto : uint8_t { Tcp = 0, Udp = 1 };

void Init();
void OnBind(s32 fd, uint16_t local_port);
void OnAccept(s32 listen_fd, s32 conn_fd);
void LogTcp(s32 fd, Dir dir, const void *data, size_t len);
void LogUdp(s32 fd, Dir dir, const void *data, size_t len, uint16_t remote_port);

bool IsFujiPort(uint16_t port);

} // namespace lsp_capture
