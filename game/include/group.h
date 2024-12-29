#ifndef __GROUP_H__
#define __GROUP_H__

#include <character.h>
#include <async/task.h>

namespace fb::game {

/**
 * @brief      This class describes a group.
 */
class group : public lua::luable, public fb::thread_switchable
{
public:
    LUA_PROTOTYPE

private:
    context&                 _context;
    uint32_t                 _id;
    std::string              _master;
    std::vector<std::string> _members;
    std::vector<character*>  _active_members;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  id       The identifier
     */
    group(context& context, uint32_t id);

    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    group(const group&) = delete;

    /**
     * @brief      Constructs a new instance.
     *
     * @param      g     { parameter_description }
     */
    group(group&& g);

    /**
     * @brief      Destroys the object.
     */
    ~group() = default;

public:
    /**
     * @brief      { function_description }
     *
     * @param      ch    { parameter_description }
     */
    void enter(character& ch);

    /**
     * @brief      { function_description }
     *
     * @param      ch    { parameter_description }
     */
    void leave(character& ch);

    /**
     * @brief      { function_description }
     *
     * @param[in]  master   The master
     * @param[in]  members  The members
     *
     * @return     { description_of_the_return_value }
     */
    async::task<void> update(const std::string& master, const std::vector<std::string>& members);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t id() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool inited() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const std::string& master() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<character*> characters() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<std::string> members() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  map       The map
     * @param[in]  position  The position
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<character*> nears(const fb::game::map& map, const point16_t& position) const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::thread* thread() const override;

public:
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_master(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_members(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_nears(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_message(lua_State* lua);
};

} // namespace fb::game

#endif // !__GROUP_H__
