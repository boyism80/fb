#include "session.h"
#include "group.h"

fb::game::group::group(session& leader) : 
    _leader(&leader)
{
    this->add(leader);
}

fb::game::group::~group()
{
    for(auto member : this->_members)
        member->group_leave();
}

bool fb::game::group::add(session& session)
{
    if(this->_members.size() >= 8)
        return false;

    this->_members.push_back(&session);
    return true;
}

bool fb::game::group::remove(session& session)
{
    auto i = std::find(this->_members.begin(), this->_members.end(), &session);
    if(i == this->_members.end())
        return false;

    (*i)->group_leave();
    this->_members.erase(i);
    return true;
}