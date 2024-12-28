#ifndef __CLAN_H__
#define __CLAN_H__

#include <character.h>
#include <clan.member.h>

namespace fb::game {

/**
 * @brief      This class describes a clan.
 */
class clan : public lua::luable
{
public:
    LUA_PROTOTYPE

private:
    context&                                           _context;
    uint32_t                                           _id;
    std::string                                        _name;
    std::optional<std::string>                         _title;
    std::vector<clan_member>                           _members;
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
    std::vector<character*> nears(const fb::game::map& map, const point16_t& position) const;

public:
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_name(lua_State* lua);
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
    static int builtin_title(lua_State* lua);
};

} // namespace fb::game

#endif // !__CLAN_H__
