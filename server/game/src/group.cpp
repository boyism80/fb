#include <fb/game/group.h>

#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/protocol/flatbuffer/protocol.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace fb::game;
using namespace fb::protocol::internal::request;

group::group(server&                         server,
             uint64_t                        id,
             std::string_view                master,
             const std::vector<std::string>& members,
             uint64_t                        revision) :
    _server(server),
    _id(id),
    _master(std::string(master)),
    _members(members),
    _revision(revision)
{ }

group::group(group&& g) :
    _server(g._server),
    _id(g._id),
    _master(g._master),
    _members(std::move(g._members)),
    _active_members(std::move(g._active_members)),
    _revision(g._revision)
{ }

void group::enter(std::weak_ptr<character> ch)
{
    auto ptr = ch.lock();
    if (ptr == nullptr)
        return;

    // A live entry is the online character, so a duplicate session must not replace it.
    auto found = this->_active_members.find(ptr->id);
    if (found != this->_active_members.end() && found->second.expired() == false)
        return;

    this->_active_members.insert_or_assign(ptr->id, ch);
}

void group::detach(std::weak_ptr<character> ch)
{
    auto ptr = ch.lock();
    if (ptr == nullptr)
        return;

    auto found = this->_active_members.find(ptr->id);
    if (found == this->_active_members.end())
        return;

    auto current = found->second.lock();
    if (current != nullptr && current != ptr)
        return;

    this->_active_members.erase(found);
}

void group::add_member(std::string_view name)
{
    auto name_str = std::string(name);
    // Prevent duplicate entries
    for (auto& member : this->_members)
    {
        if (member == name_str)
            return;
    }

    this->_members.push_back(name_str);
}

void group::remove_member(std::string_view name)
{
    auto name_str = std::string(name);
    this->_members.erase(std::remove_if(this->_members.begin(),
                                        this->_members.end(),
                                        [&name_str](const std::string& member) {
                                            return member == name_str;
                                        }),
                         this->_members.end());
}

uint64_t group::id() const
{
    return this->_id;
}

const std::string& group::master() const
{
    return this->_master;
}

std::vector<std::shared_ptr<character>> group::characters() const
{
    auto result = std::vector<std::shared_ptr<character>>{};
    result.reserve(this->_active_members.size());
    for (const auto& [id, weak] : this->_active_members)
    {
        std::ignore = id;
        auto ptr    = weak.lock();
        if (ptr != nullptr)
            result.push_back(ptr);
    }
    return result;
}

std::vector<std::string> group::members() const
{
    return std::vector<std::string>(this->_members);
}

uint64_t group::revision() const
{
    return this->_revision;
}

void group::revision(uint64_t value)
{
    this->_revision = value;
}

void group::reload(const group& fresh)
{
    this->_master   = fresh._master;
    this->_members  = fresh._members;
    this->_revision = fresh._revision;
}

std::vector<std::weak_ptr<character>> group::nears(const fb::game::map& map, const fb::model::point16_t& position) const
{
    // Find all characters near the position
    auto nears  = map.nears(position, OBJECT_TYPE::CHARACTER); // same thread
    auto result = std::vector<std::weak_ptr<character>>();

    // Filter only group members
    for (auto& obj : nears)
    {
        auto ch    = std::static_pointer_cast<fb::game::character>(obj);
        auto found = this->_active_members.find(ch->id);
        if (found != this->_active_members.end() && found->second.lock() == ch)
            result.push_back(ch);
    }

    return result;
}
