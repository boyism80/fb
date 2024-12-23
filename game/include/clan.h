#ifndef __CLAN_H__
#define __CLAN_H__

#include <character.h>

namespace fb { namespace game {

/**
 * @brief      This class describes a clan.
 */
class clan
{
private:
    character&           _owner;
    character::container _members;
    std::string          _name;
    std::string          _title;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  name     The name
     * @param      owner    The owner
     * @param[in]  members  The members
     */
    clan(const std::string& name, fb::game::character& owner, const character::container& members);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  name     The name
     * @param[in]  title    The title
     * @param      owner    The owner
     * @param[in]  members  The members
     */
    clan(const std::string&          name,
         const std::string&          title,
         fb::game::character&        owner,
         const character::container& members);
    /**
     * @brief      Destroys the object.
     */
    ~clan();

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const character& owner() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const character::container& members() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const std::string& name() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void name(const std::string& value);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const std::string& title() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void title(const std::string& value);

public:
    /**
     * @brief      { function_description }
     *
     * @param      ch  The ch
     *
     * @return     { description_of_the_return_value }
     */
    bool enter(fb::game::character& ch);
    /**
     * @brief      { function_description }
     *
     * @param      ch  The ch
     *
     * @return     { description_of_the_return_value }
     */
    bool leave(fb::game::character& ch);
};

}} // namespace fb::game

#endif // !__CLAN_H__
