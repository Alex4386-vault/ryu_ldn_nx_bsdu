#include "config.hpp"

namespace ryu_ldn::config {

namespace {

// Mario Kart Live: Home Circuit (base + update title IDs).
constexpr uint64_t kMklhcBase   = 0x0100ED100BA3A000ULL;
constexpr uint64_t kMklhcUpdate = 0x0100ED100BA3A800ULL;

bool MatchMklhc(uint64_t program_id) {
    return program_id == kMklhcBase || program_id == kMklhcUpdate;
}

} // namespace

void LoadWhitelist() {
    // Hard-coded; no SD file required.
}

bool IsGameInWhitelist(uint64_t program_id) {
    return MatchMklhc(program_id);
}

} // namespace ryu_ldn::config
