#ifndef __MAP_CONTAINER_H__
#define __MAP_CONTAINER_H__

#include <fb/container.h>
#include <fb/game/map.h>

namespace fb::game {

/**
 * @brief      This class describes map_container.
 */
class map_container : public fb::kv_container<uint32_t, map>
{
private:
    std::mutex _mutex;

public:
    fb::game::context& context;
    const uint32_t     host;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  context  The context
     * @param[in]  host     The host
     */
    map_container(fb::game::context& context, uint32_t host);
    /**
     * @brief      Destroys the object.
     */
    ~map_container();

private:
    /**
     * @brief      Loads a data.
     *
     * @param[in]  id      The identifier
     * @param      buffer  The buffer
     *
     * @return     { description_of_the_return_value }
     */
    static bool load_data(uint32_t id, std::vector<char>& buffer);
    /**
     * @brief      Loads a block.
     *
     * @param[in]  id      The identifier
     * @param      buffer  The buffer
     *
     * @return     { description_of_the_return_value }
     */
    static bool load_block(uint32_t id, Json::Value& buffer);

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  moel  The moel
     */
    void load(const fb::model::map& moel);
    /**
     * @brief      { function_description }
     *
     * @param[in]  name  The name
     *
     * @return     { description_of_the_return_value }
     */
    map* name2map(const std::string& name) const;
};

} // namespace fb::game

#endif