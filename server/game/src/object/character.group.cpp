#include <fb/game/character.h>

#include <fb/game/server.h>

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>

using namespace fb::game;

character::group::group(character& owner) :
    _owner(owner)
{ }

const std::optional<uint64_t>& character::group::id() const
{
    this->_owner.assert_thread();
    return this->_id;
}

bool character::group::enter(fb::game::group& group)
{
    this->_owner.assert_thread();

    if (this->_id == group.id() || group.enter(this->_owner.weak_from_this_as<character>()) == false)
        return false;

    this->_id = group.id();
    return true;
}

async::task<void> character::group::leave(std::string message)
{
    this->_owner.assert_thread();

    this->detach();
    this->_owner.server.groups.clear_portraits(this->_owner);
    co_await this->_owner.matchmaker.drop_ticket();
    this->_owner.message(message, MESSAGE_TYPE::STATE);

    auto map = this->_owner.map();
    if (map != nullptr && map->is_instance() &&
        map->model().instance_rule == fb::model::enum_value::INSTANCE_RULE_TYPE::GROUP)
    {
        std::ignore = co_await map->kick(this->_owner);
    }
}

void character::group::detach()
{
    this->_owner.assert_thread();

    if (this->_id.has_value() == false)
        return;

    auto group_id = std::exchange(this->_id, std::nullopt).value();
    try
    {
        this->_owner.server.groups.write(group_id, [weak = this->_owner.weak_from_this_as<character>()](auto& group) {
            if (group != nullptr)
                group->detach(weak);
        });
    }
    catch (const std::runtime_error&)
    { }

    this->_owner.server.groups.update_portraits(group_id);
}

void character::group::refresh_portrait()
{
    this->_owner.assert_thread();

    if (this->_id.has_value())
        this->_owner.server.groups.update_portraits(this->_id.value());
}
