#ifndef __THREAD_PARAMS_H__
#define __THREAD_PARAMS_H__

#include <fb/game/group.h>
#include <fb/game/map.h>
#include <fb/game/character.h>
#include <unordered_map>
#include <async/task.h>

namespace fb::game {

/**
 * @brief      Thread-specific parameter container for game world data.
 *
 *             This class serves as a container for thread-local game world data, enabling
 *             efficient multi-threaded game server operation. Each worker thread maintains
 *             its own instance of thread_params containing the subset of game world objects
 *             that the thread is responsible for managing.
 *
 *             The thread_params system enables:
 *             - Thread-local storage of game objects for performance
 *             - Reduced contention between worker threads
 *             - Efficient load balancing of game world objects
 *             - Isolated processing of game logic per thread
 *
 *             Each thread manages its assigned maps, groups, characters, and mob respawn
 *             points independently, with synchronization handled at higher levels when
 *             cross-thread communication is required.
 */
class thread_params
{
public:
    /// Container type for thread-local group management
    using group_container = std::unordered_map<uint32_t, std::unique_ptr<group>>;
    /// Container type for thread-local map references
    using map_container = std::unordered_map<uint32_t, std::shared_ptr<map>>;
    /// Container type for thread-local mob respawn points
    using rezen_container = std::vector<rezen>;
    /// Container type for thread-local character references
    using character_container = std::unordered_map<uint32_t, std::shared_ptr<character>>;

public:
    group_container     groups;     ///< Groups managed by this thread
    map_container       maps;       ///< Maps managed by this thread
    rezen_container     rezens;     ///< Mob respawn points managed by this thread
    character_container characters; ///< Characters currently on this thread's maps
};

} // namespace fb::game

#endif