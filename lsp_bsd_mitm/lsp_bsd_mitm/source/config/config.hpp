#pragma once

#include <cstdint>

namespace ryu_ldn::config {

struct DebugConfig {
    bool enabled = true;
    bool log_to_file = true;
    uint32_t level = 2; // Info
};

constexpr const char *LOG_PATH = "sdmc:/lsp_mitm/lsp_bsd_mitm.log";

void LoadWhitelist();
bool IsGameInWhitelist(uint64_t program_id);

} // namespace ryu_ldn::config
