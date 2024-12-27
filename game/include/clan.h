#ifndef __CLAN_H__
#define __CLAN_H__

#include <character.h>

namespace fb::game {

/**
 * @brief      This class describes a clan member.
 */
class clan_member
{
public:
    std::string   name;
    CLAN_POSITION position;
};

/**
 * @brief      This class describes a clan.
 */
class clan
{
public:
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
};

} // namespace fb::game

#endif // !__CLAN_H__
