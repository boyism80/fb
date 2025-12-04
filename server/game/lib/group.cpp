#include <fb/game/group.h>
#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <algorithm>

using namespace fb::game;
using namespace fb::protocol::internal::request;

group::group(server& server, uint32_t id, const std::string& master, const std::vector<std::string>& members) :
    _server(server),
    _id(id),
    _master(master),
    _members(members)
{ }

group::group(group&& g) :
    _server(g._server),
    _id(g._id),
    _master(g._master),
    _members(std::move(g._members)),
    _active_members(std::move(g._active_members))
{ }

void group::enter(std::weak_ptr<character> ch)
{
    auto ptr = ch.lock();
    if (ptr == nullptr)
        return;

    // Prevent duplicate entries
    if (this->_active_members.contains(ptr))
        return;

    this->_active_members.insert(ptr);
}

void group::detach(std::weak_ptr<character> ch)
{
    auto ptr = ch.lock();
    if (ptr == nullptr)
        return;

    if (this->_active_members.contains(ptr) == false)
        return;

    this->_active_members.erase(ptr);
}

void group::add_member(const std::string& name)
{
    // Prevent duplicate entries
    for (auto& member : this->_members)
    {
        if (member == name)
            return;
    }

    this->_members.push_back(name);
}

void group::remove_member(const std::string& name)
{
    this->_members.erase(std::remove_if(this->_members.begin(),
                                        this->_members.end(),
                                        [&name](const std::string& member) {
                                            return member == name;
                                        }),
                         this->_members.end());
}

uint32_t group::id() const
{
    return this->_id;
}

const std::string& group::master() const
{
    return this->_master;
}

std::unordered_set<std::shared_ptr<character>> group::characters() const
{
    return this->_active_members;
}

std::vector<std::string> group::members() const
{
    return std::vector<std::string>(this->_members);
}

std::vector<std::weak_ptr<character>> group::nears(const fb::game::map& map, const fb::model::point16_t& position) const
{
    // Find all characters near the position
    auto nears  = map.nears(position, OBJECT_TYPE::CHARACTER); // same thread
    auto result = std::vector<std::weak_ptr<character>>();

    // Filter only group members
    for (auto& obj : nears)
    {
        auto ch = std::static_pointer_cast<fb::game::character>(obj);
        if (this->_active_members.contains(ch))
            result.push_back(ch);
    }

    return result;
}
