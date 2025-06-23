#ifndef __SHARED_H__
#define __SHARED_H__

#include <cstdint>
#include <unordered_map>
#include <string>
#include <memory>
#include <fb/game/group.h>
#include <fb/game/clan.h>
#include <fb/locker.h>
#include <shared_mutex>

namespace fb::game {

/**
 * @brief      Manages shared game data across multiple threads and server shards.
 *
 *             This class provides thread-safe containers for managing global game
 *             state that needs to be shared across different server threads and
 *             potentially multiple server shards. It includes collections for
 *             groups, clans, and character lookups with proper synchronization.
 *
 *             Key features:
 *             - Thread-safe containers with automatic locking
 *             - Character lookup by both ID and name
 *             - Group and clan management with shared locks
 *             - Cross-shard data synchronization support
 *             - Efficient concurrent access patterns
 */
class shard_params
{
public:
    using group_container = std::unordered_map<uint32_t, shared_group_lock>; ///< Container for groups with shared locks
    using clan_container  = std::unordered_map<uint32_t, shared_clan_lock>;  ///< Container for clans with shared locks
    using character_name_container =
        std::unordered_map<std::string, std::shared_ptr<fb::game::character>>; ///< Character lookup by name
    using character_id_container =
        std::unordered_map<uint32_t, std::shared_ptr<fb::game::character>>; ///< Character lookup by ID

public:
    fb::locker<group_container>          groups; ///< Thread-safe container for active groups
    fb::locker<clan_container>           clans;  ///< Thread-safe container for active clans
    fb::locker<character_id_container>   ids;    ///< Thread-safe character lookup by ID
    fb::locker<character_name_container> names;  ///< Thread-safe character lookup by name

public:
    /**
     * @brief      Constructs a new shard parameters instance.
     *
     *             Initializes all containers and synchronization primitives for
     *             managing shared game state across server threads.
     */
    shard_params() = default;

    /**
     * @brief      Copy constructor is deleted to prevent accidental copying.
     */
    shard_params(const shard_params&) = delete;

    /**
     * @brief      Move constructor for efficient resource transfer.
     */
    shard_params(shard_params&&) = default;

    /**
     * @brief      Destroys the shard parameters and cleans up resources.
     */
    ~shard_params() = default;
};

} // namespace fb::game

#endif