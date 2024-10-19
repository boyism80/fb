#include <group.h>

fb::game::group::group(session& leader) : 
    _leader(&leader)
{
    this->enter(leader);
}

fb::game::group::~group()
{ }

fb::game::session* fb::game::group::enter(session& session)
{
    if(this->_members.size() >= 8)
        return nullptr;

    auto i = std::find(this->_members.begin(), this->_members.end(), &session);
    if(i != this->_members.end())
        return nullptr;

    session._group = this;
    this->_members.push(session);
    return &session;
}

fb::game::session* fb::game::group::leave(session& session)
{
    auto i = std::find(this->_members.begin(), this->_members.end(), &session);
    if(i == this->_members.end())
        return this->_leader;

    this->_members.erase(session);
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

const fb::game::session::container& fb::game::group::members() const
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
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto group = thread->touserdata<fb::game::group>(1);
    if(group == nullptr)
        return 0;
    
    thread->new_table();
    for(int i = 0; i < group->_members.size(); i++)
    {
        thread->pushobject(group->_members[i]);
        thread->rawseti(-2, i+1);
    }

    return 1;
}

int fb::game::group::builtin_leader(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto group = thread->touserdata<fb::game::group>(1);
    if(group == nullptr)
        return 0;
    
    thread->pushobject(group->_leader);
    return 1;
}
