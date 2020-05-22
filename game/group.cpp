#include "session.h"
#include "group.h"

fb::game::group::group(session& leader) : 
    _leader(&leader)
{
    this->enter(leader);
}

fb::game::group::~group()
{
    this->clear();
}

fb::game::session* fb::game::group::enter(session& session)
{
    if(this->size() >= 8)
        return nullptr;

    auto i = std::find(this->begin(), this->end(), &session);
    if(i != this->end())
        return nullptr;

    session._group = this;
    this->push_back(&session);
    return &session;
}

fb::game::session* fb::game::group::leave(session& session)
{
    auto i = std::find(this->begin(), this->end(), &session);
    if(i == this->end())
        return this->_leader;

    this->erase(i);
    session._group = nullptr;

    if(session == *this->_leader)
    {
        if(this->size() == 0)
            this->_leader = nullptr;
        else
            this->_leader = (*this)[0];
    }

    return this->_leader;
}

bool fb::game::group::contains(session& session)
{
    return std::find(this->begin(), this->end(), &session) != this->end();
}

fb::game::session& fb::game::group::leader() const
{
    return *this->_leader;
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
    for(auto session : group)
        session->_group = nullptr;

    delete &group;
}
