#ifndef __MAP_CONTAINER_H__
#define __MAP_CONTAINER_H__

#include <fb/container.h>
#include <fb/game/map.h>

namespace fb::game {

/**
 * @brief      Container for managing game maps within a server context.
 *
 *             This class provides a thread-safe container for managing all game maps
 *             within a server instance. It handles map loading, caching, and provides
 *             efficient lookup capabilities for map-related operations. The container
 *             supports both data and block loading for map initialization.
 *
 *             Key features:
 *             - Thread-safe map storage and retrieval
 *             - Dynamic map loading from data sources
 *             - Name-based map lookup for transitions
 *             - Integration with game context and host management
 *             - Efficient caching and memory management
 */
class map_container : public fb::kv_container<uint32_t, map>
{
private:
    std::mutex _mutex; ///< Mutex for thread-safe operations

public:
    fb::game::context& context; ///< Reference to the game context
    const uint32_t     host;    ///< Host identifier for this map container

public:
    /**
     * @brief      Constructs a new map container for the specified context and host.
     *
     *             Initializes a map container associated with the given game context
     *             and host identifier, ready to manage maps for that server instance.
     *
     * @param[in]  context  The game context that will manage these maps.
     * @param[in]  host     The host identifier for this map container.
     */
    map_container(fb::game::context& context, uint32_t host);

    /**
     * @brief      Destroys the map container and cleans up resources.
     */
    ~map_container();

private:
    /**
     * @brief      Loads map data from the data source.
     *
     *             Loads the raw map data for the specified map ID into the provided
     *             buffer, handling file I/O and data validation.
     *
     * @param[in]  id      The map identifier to load data for.
     * @param      buffer  The buffer to store the loaded map data.
     *
     * @return     True if the data was loaded successfully, false otherwise.
     */
    static bool load_data(uint32_t id, std::vector<char>& buffer);

    /**
     * @brief      Loads map block configuration from the data source.
     *
     *             Loads the map block configuration data for the specified map ID
     *             into the provided JSON buffer, handling parsing and validation.
     *
     * @param[in]  id      The map identifier to load block data for.
     * @param      buffer  The JSON buffer to store the loaded block configuration.
     *
     * @return     True if the block data was loaded successfully, false otherwise.
     */
    static bool load_block(uint32_t id, Json::Value& buffer);

public:
    /**
     * @brief      Loads a map from the specified model configuration.
     *
     *             Creates and initializes a map instance using the provided model
     *             configuration, adding it to the container for management.
     *
     * @param[in]  model  The map model containing configuration and metadata.
     */
    void load(const fb::model::map& model);

    /**
     * @brief      Finds a map by its name.
     *
     *             Searches the container for a map with the specified name,
     *             enabling name-based map transitions and lookups.
     *
     * @param[in]  name  The name of the map to find.
     *
     * @return     Pointer to the map with the specified name, or nullptr if not found.
     */
    map* name2map(const std::string& name) const;
};

} // namespace fb::game

#endif