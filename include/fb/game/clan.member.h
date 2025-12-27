#ifndef __CLAN_MEMBER_H__
#define __CLAN_MEMBER_H__

#include <fb/lua.h>
#include <fb/model/model.h>

namespace fb::game {

using namespace fb::model::enum_value;

class clan_member : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    std::string name;
    CLAN_ROLE   role;

public:
    clan_member(const std::string& name, CLAN_ROLE role);
    ~clan_member() = default;
};

} // namespace fb::game

#endif