#ifndef __CLAN_H__
#define __CLAN_H__

#include <fb/game/character.h>
#include <fb/game/clan.member.h>

namespace fb::game {

/**
 * @brief      This class describes a clan.
 */
class clan : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    struct builtin;

private:
    context&                                           _context;
    uint32_t                                           _id;
    std::string                                        _name;
    std::optional<std::string>                         _title;
    std::unordered_map<std::string, clan_member>       _members;
    std::unordered_map<uint32_t, fb::game::character*> _characters;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  id       The identifier
     */
    clan(context& context, uint32_t id);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    clan(const clan&) = delete;
    /**
     * @brief      Constructs a new instance.
     *
     * @param      <unnamed>  { parameter_description }
     */
    clan(clan&&);
    /**
     * @brief      Destroys the object.
     */
    ~clan() = default;

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  name     The name
     * @param[in]  title    The title
     * @param[in]  members  The members
     */
    void update(const std::string&                name,
                const std::optional<std::string>& title,
                const std::vector<clan_member>&   members);

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
    const std::string& name() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const std::optional<std::string>& title() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  title  The title
     */
    void title(const std::optional<std::string>& title);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const std::unordered_map<std::string, clan_member>& members() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  member  The member
     */
    void join(const clan_member& member);

    /**
     * @brief      { function_description }
     *
     * @param[in]  member  The member
     */
    void leave(const std::string& member);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const std::unordered_map<uint32_t, fb::game::character*>& characters() const;

    /**
     * @brief      Attaches the character.
     *
     * @param      ch    { parameter_description }
     */
    void attach_character(character& ch);
    /**
     * @brief      Detaches the character.
     *
     * @param      ch    { parameter_description }
     */
    void detach_character(character& ch);

    /**
     * @brief      { function_description }
     *
     * @param[in]  map       The map
     * @param[in]  position  The position
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<character*> nears(const fb::game::map& map, const fb::model::point16_t& position) const;
};

struct clan::builtin
{
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_name(lua_State* L);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_members(lua_State* L);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_nears(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_title(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_join(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_leave(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_message(lua_State* L);
};

} // namespace fb::game

#endif // !__CLAN_H__
