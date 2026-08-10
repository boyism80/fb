#ifndef __FB_GAME_SERVICE_SCRIPT_TIMER_H__
#define __FB_GAME_SERVICE_SCRIPT_TIMER_H__

#include <async/task.h>
#include <fb/timer.h>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace fb::game {
class server;
class map;
} // namespace fb::game

namespace fb::game::service {

/**
 * @brief      Runtime Lua timers pinned to a map thread.
 *
 *             Keyed by (map.id, name). Re-setting the same name replaces the previous timer.
 *             Callbacks are path + function (no closures). Distinct from client UI timer packets.
 */
class script_timer
{
public:
    static constexpr size_t MAX_ACTIVE = 256;

public:
    fb::game::server& server;

private:
    struct key_t
    {
        uint32_t    map_id = 0;
        std::string name;

        bool operator== (const key_t& other) const
        {
            return this->map_id == other.map_id && this->name == other.name;
        }
    };

    struct key_hash
    {
        size_t operator() (const key_t& key) const
        {
            return std::hash<uint32_t>{}(key.map_id) ^ (std::hash<std::string>{}(key.name) << 1);
        }
    };

    struct entry_t
    {
        uint64_t                   id     = 0;
        uint32_t                   map_id = 0;
        std::string                name;
        std::string                path;
        std::string                func;
        bool                       repeat = true;
        std::shared_ptr<fb::timer> timer;
    };

    std::mutex                                    _mutex;
    uint64_t                                      _next_id = 1;
    std::unordered_map<uint64_t, entry_t>         _by_id;
    std::unordered_map<key_t, uint64_t, key_hash> _by_key;

public:
    explicit script_timer(fb::game::server& server);

public:
    // clang-format off
    async::task<std::optional<uint64_t>> create_or_replace(fb::game::map& map,
                                                           std::string     name,
                                                           uint32_t        interval_ms,
                                                           std::string     path,
                                                           std::string     func,
                                                           bool            repeat);
    bool                                 cancel(uint32_t map_id, std::string_view name);
    bool                                 cancel(uint64_t id);
    void                                 cancel_all(uint32_t map_id);
    // clang-format on

private:
    void erase_unlocked(uint64_t id);
    void erase(uint64_t id);
};

} // namespace fb::game::service

#endif
