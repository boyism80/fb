#ifndef __PROTOCOL_GAME_CHARACTER_SHOW_H__
#define __PROTOCOL_GAME_CHARACTER_SHOW_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#include <fb/game/portrait.h>
#endif

namespace fb::protocol::game::response {

#ifndef BOT
template <bool Detailed>
struct preset_serializer
{
    uint32_t                   oid;
    fb::model::point<uint16_t> position;
    DIRECTION                  direction;
    HEAD_MARKER                head_marker;
    std::string                name;
    character_portrait         portrait;

    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        if constexpr (Detailed)
        {
            writer.write<uint16_t>(this->position.x);
            writer.write<uint16_t>(this->position.y);
            writer.write<uint8_t>(static_cast<uint8_t>(this->direction));
        }

        writer.write<uint32_t>(this->oid);
        writer.write<uint8_t>(this->portrait.state == STATE::DISGUISE);
        writer.write<uint8_t>(static_cast<uint8_t>(this->portrait.sex));
        writer.write<uint8_t>(static_cast<uint8_t>(this->portrait.state));
        if (this->portrait.state == STATE::DISGUISE)
        {
            writer.write<uint16_t>(this->portrait.disguise.value());
            writer.write<uint8_t>(this->portrait.armor_color.value_or(0x00));
        }
        else
        {
            writer.write<uint16_t>(this->portrait.hair);
            writer.write<uint8_t>(this->portrait.hair_color);

            if (this->portrait.armor.has_value())
            {
                writer.write<uint8_t>(this->portrait.armor.value());
                writer.write<uint8_t>(this->portrait.armor_color.value_or(0x00));
            }
            else
            {
                writer.write<uint8_t>(static_cast<uint8_t>(this->portrait.sex));
                writer.write<uint8_t>(0x00);
            }

            if (this->portrait.weapon.has_value())
            {
                writer.write<uint16_t>(this->portrait.weapon.value());
                writer.write<uint8_t>(this->portrait.weapon_color.value_or(0x00));
            }
            else
            {
                writer.write<uint16_t>(0xFFFF);
                writer.write<uint8_t>(0x00);
            }

            if (this->portrait.shield.has_value())
            {
                writer.write<uint8_t>(this->portrait.shield.value());
                writer.write<uint8_t>(this->portrait.shield_color.value_or(0x00));
            }
            else
            {
                writer.write<uint8_t>(0xFF); // about shield
                writer.write<uint8_t>(0x00);
            }
        }

        writer.write<uint8_t>(static_cast<uint8_t>(this->head_marker));
        writer.write<std::string, uint8_t>(this->name);
    }
};
#endif

// Template implementation for compile-time optimization
template <bool Detailed>
class update_external : public fb::protocol::header
{
public:
    static constexpr uint8_t header = Detailed ? 0x33 : 0x1D;

public:
#ifndef BOT
    preset_serializer<Detailed> preset;
#else
    uint16_t    x;
    uint16_t    y;
    uint8_t     direction;
    uint32_t    oid;
    uint8_t     disguised;
    SEX         sex;
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
        preset(preset_serializer<Detailed>{
            .oid         = ch.oid(),
            .position    = ch.position(),
            .direction   = ch.direction(),
            .head_marker = head_marker(ch, to),
            .name        = ch.name(),
            .portrait =
                character_portrait(ch.sex(), ch.state(), ch.look(), ch.color(), std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt)})
    {
        if (ch.items.armor() != nullptr)
        {
            preset.portrait.armor       = ch.items.armor()->based<fb::model::armor>().dress;
            preset.portrait.armor_color = ch.armor_color();
        }

        if (ch.items.weapon() != nullptr)
        {
            preset.portrait.weapon       = ch.items.weapon()->based<fb::model::weapon>().dress;
            preset.portrait.weapon_color = std::nullopt;
        }

        if (ch.items.shield() != nullptr)
        {
            preset.portrait.shield       = ch.items.shield()->based<fb::model::shield>().dress;
            preset.portrait.shield_color = std::nullopt;
        }
    }
    update_external(const preset_serializer<Detailed>& preset) :
        preset(preset)
    { }
    update_external(const update_external&) = delete;
#else
    update_external() = default;
#endif

private:
#ifndef BOT
    static HEAD_MARKER head_marker(const fb::game::character& ch, const fb::game::object& to)
    {
        if (&ch == &to)
            return HEAD_MARKER::NONE;

        if (to.is(OBJECT_TYPE::CHARACTER) == false)
            return HEAD_MARKER::NONE;

        auto clan_id_1 = ch.clan_id();
        if (clan_id_1.has_value() == false)
            return HEAD_MARKER::NONE;

        auto& you       = static_cast<const fb::game::character&>(to);
        auto  clan_id_2 = you.clan_id();
        if (clan_id_2.has_value() == false)
            return HEAD_MARKER::NONE;

        if (clan_id_1 != clan_id_2)
            return HEAD_MARKER::NONE;

        return HEAD_MARKER::BLUE;
    }
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header); // Use compile-time constant header
        this->preset.serialize(writer);
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
        this->sex       = static_cast<SEX>(reader.read<uint8_t>());
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