/*
 * lsp_bsd_mitm — log MKLHC Fuji LSP socket traffic via bsd:u MITM.
 * Built for Atmosphere 1.11.x (FW 22.x).
 */
#include <stratosphere.hpp>

#include "bsd/bsd_mitm_service.hpp"
#include "config/config.hpp"
#include "debug/log.hpp"
#include "lsp_capture.hpp"

namespace ams {

namespace {

constexpr size_t MallocBufferSize = 1_MB;
alignas(os::MemoryPageSize) constinit u8 g_malloc_buffer[MallocBufferSize];

alignas(os::MemoryPageSize) constinit u8 g_heap_memory[256_KB];
constinit lmem::HeapHandle g_heap_handle;
constinit bool g_heap_ready;

void *Allocate(size_t size) {
    if (!g_heap_ready) {
        g_heap_handle =
            lmem::CreateExpHeap(g_heap_memory, sizeof(g_heap_memory), lmem::CreateOption_ThreadSafe);
        g_heap_ready = true;
    }
    return lmem::AllocateFromExpHeap(g_heap_handle, size);
}

void Deallocate(void *p, size_t size) {
    AMS_UNUSED(size);
    if (g_heap_ready) {
        lmem::FreeToExpHeap(g_heap_handle, p);
    }
}

namespace srv {

const s32 ThreadPriority = 6;
const size_t TotalThreads = 2;
const size_t NumExtraThreads = TotalThreads - 1;
const size_t ThreadStackSize = 0x8000;

alignas(os::MemoryPageSize) u8 g_thread_stack[ThreadStackSize];
alignas(os::MemoryPageSize) u8 g_extra_thread_stacks[NumExtraThreads][ThreadStackSize];
os::ThreadType g_thread;
os::ThreadType g_extra_threads[NumExtraThreads];

struct ServerOptions {
    static constexpr size_t PointerBufferSize = 0x10000;
    static constexpr size_t MaxDomains = 0x40;
    static constexpr size_t MaxDomainObjects = 0x4000;
    static constexpr bool CanDeferInvokeRequest = false;
    static constexpr bool CanManageMitmServers = true;
};

constexpr int PortIndex_BsdMitm = 0;
constexpr sm::ServiceName BsdMitmServiceName = sm::ServiceName::Encode("bsd:u");

class ServerManager final
    : public sf::hipc::ServerManager<1, ServerOptions, 16> {
private:
    Result OnNeedsToAccept(int port_index, Server *server) override;
};

ServerManager g_server_manager;

Result ServerManager::OnNeedsToAccept(int port_index, Server *server) {
    AMS_UNUSED(port_index);

    std::shared_ptr<::Service> forward_service;
    sm::MitmProcessInfo client_info;
    server->AcknowledgeMitmSession(std::addressof(forward_service), std::addressof(client_info));

    R_RETURN(this->AcceptMitmImpl(
        server,
        sf::CreateSharedObjectEmplaced<mitm::bsd::IBsdMitmService, mitm::bsd::BsdMitmService>(
            decltype(forward_service)(forward_service), client_info),
        forward_service));
}

void LoopServerThread(void *) {
    g_server_manager.LoopProcess();
}

void ProcessForServerOnAllThreads(void *) {
    if constexpr (NumExtraThreads > 0) {
        const s32 priority = os::GetThreadCurrentPriority(os::GetCurrentThread());
        for (size_t i = 0; i < NumExtraThreads; i++) {
            R_ABORT_UNLESS(os::CreateThread(g_extra_threads + i, LoopServerThread, nullptr,
                                             g_extra_thread_stacks[i], ThreadStackSize, priority));
            os::StartThread(g_extra_threads + i);
        }
    }

    LoopServerThread(nullptr);

    if constexpr (NumExtraThreads > 0) {
        for (size_t i = 0; i < NumExtraThreads; i++) {
            os::WaitThread(g_extra_threads + i);
        }
    }
}

} // namespace srv

} // namespace

namespace init {

void InitializeSystemModule() {
    R_ABORT_UNLESS(sm::Initialize());

    fs::InitializeForSystem();
    fs::SetAllocator(Allocate, Deallocate);
    fs::SetEnabledAutoAbort(false);

    R_ABORT_UNLESS(fs::MountSdCard("sdmc"));

    ryu_ldn::config::LoadWhitelist();

    ryu_ldn::config::DebugConfig dbg{};
    dbg.enabled = true;
    dbg.log_to_file = true;
    dbg.level = 2;
    ryu_ldn::debug::g_logger.init(dbg, ryu_ldn::config::LOG_PATH);

    lsp_capture::Init();

    AMS_LOG("lsp_bsd_mitm ready (MKLHC bsd:u capture)\n");
}

void FinalizeSystemModule() {
    ryu_ldn::debug::g_logger.flush();
    fs::Unmount("sdmc");
    R_ABORT_UNLESS(sm::Finalize());
}

void Startup() {
    init::InitializeAllocator(g_malloc_buffer, sizeof(g_malloc_buffer));
}

} // namespace init

void Main() {
    os::SetThreadNamePointer(os::GetCurrentThread(), "lsp_bsd_mitm::Main");

    R_ABORT_UNLESS((srv::g_server_manager.RegisterMitmServer<mitm::bsd::BsdMitmService>(
        srv::PortIndex_BsdMitm, srv::BsdMitmServiceName)));

    R_ABORT_UNLESS(os::CreateThread(&srv::g_thread, srv::ProcessForServerOnAllThreads, nullptr,
                                    srv::g_thread_stack, srv::ThreadStackSize,
                                    srv::ThreadPriority));
    os::SetThreadNamePointer(&srv::g_thread, "lsp_bsd_mitm::Srv");
    os::StartThread(&srv::g_thread);
    os::WaitThread(&srv::g_thread);
}

} // namespace ams

void *operator new(size_t size) { return ams::Allocate(size); }
void *operator new(size_t size, const std::nothrow_t &) { return ams::Allocate(size); }
void operator delete(void *p) { ams::Deallocate(p, 0); }
void operator delete(void *p, size_t size) { ams::Deallocate(p, size); }
void *operator new[](size_t size) { return ams::Allocate(size); }
void *operator new[](size_t size, const std::nothrow_t &) { return ams::Allocate(size); }
void operator delete[](void *p) { ams::Deallocate(p, 0); }
void operator delete[](void *p, size_t size) { ams::Deallocate(p, size); }
