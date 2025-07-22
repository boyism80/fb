#include <fb/game/clan.h>
#include <fb/game/context.h>

using namespace fb::game;
using namespace fb::model;

clan::clan(context& context, uint32_t id, const std::string& name, const std::optional<std::string>& title) :
    _context(context),
    _id(id),
    _name(name),
    _title(title)
{ }

clan::clan(clan&& r) :
    _context(r._context),
    _id(r._id),
    _name(r._name),
    _title(r._title),
    _members(std::move(r._members))
{ }

void clan::update(const std::string&                name,
                  const std::optional<std::string>& title,
                  const std::vector<clan_member>&   members)
{
    this->_name  = name;
    this->_title = title;

    this->_members.clear();
    for (auto& member : members)
    {
        this->_members.insert({member.name, member});
    }
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

clan_member* clan::member(const std::string& name)
{
    auto it = this->_members.find(name);
    if (it != this->_members.end())
        return &it->second;

    return nullptr;
}

void clan::join(const clan_member& member)
{
    this->_members.insert({member.name, member});
}

void clan::leave(const std::string& member)
{
    this->_members.erase(member);
}

void clan::change_role(const std::string& member_name, CLAN_ROLE new_role)
{
    auto it = this->_members.find(member_name);
    if (it != this->_members.end())
    {
        it->second.role = new_role;
    }
}

const std::unordered_map<uint32_t, std::weak_ptr<fb::game::character>>& clan::characters() const
{
    return this->_characters;
}

void clan::attach_character(std::weak_ptr<character> ch)
{
    auto shared = ch.lock();
    if (shared == nullptr)
        return;

    if (this->_characters.contains(shared->id()))
        return;

    this->_characters.insert({shared->id(), ch});
}

void clan::detach(std::weak_ptr<character> ch)
{
    auto shared = ch.lock();
    if (shared == nullptr)
        return;

    if (!this->_characters.contains(shared->id()))
        return;

    this->_characters.erase(shared->id());
}

std::vector<std::shared_ptr<fb::game::character>> clan::nears(const fb::game::map& map, const point16_t& position) const
{
    auto nears  = map.nears(position, OBJECT_TYPE::CHARACTER); // same thread
    auto result = std::vector<std::shared_ptr<fb::game::character>>();

    for (auto obj : nears)
    {
        if (obj->is(OBJECT_TYPE::CHARACTER) == false)
            continue;

        auto ch = std::static_pointer_cast<fb::game::character>(obj);
        if (ch->clan_id().has_value() == false)
            continue;

        auto& clan_id = ch->clan_id().value();
        this->_context.clans.read(clan_id, [ch, clan_id, &result](const auto& clan) {
            if (ch->clan_id() == clan_id)
                result.push_back(ch);
        }); // same thread
    }

    return result;
}