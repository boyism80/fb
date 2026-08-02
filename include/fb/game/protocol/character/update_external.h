#ifndef __PROTOCOL_GAME_CHARACTER_SHOW_H__
#define __PROTOCOL_GAME_CHARACTER_SHOW_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#include <fb/game/appearance.h>
#endif

namespace fb::protocol::game::response {

#ifndef BOT

using namespace fb::game;

template <bool Detailed>
struct appearance_serializer
{
    uint32_t                   oid;
    fb::model::point<uint16_t> position;
    DIRECTION                  direction;
    HEAD_MARKER                head_marker;
    std::string                name;
    character_appearance       appearance;

    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        if constexpr (Detailed)
        {
            writer.write<uint16_t>(this->position.x);
            writer.write<uint16_t>(this->position.y);
            writer.write<uint8_t>(static_cast<uint8_t>(this->direction));
        }

        writer.write<uint32_t>(this->oid);
        writer.write<uint8_t>(this->appearance.disguise.has_value());
        writer.write<uint8_t>(static_cast<uint8_t>(this->appearance.gender));
        writer.write<uint8_t>(static_cast<uint8_t>(this->appearance.state.value_or(STATE::NORMAL)));
        if (this->appearance.disguise.has_value())
        {
            writer.write<uint16_t>(this->appearance.disguise.value());
            writer.write<uint8_t>(this->appearance.hair_color.value_or(this->appearance.armor_color.value_or(0x00)));
        }
        else
        {
            writer.write<uint16_t>(this->appearance.hair);
            writer.write<uint8_t>(this->appearance.hair_color.value_or(0x00));

            if (this->appearance.armor.has_value())
            {
                writer.write<uint8_t>(this->appearance.armor.value());
                writer.write<uint8_t>(this->appearance.armor_color.value_or(0x00));
            }
            else
            {
                writer.write<uint8_t>(static_cast<uint8_t>(this->appearance.gender));
                writer.write<uint8_t>(0x00);
            }

            if (this->appearance.weapon.has_value())
            {
                writer.write<uint16_t>(this->appearance.weapon.value());
                writer.write<uint8_t>(this->appearance.weapon_color.value_or(0x00));
            }
            else
            {
                writer.write<uint16_t>(0xFFFF);
                writer.write<uint8_t>(0x00);
            }

            if (this->appearance.shield.has_value())
            {
                writer.write<uint8_t>(this->appearance.shield.value());
                writer.write<uint8_t>(this->appearance.shield_color.value_or(0x00));
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
    static constexpr uint8_t opcode = Detailed ? 0x33 : 0x1D;

public:
#ifndef BOT
    appearance_serializer<Detailed> serializer;
#else
    uint16_t    x;
    uint16_t    y;
    uint8_t     direction;
    uint32_t    oid;
    uint8_t     disguised;
    GENDER      gender;
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
        serializer(appearance_serializer<Detailed>{.oid         = ch.oid(),
                                                   .position    = ch.position(),
                                                   .direction   = ch.direction(),
                                                   .head_marker = head_marker(ch, to),
                                                   .name        = ch.name(),
                                                   .appearance  = character_appearance()})
    {
        if (ch.mimicry().has_value())
        {
            serializer.appearance = ch.mimicry().value();
            if (serializer.appearance.state.has_value() == false)
                serializer.appearance.state = ch.state();
        }
        else
        {
            serializer.appearance.gender      = ch.gender();
            serializer.appearance.state       = ch.state();
            serializer.appearance.hair        = ch.look();
            serializer.appearance.hair_color  = ch.color();
            serializer.appearance.armor_color = ch.armor_color();
            serializer.appearance.disguise    = std::nullopt;

            if (ch.items.armor() != nullptr)
            {
                serializer.appearance.armor = static_cast<uint8_t>(ch.items.armor()->model().dress);
                if (serializer.appearance.armor_color.has_value() == false)
                    serializer.appearance.armor_color = ch.items.armor()->model().color;
            }

            if (ch.items.weapon() != nullptr)
            {
                serializer.appearance.weapon = ch.items.weapon()->model().dress;
                serializer.appearance.weapon_color =
                    ch.weapon_color().value_or(static_cast<uint8_t>(ch.items.weapon()->color()));
            }

            if (ch.items.shield() != nullptr)
            {
                serializer.appearance.shield       = ch.items.shield()->model().dress;
                serializer.appearance.shield_color = ch.shield_color().value_or(ch.items.shield()->color());
            }
        }

        serializer.appearance.state = ch.state_to(to, serializer.appearance.state.value_or(ch.state()));
    }
    update_external(const appearance_serializer<Detailed>& serializer) :
        serializer(serializer)
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
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        header::serialize(writer);
        writer.write<uint8_t>(opcode); // Use compile-time constant header
        this->serializer.serialize(writer);
    }
#else
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        header::deserialize(reader);

        if constexpr (Detailed)
        {
            this->x         = reader.read<uint16_t>();
            this->y         = reader.read<uint16_t>();
            this->direction = reader.read<uint8_t>();
        }

        this->oid       = reader.read<uint32_t>();
        this->disguised = reader.read<uint8_t>();
        this->gender    = static_cast<GENDER>(reader.read<uint8_t>());
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