#ifndef __FB_TRANSFER_TICKET_H__
#define __FB_TRANSFER_TICKET_H__

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

// Signed tail appended to the transfer parameter blob that the client echoes back in game C2S 0x10:
//   expire u32 | nonce u64 | tag[TAG_SIZE]
// tag = HMAC-SHA256(secret, world u32 | host u8 | blob up to and including nonce), truncated.
namespace fb::transfer_ticket {

constexpr size_t TAG_SIZE = 16;
constexpr size_t SIZE     = sizeof(uint32_t) + sizeof(uint64_t) + TAG_SIZE;

// The client stores the echoed blob in a 256-byte buffer behind a uint8 length.
constexpr size_t MAX_PARAMETER_SIZE = 255;

constexpr uint32_t TTL_SECONDS        = 30;
constexpr uint32_t CLOCK_SKEW_SECONDS = 5;
constexpr size_t   MIN_SECRET_SIZE    = 32;

using tag_type = std::array<uint8_t, TAG_SIZE>;

// Must not be called before init_config().
const std::string& secret();
uint32_t           now();
uint64_t           make_nonce();
tag_type           sign(uint32_t world, uint8_t host, const uint8_t* data, size_t size);

// Purges by steady time since insertion so a wall clock jump cannot drop a nonce that is still acceptable.
class nonce_cache
{
private:
    using clock = std::chrono::steady_clock;

    static constexpr auto PURGE_INTERVAL = std::chrono::seconds(10);
    static constexpr auto RETENTION      = std::chrono::seconds(TTL_SECONDS + CLOCK_SKEW_SECONDS * 2);

private:
    std::mutex                                      _mutex;
    std::unordered_map<uint64_t, clock::time_point> _nonces;
    clock::time_point                               _purged_at = {};

public:
    bool insert(uint64_t nonce);
};

} // namespace fb::transfer_ticket

#endif // !__FB_TRANSFER_TICKET_H__
