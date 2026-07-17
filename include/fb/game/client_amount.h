#ifndef __FB_GAME_CLIENT_AMOUNT_H__
#define __FB_GAME_CLIENT_AMOUNT_H__

#include <algorithm>
#include <cstdint>
#include <limits>
#include <utility>

namespace fb::game {

inline constexpr uint64_t CLIENT_AMOUNT_UNIT = 100'000'000ULL;

inline uint32_t encode_client_amount(uint64_t value)
{
    constexpr auto max32 = static_cast<uint64_t>(std::numeric_limits<uint32_t>::max());
    if (value <= max32)
        return static_cast<uint32_t>(value);

    auto scaled = (value + CLIENT_AMOUNT_UNIT - 1) / CLIENT_AMOUNT_UNIT;
    if (scaled == 0)
        return 0;
    if (scaled > max32)
        return std::numeric_limits<uint32_t>::max();
    return static_cast<uint32_t>(scaled);
}

inline bool client_pool_exceeds_u32(uint64_t current, uint64_t maximum)
{
    constexpr auto max32 = static_cast<uint64_t>(std::numeric_limits<uint32_t>::max());
    return maximum > max32 || current > max32;
}

inline std::pair<uint32_t, uint32_t> encode_client_pool(uint64_t current, uint64_t maximum)
{
    constexpr auto max32 = static_cast<uint64_t>(std::numeric_limits<uint32_t>::max());
    current              = std::min(current, maximum);

    if (maximum <= max32)
        return {static_cast<uint32_t>(current), static_cast<uint32_t>(maximum)};

    if (current <= max32)
        return {static_cast<uint32_t>(current), std::numeric_limits<uint32_t>::max()};

    auto encoded_max = encode_client_amount(maximum);
    auto encoded_cur = encode_client_amount(current);
    if (encoded_max == 0)
        encoded_max = 1;
    if (encoded_cur == 0)
        encoded_cur = 1;
    if (encoded_cur > encoded_max)
        encoded_cur = encoded_max;
    return {encoded_cur, encoded_max};
}

} // namespace fb::game

#endif // !__FB_GAME_CLIENT_AMOUNT_H__
