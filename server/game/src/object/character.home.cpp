#include <fb/game/character.h>

#include <fb/config.h>
#include <fb/game/map.h>
#include <fb/game/server.h>
#include <fb/logger.h>
#include <fb/model/model.h>
#include <fb/protocol/flatbuffer/protocol.h>

#include <cstdint>
#include <optional>

using namespace fb::game;
using namespace fb::model;

character::home::home(character& owner) :
    _owner(owner)
{ }

const std::optional<character::home::spot>& character::home::pinned() const
{
    return this->_spot;
}

bool character::home::pin()
{
    this->_owner.assert_thread();

    auto map = this->_owner.map();
    if (map == nullptr)
        return false;

    this->_spot = spot{map->model().id, this->_owner.position()};
    return true;
}

void character::home::unpin()
{
    this->_owner.assert_thread();
    this->_spot = std::nullopt;
}

void character::home::restore(uint32_t map, const fb::model::point16_t& position)
{
    this->_owner.assert_thread();
    if (fb::config<std::optional<uint32_t>>("world"))
        return;

    this->_spot = spot{map, position};
}

async::task<bool> character::home::recall()
{
    this->_owner.assert_thread();

    auto dest = this->_spot.has_value() ? this->_owner.server.maps.find(this->_spot->map) : nullptr;
    if (fb::config<std::optional<uint32_t>>("world"))
    {
        if (dest == nullptr)
            co_return false;

        co_return co_await this->_owner.map(dest, this->_spot->position);
    }

    auto result = transfer_result::FAILED;
    if (dest != nullptr)
        result = co_await this->_owner.transfer(*dest, this->_spot->position);

    if (result != transfer_result::STARTED && this->_owner.transferring() == false)
        this->disconnect();

    co_return result == transfer_result::STARTED;
}

void character::home::disconnect()
{
    fb::logger::warn("Character {} could not return home from the cross server; disconnecting", this->_owner.name());
    if (auto socket_ptr = this->_owner.socket_ptr(); socket_ptr != nullptr)
        socket_ptr->close();
}

bool character::home::savable() const
{
    return fb::config<std::optional<uint32_t>>("world") || this->_spot.has_value();
}

void character::home::to_protocol(fb::protocol::internal::Character& dto) const
{
    auto cross = !fb::config<std::optional<uint32_t>>("world");
    auto map   = this->_owner.map();
    if (this->_spot.has_value())
    {
        dto.map      = this->_spot->map;
        dto.position = fb::protocol::internal::Position{this->_spot->position.x, this->_spot->position.y};
    }
    else if (cross)
    {
        fb::logger::fatal("Character {} cross to_protocol without home snapshot", this->_owner.name());
        dto.map      = 0;
        dto.position = fb::protocol::internal::Position{1, 1};
    }
    else if (map != nullptr && map->model().return_to.has_value())
    {
        auto return_map_id = map->model().return_to.value();
        dto.map            = return_map_id;
        auto spawn         = fb::model::point16_t{0, 0};
        if (table::map->contains(return_map_id))
            spawn = table::map[return_map_id].spawn_position().value_or(fb::model::point16_t{0, 0});

        dto.position = fb::protocol::internal::Position{spawn.x, spawn.y};
    }
    else if (map != nullptr)
    {
        auto position = this->_owner.position();
        dto.map       = map->model().id;
        dto.position  = fb::protocol::internal::Position{position.x, position.y};
    }
    else
    {
        dto.map      = 0;
        dto.position = fb::protocol::internal::Position{1, 1};
    }

    if (cross && this->_owner.state() == STATE::GHOST)
    {
        dto.state = static_cast<uint8_t>(STATE::NORMAL);
        dto.hp    = revived_hp(dto.hp);
    }
}
