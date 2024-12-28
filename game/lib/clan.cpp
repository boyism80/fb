#include <clan.h>
#include <context.h>

using namespace fb::game;

clan::clan(context& context, uint32_t id) :
    _context(context),
    _id(id)
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
    this->_name    = name;
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

const std::unordered_map<uint32_t, fb::game::character*>& clan::characters() const
{
    return this->_characters;
}

void clan::attach_character(character& ch)
{
    if (this->_characters.contains(ch.id()))
        return;

    this->_characters.insert({ch.id(), &ch});
}

void clan::detach_character(character& ch)
{
    if (!this->_characters.contains(ch.id()))
        return;

    this->_characters.erase(ch.id());
}

std::vector<character*> clan::nears(const fb::game::map& map, const point16_t& position) const
{
    auto nears  = map.nears(position, OBJECT_TYPE::CHARACTER); // same thread
    auto result = std::vector<character*>();

    for (auto obj : nears)
    {
        if (obj->is(OBJECT_TYPE::CHARACTER) == false)
            continue;

        auto ch = static_cast<character*>(obj);
        if (ch->clan() == nullptr)
            continue;

        ch->clan()->lock([this, ch, &result](auto& clan) {
            if (&clan == this)
                result.push_back(ch);
        });
    }

    return result;
}