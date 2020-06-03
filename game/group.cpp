#include "session.h"
#include "group.h"

fb::game::group::group(session& leader) : 
    _leader(&leader)
{
    this->enter(leader);
}

fb::game::group::~group()
{
    this->_members.clear();
}

fb::game::session* fb::game::group::enter(session& session)
{
    if(this->_members.size() >= 8)
        return nullptr;

    auto i = std::find(this->_members.begin(), this->_members.end(), &session);
    if(i != this->_members.end())
        return nullptr;

    session._group = this;
    this->_members.push_back(&session);
    return &session;
}

fb::game::session* fb::game::group::leave(session& session)
{
    auto i = std::find(this->_members.begin(), this->_members.end(), &session);
    if(i == this->_members.end())
        return this->_leader;

    this->_members.erase(i);
    session._group = nullptr;

    if(session == *this->_leader)
    {
        if(this->_members.size() == 0)
            this->_leader = nullptr;
        else
            this->_leader = this->_members[0];
    }

    return this->_leader;
}

bool fb::game::group::contains(session& session)
{
    return std::find(this->_members.begin(), this->_members.end(), &session) != this->_members.end();
}

fb::game::session& fb::game::group::leader() const
{
    return *this->_leader;
}

const std::vector<fb::game::session*>& fb::game::group::members() const
{
    return this->_members;
}

fb::game::group* fb::game::group::create(session& leader)
{
    if(leader._group == nullptr)
        return new fb::game::group(leader);
    else
        return leader._group;
}

void fb::game::group::destroy(fb::game::group& group)
{
    for(auto session : group._members)
        session->_group = nullptr;

    delete &group;
}

int fb::game::group::builtin_members(lua_State* lua)
{
    auto group = *(fb::game::group**)lua_touserdata(lua, 1);

    lua_newtable(lua);
    for(int i = 0; i < group->_members.size(); i++)
    {
        group->_members[i]->to_lua(lua);
        lua_rawseti(lua, -2, i+1);
    }

    return 1;
}

int fb::game::group::builtin_leader(lua_State* lua)
{
    auto group = *(fb::game::group**)lua_touserdata(lua, 1);
    group->_leader->to_lua(lua);
    return 1;
}
