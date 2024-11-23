#ifndef __GROUP_H__
#define __GROUP_H__

#include <character.h>

namespace fb::game {

class group
{
private:
    uint32_t                 _id;
    std::string              _master;
    std::vector<std::string> _members;
    std::vector<character*>  _active_members;

public:
    group(uint32_t id, const std::string& master, const std::string& member) :
        _id(id),
        _master(master)
    {
        this->_members.push_back(member);
    }
    group(uint32_t id, const std::string& master, const std::vector<std::string>& members) :
        _id(id),
        _master(master),
        _members(members)
    { }
    group(const group&) = delete;
    group(group&& g) :
        _id(g._id),
        _master(g._master),
        _members(std::move(g._members)),
        _active_members(std::move(g._active_members))
    { }
    ~group() = default;

public:
    void enter(const std::string& name)
    {
        auto i = std::find(this->_members.begin(), this->_members.end(), name);
        if (i != this->_members.end())
            throw std::runtime_error(std::format("{} already joined this group {}", name, this->_id));

        this->_members.push_back(name);
    }

    void enter(fb::game::character& ch)
    {
        auto i = std::find(this->_active_members.begin(), this->_active_members.end(), &ch);
        if (i != this->_active_members.end())
            throw std::runtime_error(std::format("{} already joined this group {}", ch.name(), this->_id));

        this->_active_members.push_back(&ch);
    }

    void leave_active_member(fb::game::character& ch)
    {
        auto i = std::find(this->_active_members.begin(), _active_members.end(), &ch);
        if (i == this->_active_members.end())
            throw std::runtime_error(std::format("{} is not a member of this group {}", ch.name(), this->_id));

        this->_active_members.erase(i);
    }

    void leave(const std::string& name)
    {
        auto i = std::find(this->_members.begin(), this->_members.end(), name);
        if (i == this->_members.end())
            throw std::runtime_error(std::format("{} is not a member of this group {}", name, this->_id));

        this->_members.erase(i);
    }

    std::vector<fb::game::character*> active_members() const
    {
        return this->_active_members;
    }

    std::string master() const
    {
        return this->_master;
    }

    std::vector<std::string> members() const
    {
        return std::vector<std::string>(this->_members);
    }
};

} // namespace fb::game

#endif // !__GROUP_H__
