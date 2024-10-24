#ifndef __CLAN_H__
#define __CLAN_H__

#include <character.h>

namespace fb { namespace game {

class clan
{
private:
    character&           _owner;
    character::container _members;
    std::string          _name;
    std::string          _title;

public:
    clan(const std::string& name, fb::game::character& owner, const character::container& members);
    clan(const std::string& name, const std::string& title, fb::game::character& owner, const character::container& members);
    ~clan();

public:
    const character&            owner() const;
    const character::container& members() const;
    const std::string&          name() const;
    void                        name(const std::string& value);
    const std::string&          title() const;
    void                        title(const std::string& value);

public:
    bool enter(fb::game::character& session);
    bool leave(fb::game::character& session);
};

}} // namespace fb::game

#endif // !__CLAN_H__
