#include <fb/game/character.h>

#include <fb/game/server.h>

#include <cstdint>
#include <optional>

using namespace fb::game;

character::group::group(character& owner) :
    _owner(owner)
{ }

const std::optional<uint64_t>& character::group::id() const
{
    this->_owner.assert_thread();
    return this->_id;
}

void character::group::id(uint64_t value)
{
    this->_owner.assert_thread();
    this->_id = value;
}

void character::group::reset()
{
    this->_owner.assert_thread();
    this->_id.reset();
}

void character::group::refresh_portrait()
{
    this->_owner.assert_thread();

    if (this->_id.has_value())
        this->_owner.server.groups.update_portraits(this->_id.value());
}
