#ifndef __CLAN_H__
#define __CLAN_H__

#include "session.h"
#include "lua.h"

namespace fb { namespace game {

class clan
{
private:
    session&                    _owner;
    session::container          _members;
    std::string                 _name;
    std::string                 _title;

public:
    clan(const std::string& name, fb::game::session& owner, const session::container& members);
    clan(const std::string& name, const std::string& title, fb::game::session& owner, const session::container& members);
    ~clan();

public:
    const session&              owner() const;
    const session::container&   members() const;
    const std::string&          name() const;
    void                        name(const std::string& value);
    const std::string&          title() const;
    void                        title(const std::string& value);

public:
    bool                        enter(fb::game::session& session);
    bool                        leave(fb::game::session& session);
};

} }

#endif // !__CLAN_H__
