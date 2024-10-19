#include <clan.h>

fb::game::clan::clan(const std::string& name, fb::game::session& owner, const session::container& members) : 
    _name(name), _owner(owner), _members(members)
{ }

fb::game::clan::clan(const std::string& name, const std::string& title, fb::game::session& owner, const session::container& members) : 
    _name(name), _title(title), _owner(owner), _members(members)
{ }

fb::game::clan::~clan()
{ }

const fb::game::session& fb::game::clan::owner() const
{
    return this->_owner;
}

const fb::game::session::container& fb::game::clan::members() const
{
    return this->_members;
}

const std::string& fb::game::clan::name() const
{
    return this->_name;
}

void fb::game::clan::name(const std::string& value)
{
    this->_name = value;
}

const std::string& fb::game::clan::title() const
{
    return this->_title;
}

void fb::game::clan::title(const std::string& value)
{
    this->_title = value;
}

bool fb::game::clan::enter(fb::game::session& session)
{
    if(this->_members.contains(session))
        return false;

    this->_members.push(session);
    session._clan = this;
    return true;
}

bool fb::game::clan::leave(fb::game::session& session)
{
    if(this->_members.contains(session) == false)
        return false;

    this->_members.erase(session);
    session._clan = nullptr;
    return true;
}
