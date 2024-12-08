#include "group.h"
#include "context.h"

fb::game::group::group(context& context, uint32_t id) :
    _context(context)
{ }
fb::game::group::group(context&                        context,
                       uint32_t                        id,
                       const std::string&              master,
                       const std::vector<std::string>& members) :
    _context(context),
    _id(id),
    _master(master),
    _members(members)
{ }
fb::game::group::group(group&& g) :
    _context(g._context),
    _id(g._id),
    _master(g._master),
    _members(std::move(g._members)),
    _active_members(std::move(g._active_members))
{ }

uint32_t fb::game::group::id() const
{
    return this->_id;
}

void fb::game::group::master(const std::string& name)
{
    this->_master = name;
}

bool fb::game::group::inited() const
{
    return !this->_master.empty();
}

const std::string& fb::game::group::master() const
{
    return this->_master;
}

void fb::game::group::enter(const std::string& name)
{
    auto i = std::find(this->_members.begin(), this->_members.end(), name);
    if (i != this->_members.end())
        return;

    this->_members.push_back(name);
}

void fb::game::group::enter(fb::game::character& ch)
{
    auto i = std::find(this->_active_members.begin(), this->_active_members.end(), &ch);
    if (i != this->_active_members.end())
        return;

    this->_active_members.push_back(&ch);
}

void fb::game::group::leave_active_member(fb::game::character& ch)
{
    auto i = std::find(this->_active_members.begin(), _active_members.end(), &ch);
    if (i == this->_active_members.end())
        return;

    this->_active_members.erase(i);
}

void fb::game::group::leave(const std::string& name)
{
    auto i = std::find(this->_members.begin(), this->_members.end(), name);
    if (i == this->_members.end())
        return;

    this->_members.erase(i);
}

std::vector<fb::game::character*> fb::game::group::active_members() const
{
    return this->_active_members;
}

std::vector<std::string> fb::game::group::members() const
{
    return std::vector<std::string>(this->_members);
}

fb::thread* fb::game::group::thread() const
{
    return this->_context.threads.modular(this->_id);
}