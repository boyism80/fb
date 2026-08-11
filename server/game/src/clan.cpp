#include <fb/game/clan.h>
#include <fb/game/server.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/config.h>

using namespace fb::game;
using namespace fb::model;
using namespace fb::protocol::internal;

clan::clan(server&                                             server,
           uint32_t                                            id,
           std::string_view                                    name,
           const std::optional<std::string>&                   title,
           const std::unordered_map<std::string, clan_member>& members,
           const std::optional<uint32_t>&                      allied_clan_id,
           const std::unordered_set<uint32_t>&                 enemy_clan_ids) :
    _server(server),
    _id(id),
    _name(std::string(name)),
    _title(title),
    _members(members),
    _allied_clan_id(allied_clan_id),
    _enemy_clan_ids(enemy_clan_ids)
{ }

clan::clan(clan&& r) :
    _server(r._server),
    _id(r._id),
    _name(r._name),
    _title(r._title),
    _members(std::move(r._members)),
    _allied_clan_id(r._allied_clan_id),
    _enemy_clan_ids(std::move(r._enemy_clan_ids))
{ }

void clan::update(std::string_view name, const std::optional<std::string>& title, const member_map& members)
{
    this->_name    = std::string(name);
    this->_title   = title;
    this->_members = members;
}

uint32_t clan::id() const
{
    return this->_id;
}

const std::string& clan::name() const
{
    return this->_name;
}

const std::optional<std::string>& clan::title() const
{
    return this->_title;
}

void clan::title(const std::optional<std::string>& title)
{
    this->_title = title;
}

const std::unordered_map<std::string, clan_member>& clan::members() const
{
    return this->_members;
}

clan_member* clan::member(std::string_view name)
{
    auto name_str = std::string(name);
    auto it       = this->_members.find(name_str);
    if (it != this->_members.end())
        return &it->second;

    return nullptr;
}

const clan_member* clan::member(std::string_view name) const
{
    auto name_str = std::string(name);
    auto it       = this->_members.find(name_str);
    if (it != this->_members.end())
        return &it->second;

    return nullptr;
}

bool clan::contains(std::string_view name) const
{
    auto name_str = std::string(name);
    return this->_members.contains(name_str);
}

void clan::join(const clan_member& member)
{
    this->_members.insert({member.name, member});
}

void clan::leave(std::string_view member)
{
    auto member_str = std::string(member);
    this->_members.erase(member_str);
}

void clan::change_role(std::string_view member_name, CLAN_ROLE new_role)
{
    auto member_name_str = std::string(member_name);
    auto it              = this->_members.find(member_name_str);
    if (it != this->_members.end())
    {
        it->second.role = new_role;
    }
}

const std::unordered_map<uint32_t, std::weak_ptr<fb::game::character>>& clan::characters() const
{
    return this->_characters;
}

void clan::attach(std::weak_ptr<character> ch)
{
    auto shared = ch.lock();
    if (shared == nullptr)
        return;

    if (this->_characters.contains(shared->id))
        return;

    this->_characters.insert({shared->id, ch});
}

void clan::detach(std::weak_ptr<character> ch)
{
    auto shared = ch.lock();
    if (shared == nullptr)
        return;

    if (!this->_characters.contains(shared->id))
        return;

    this->_characters.erase(shared->id);
}

const std::optional<uint32_t>& clan::allied_clan_id() const
{
    return this->_allied_clan_id;
}

void clan::allied_clan_id(const std::optional<uint32_t>& value)
{
    this->_allied_clan_id = value;
}

bool clan::is_allied(uint32_t other_clan_id) const
{
    return this->_allied_clan_id.has_value() && this->_allied_clan_id.value() == other_clan_id;
}

const std::unordered_set<uint32_t>& clan::enemy_clan_ids() const
{
    return this->_enemy_clan_ids;
}

void clan::add_enemy_clan(uint32_t other_clan_id)
{
    this->_enemy_clan_ids.insert(other_clan_id);
}

void clan::remove_enemy_clan(uint32_t other_clan_id)
{
    this->_enemy_clan_ids.erase(other_clan_id);
}

bool clan::is_hostile(uint32_t other_clan_id) const
{
    return this->_enemy_clan_ids.contains(other_clan_id);
}

std::vector<std::shared_ptr<fb::game::character>> clan::nears(const fb::game::map& map, const point16_t& position) const
{
    auto nears  = map.nears(position, OBJECT_TYPE::CHARACTER); // same thread
    auto result = std::vector<std::shared_ptr<fb::game::character>>();

    for (const auto& obj : nears)
    {
        if (obj->is(OBJECT_TYPE::CHARACTER) == false)
            continue;

        auto ch = std::static_pointer_cast<fb::game::character>(obj);
        if (ch->clan_id().has_value() == false)
            continue;

        auto& clan_id = ch->clan_id().value();
        {
            auto guard = this->_server.clans.enter_read(clan_id);
            if (ch->clan_id() == clan_id)
                result.push_back(ch);
        } // same thread
    }

    return result;
}
