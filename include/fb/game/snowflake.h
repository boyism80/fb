#ifndef FB_GAME_SNOWFLAKE_H
#define FB_GAME_SNOWFLAKE_H

#include <chrono>
#include <cstdint>
#include <format>
#include <mutex>
#include <stdexcept>

namespace fb::game {

// 63-bit id: milliseconds since 2026-01-01 UTC (41) | world (6) | server id (8) | sequence (8).
// World 0 is the cross server space; server ids only need to be unique within a world.
class snowflake
{
public:
    static constexpr uint32_t SEQUENCE_BITS   = 8;
    static constexpr uint32_t SERVER_BITS     = 8;
    static constexpr uint32_t WORLD_BITS      = 6;
    static constexpr uint32_t SERVER_SHIFT    = SEQUENCE_BITS;
    static constexpr uint32_t WORLD_SHIFT     = SERVER_SHIFT + SERVER_BITS;
    static constexpr uint32_t TIMESTAMP_SHIFT = WORLD_SHIFT + WORLD_BITS;
    static constexpr uint64_t SEQUENCE_MASK   = (1ULL << SEQUENCE_BITS) - 1;
    static constexpr int64_t  EPOCH_MS        = 1767225600000; // 2026-01-01T00:00:00Z

private:
    uint64_t   _origin;
    std::mutex _mutex;
    uint64_t   _last_timestamp = 0;
    uint64_t   _sequence       = 0;

public:
    snowflake(uint32_t world, uint8_t server)
    {
        if (world >= (1U << WORLD_BITS))
            throw std::runtime_error(std::format("snowflake world {} does not fit in {} bits", world, WORLD_BITS));

        this->_origin = (static_cast<uint64_t>(world) << WORLD_SHIFT) | (static_cast<uint64_t>(server) << SERVER_SHIFT);
    }

    uint64_t next()
    {
        auto now = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                .count() -
            EPOCH_MS);

        auto _ = std::lock_guard(this->_mutex);
        if (now > this->_last_timestamp)
        {
            this->_last_timestamp = now;
            this->_sequence       = 0;
        }
        else
        {
            // Borrow the next millisecond instead of waiting so ids stay unique and increasing.
            this->_sequence++;
            if (this->_sequence > SEQUENCE_MASK)
            {
                this->_last_timestamp++;
                this->_sequence = 0;
            }
        }

        return (this->_last_timestamp << TIMESTAMP_SHIFT) | this->_origin | this->_sequence;
    }
};

} // namespace fb::game

#endif // FB_GAME_SNOWFLAKE_H
