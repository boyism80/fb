#include <clan.h>

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

const std::string& clan::name() const
{
    return this->_name;
}

const std::optional<std::string>& clan::title() const
{
    return this->_title;
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