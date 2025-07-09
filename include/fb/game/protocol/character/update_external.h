#ifndef __PROTOCOL_GAME_CHARACTER_SHOW_H__
#define __PROTOCOL_GAME_CHARACTER_SHOW_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

// Template implementation for compile-time optimization
template <bool Detailed>
class update_external : public fb::protocol::header
{
public:
    static constexpr uint8_t header = Detailed ? 0x33 : 0x1D;

public:
#ifndef BOT
    const fb::game::character& ch;
    const fb::game::object&    to;
#else
    uint16_t    x;
    uint16_t    y;
    uint8_t     direction;
    uint32_t    oid;
    uint8_t     disguised;
    uint8_t     sex;
    STATE       state;
    uint16_t    look;
    uint8_t     color;
    uint8_t     armor_dress;
    uint8_t     armor_color;
    uint16_t    weapon_dress;
    uint8_t     weapon_color;
    uint8_t     shield_dress;
    uint8_t     shield_color;
    uint8_t     head_marker;
    std::string name;
#endif

public:
#ifndef BOT
    update_external(const fb::game::character& ch, const fb::game::object& to) :
        ch(ch),
        to(to)
    { }
    update_external(const update_external&) = delete;
#else
    update_external() = default;
#endif

private:
#ifndef BOT
    HEAD_MARKER head_marker() const
    {
        if (&this->ch == &this->to)
            return HEAD_MARKER::NONE;

        if (this->to.is(OBJECT_TYPE::CHARACTER) == false)
            return HEAD_MARKER::NONE;

        auto& you = static_cast<const fb::game::character&>(this->to);
        auto  is_diff_clan =
            (this->ch.clan_id().has_value() || you.clan_id().has_value()) && this->ch.clan_id() != you.clan_id();
        if (!is_diff_clan)
            return HEAD_MARKER::NONE;

        return HEAD_MARKER::NONE;
    }
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        auto map = this->ch.map();
        if (map == nullptr)
            co_return;

        writer.write<uint8_t>(header); // Use compile-time constant header

        if constexpr (Detailed)
        {
            writer.write<uint16_t>(this->ch.x());                              // x
            writer.write<uint16_t>(this->ch.y());                              // y
            writer.write<uint8_t>(static_cast<uint8_t>(this->ch.direction())); // side
        }

        writer.write<uint32_t>(this->ch.oid());
        writer.write<uint8_t>(this->ch.state() == STATE::DISGUISE);  // 변신유무
        writer.write<uint8_t>(static_cast<uint8_t>(this->ch.sex())); // sex
        writer.write<uint8_t>(static_cast<uint8_t>(this->ch.state_to(this->to)));
        if (this->ch.state() == STATE::DISGUISE)
        {
            writer.write<uint16_t>(this->ch.disguise().value());
            writer.write<uint8_t>(this->ch.current_armor_color());
        }
        else
        {
            writer.write<uint16_t>(this->ch.look()); // face
            writer.write<uint8_t>(this->ch.color()); // hair color

            auto armor = this->ch.items.armor();
            if (armor != nullptr)
            {
                writer.write<uint8_t>((uint8_t)armor->based<fb::model::armor>().dress);
                writer.write<uint8_t>(ch.current_armor_color());
            }
            else
            {
                writer.write<uint8_t>(static_cast<uint8_t>(this->ch.sex()));
                writer.write<uint8_t>(0x00);
            }

            auto weapon = this->ch.items.weapon();
            if (weapon != nullptr)
            {
                writer.write<uint16_t>(weapon->based<fb::model::weapon>().dress);
                writer.write<uint8_t>(weapon->color());
            }
            else
            {
                writer.write<uint16_t>(0xFFFF);
                writer.write<uint8_t>(0x00);
            }

            auto shield = this->ch.items.shield();
            if (shield != nullptr)
            {
                writer.write<uint8_t>((uint8_t)shield->based<fb::model::shield>().dress);
                writer.write<uint8_t>(shield->color());
            }
            else
            {
                writer.write<uint8_t>(0xFF); // about shield
                writer.write<uint8_t>(0x00);
            }
        }

        writer.write<uint8_t>(static_cast<uint8_t>(this->head_marker())); // head mark
        writer.write<std::string, uint8_t>(this->ch.name());              // name
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);

        if constexpr (Detailed)
        {
            this->x         = reader.read<uint16_t>();
            this->y         = reader.read<uint16_t>();
            this->direction = reader.read<uint8_t>();
        }

        this->oid       = reader.read<uint32_t>();
        this->disguised = reader.read<uint8_t>();
        this->sex       = reader.read<uint8_t>();
        this->state     = static_cast<STATE>(reader.read<uint8_t>());

        if (this->disguised)
        {
            this->look  = reader.read<uint16_t>();
            this->color = reader.read<uint8_t>();
        }
        else
        {
            this->look         = reader.read<uint16_t>();
            this->color        = reader.read<uint8_t>();
            this->armor_dress  = reader.read<uint8_t>();
            this->armor_color  = reader.read<uint8_t>();
            this->weapon_dress = reader.read<uint16_t>();
            this->weapon_color = reader.read<uint8_t>();
            this->shield_dress = reader.read<uint8_t>();
            this->shield_color = reader.read<uint8_t>();
        }

        this->head_marker = reader.read<uint8_t>();
        this->name        = reader.read<std::string, uint8_t>();
    }
#endif
};

// Type aliases for convenience
using update_external_detailed = update_external<true>;  // 0x33 header with position info
using update_external_brief    = update_external<false>; // 0x1D header without position info

} // namespace fb::protocol::game::response

#endif