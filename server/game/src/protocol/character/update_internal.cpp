#include <fb/game/protocol/character/update_internal.h>
#include <fb/game/client_amount.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
update_internal<V>::update_internal(const fb::game::character& ch, UPDATE_STATE_LEVEL level) :
    ch(ch),
    level(level)
{ }

template <CLIENT_VERSION V>
void update_internal<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);

    auto flags = this->level;
    if (this->ch.option(OPTION::FIXED_MOVE) == false)
        flags |= UPDATE_STATE_LEVEL::FOLLOW_CAMERA;
    if (this->ch.option(OPTION::LOCK_WALK_SPEED))
        flags |= UPDATE_STATE_LEVEL::LOCK_WALK_SPEED;
    writer.write<uint8_t>(static_cast<uint8_t>(flags));

    auto [encoded_hp, encoded_maxhp] = fb::game::encode_client_pool(this->ch.stat.hp(), this->ch.stat.maxhp());
    auto [encoded_mp, encoded_maxmp] = fb::game::encode_client_pool(this->ch.stat.mp(), this->ch.stat.maxmp());

    if (ENUM_IN(this->level, UPDATE_STATE_LEVEL::BASED))
    {
        writer.write<uint8_t>(static_cast<uint8_t>(this->ch.nation()));
        writer.write<uint8_t>(static_cast<uint8_t>(this->ch.divine_beast()));
        writer.write<uint8_t>(0x00);
        writer.write<uint8_t>(this->ch.level());
        writer.write<uint32_t>(encoded_maxhp);
        writer.write<uint32_t>(encoded_maxmp);
        writer.write<uint8_t>(this->ch.stat.str());
        writer.write<uint8_t>(this->ch.stat.intelligence());
        writer.write<uint8_t>(0x03);
        writer.write<uint8_t>(0x03);
        writer.write<uint8_t>(this->ch.stat.dex());
        writer.write<uint8_t>(0x03);
        writer.write<uint32_t>(0x00);
        writer.write<uint8_t>(0x00);
    }

    if (ENUM_IN(this->level, UPDATE_STATE_LEVEL::HP_MP))
    {
        writer.write<uint32_t>(encoded_hp);
        writer.write<uint32_t>(encoded_mp);
    }

    if (ENUM_IN(this->level, UPDATE_STATE_LEVEL::EXP_MONEY))
    {
        writer.write<uint32_t>(fb::game::encode_client_amount(this->ch.exp()));
        writer.write<uint32_t>(fb::game::encode_client_amount(this->ch.money() - this->ch.trade.money()));
    }

    if (ENUM_IN(this->level, UPDATE_STATE_LEVEL::CROWD_CONTROL))
    {
        writer.write<bool>(this->ch.cc.contains(CROWD_CONTROL::DIRECTION));
        writer.write<bool>(this->ch.cc.contains(CROWD_CONTROL::SIGHT));
        writer.write<bool>(this->ch.cc.contains(CROWD_CONTROL::HEAR));
        writer.write<bool>(this->ch.cc.contains(CROWD_CONTROL::CHAT));
        writer.write<bool>(this->ch.cc.contains(CROWD_CONTROL::MAP));
    }

    writer.write<uint8_t>(this->ch.mail_box.unread_count());
    writer.write<bool>(this->ch.option(OPTION::FAST_MOVE));
    writer.write<uint8_t>(this->ch.stat.speed());
}

template <>
void update_internal<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);

    auto flags = this->level;
    if (this->ch.option(OPTION::FIXED_MOVE) == false)
        flags |= UPDATE_STATE_LEVEL::FOLLOW_CAMERA;
    if (this->ch.option(OPTION::LOCK_WALK_SPEED))
        flags |= UPDATE_STATE_LEVEL::LOCK_WALK_SPEED;
    writer.write<uint8_t>(static_cast<uint8_t>(flags));

    auto [encoded_hp, encoded_maxhp] = fb::game::encode_client_pool(this->ch.stat.hp(), this->ch.stat.maxhp());
    auto [encoded_mp, encoded_maxmp] = fb::game::encode_client_pool(this->ch.stat.mp(), this->ch.stat.maxmp());

    if (ENUM_IN(this->level, UPDATE_STATE_LEVEL::BASED))
    {
        // nation u16 wire = stored+1
        writer.write<uint16_t>(static_cast<uint16_t>(this->ch.nation()) + 1);
        writer.write<uint8_t>(static_cast<uint8_t>(this->ch.divine_beast())); // totem @+4
        writer.write<uint8_t>(this->unknown_based_5);                         // +5 NOT READ
        writer.write<uint8_t>(this->ch.level());
        writer.write<uint32_t>(encoded_maxhp);
        writer.write<uint32_t>(encoded_maxmp);
        writer.write<uint8_t>(this->ch.stat.str());
        writer.write<uint8_t>(this->ch.stat.intelligence());
        writer.write<uint8_t>(0x00); // +17 NOT READ
        writer.write<uint8_t>(0x00); // +18 NOT READ
        writer.write<uint8_t>(this->ch.stat.dex());
        writer.write<uint8_t>(0x00); // +20
        writer.write<uint8_t>(0x00); // +21
        writer.write<uint8_t>(0x00); // +22
        writer.write<uint8_t>(0x00); // +23
        writer.write<uint8_t>(0x00); // +24
        writer.write<uint8_t>(0x00); // +25
        writer.write<uint16_t>(this->unknown_based_26);
        writer.write<uint16_t>(this->unknown_based_28);
    }

    if (ENUM_IN(this->level, UPDATE_STATE_LEVEL::HP_MP))
    {
        writer.write<uint32_t>(encoded_hp);
        writer.write<uint32_t>(encoded_mp);
    }

    if (ENUM_IN(this->level, UPDATE_STATE_LEVEL::EXP_MONEY))
    {
        writer.write<uint32_t>(fb::game::encode_client_amount(this->ch.exp()));
        writer.write<uint32_t>(fb::game::encode_client_amount(this->ch.money() - this->ch.trade.money()));
        writer.write<uint8_t>(this->unknown_exp_pad); // CharStats skips +9
    }

    if (ENUM_IN(this->level, UPDATE_STATE_LEVEL::CROWD_CONTROL))
    {
        writer.write<bool>(this->ch.cc.contains(CROWD_CONTROL::DIRECTION));
        writer.write<bool>(this->ch.cc.contains(CROWD_CONTROL::SIGHT));
        writer.write<bool>(this->ch.cc.contains(CROWD_CONTROL::HEAR));
        writer.write<bool>(this->ch.cc.contains(CROWD_CONTROL::CHAT));
        // 5th bool skipped by client (not stored); write pad 0
        writer.write<uint8_t>(0x00);
    }

    // mail skipped by CharStats; still present in span for other consumers — write then fast_move + option_bits
    writer.write<uint8_t>(this->ch.mail_box.unread_count()); // skipped by CharStats cursor
    writer.write<bool>(this->ch.option(OPTION::FAST_MOVE));
    writer.write<uint32_t>(this->unknown_option_bits);
}

template update_internal<CLIENT_VERSION::v550>::update_internal(const fb::game::character&, UPDATE_STATE_LEVEL);
template update_internal<CLIENT_VERSION::v565>::update_internal(const fb::game::character&, UPDATE_STATE_LEVEL);
template update_internal<CLIENT_VERSION::v651>::update_internal(const fb::game::character&, UPDATE_STATE_LEVEL);
template void update_internal<CLIENT_VERSION::v550>::serialize(fb::stream_writer<big_endian>&) const;
template void update_internal<CLIENT_VERSION::v565>::serialize(fb::stream_writer<big_endian>&) const;
#else
template <CLIENT_VERSION V>
void update_internal<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->level              = static_cast<fb::model::enum_value::UPDATE_STATE_LEVEL>(reader.read<uint8_t>());
    this->ch_follow_camera   = ENUM_IN(this->level, UPDATE_STATE_LEVEL::FOLLOW_CAMERA);
    this->ch_lock_walk_speed = ENUM_IN(this->level, UPDATE_STATE_LEVEL::LOCK_WALK_SPEED);
    this->ch_gm              = ENUM_IN(this->level, UPDATE_STATE_LEVEL::GM);
    if (ENUM_IN(this->level, UPDATE_STATE_LEVEL::BASED))
    {
        this->ch_nation       = reader.read<uint8_t>();
        this->ch_divine_beast = reader.read<uint8_t>();
        reader.read<uint8_t>();
        this->ch_level        = reader.read<uint8_t>();
        this->ch_base_hp      = reader.read<uint32_t>();
        this->ch_base_mp      = reader.read<uint32_t>();
        this->ch_strength     = reader.read<uint8_t>();
        this->ch_intelligence = reader.read<uint8_t>();
        reader.read<uint8_t>();
        reader.read<uint8_t>();
        this->ch_dexterity = reader.read<uint8_t>();
        reader.read<uint8_t>();
        reader.read<uint32_t>();
        reader.read<uint8_t>();
    }

    if (ENUM_IN(this->level, UPDATE_STATE_LEVEL::HP_MP))
    {
        this->ch_hp = reader.read<uint32_t>();
        this->ch_mp = reader.read<uint32_t>();
    }

    if (ENUM_IN(this->level, UPDATE_STATE_LEVEL::EXP_MONEY))
    {
        this->ch_exp   = reader.read<uint32_t>();
        this->ch_money = reader.read<uint32_t>();
    }

    if (ENUM_IN(this->level, UPDATE_STATE_LEVEL::CROWD_CONTROL))
    {
        if (reader.read<bool>())
            this->ch_crowd_control |= (uint32_t)CROWD_CONTROL::DIRECTION;
        if (reader.read<bool>())
            this->ch_crowd_control |= (uint32_t)CROWD_CONTROL::SIGHT;
        if (reader.read<bool>())
            this->ch_crowd_control |= (uint32_t)CROWD_CONTROL::HEAR;
        if (reader.read<bool>())
            this->ch_crowd_control |= (uint32_t)CROWD_CONTROL::CHAT;
        if (reader.read<bool>())
            this->ch_crowd_control |= (uint32_t)CROWD_CONTROL::MAP;
    }

    this->ch_mail      = reader.read<uint8_t>();
    this->ch_fast_move = reader.read<bool>();
    this->ch_speed     = reader.read<uint8_t>();
}

template <>
void update_internal<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->level              = static_cast<fb::model::enum_value::UPDATE_STATE_LEVEL>(reader.read<uint8_t>());
    this->ch_follow_camera   = ENUM_IN(this->level, UPDATE_STATE_LEVEL::FOLLOW_CAMERA);
    this->ch_lock_walk_speed = ENUM_IN(this->level, UPDATE_STATE_LEVEL::LOCK_WALK_SPEED);
    this->ch_gm              = ENUM_IN(this->level, UPDATE_STATE_LEVEL::GM);
    if (ENUM_IN(this->level, UPDATE_STATE_LEVEL::BASED))
    {
        this->ch_nation       = reader.read<uint16_t>();
        this->ch_divine_beast = reader.read<uint8_t>();
        this->unknown_based_5 = reader.read<uint8_t>();
        this->ch_level        = reader.read<uint8_t>();
        this->ch_base_hp      = reader.read<uint32_t>();
        this->ch_base_mp      = reader.read<uint32_t>();
        this->ch_strength     = reader.read<uint8_t>();
        this->ch_intelligence = reader.read<uint8_t>();
        reader.read<uint8_t>();
        reader.read<uint8_t>();
        this->ch_dexterity = reader.read<uint8_t>();
        reader.read<uint8_t>();
        reader.read<uint8_t>();
        reader.read<uint8_t>();
        reader.read<uint8_t>();
        reader.read<uint8_t>();
        reader.read<uint8_t>();
        this->unknown_based_26 = reader.read<uint16_t>();
        this->unknown_based_28 = reader.read<uint16_t>();
    }

    if (ENUM_IN(this->level, UPDATE_STATE_LEVEL::HP_MP))
    {
        this->ch_hp = reader.read<uint32_t>();
        this->ch_mp = reader.read<uint32_t>();
    }

    if (ENUM_IN(this->level, UPDATE_STATE_LEVEL::EXP_MONEY))
    {
        this->ch_exp          = reader.read<uint32_t>();
        this->ch_money        = reader.read<uint32_t>();
        this->unknown_exp_pad = reader.read<uint8_t>();
    }

    if (ENUM_IN(this->level, UPDATE_STATE_LEVEL::CROWD_CONTROL))
    {
        if (reader.read<bool>())
            this->ch_crowd_control |= (uint32_t)CROWD_CONTROL::DIRECTION;
        if (reader.read<bool>())
            this->ch_crowd_control |= (uint32_t)CROWD_CONTROL::SIGHT;
        if (reader.read<bool>())
            this->ch_crowd_control |= (uint32_t)CROWD_CONTROL::HEAR;
        if (reader.read<bool>())
            this->ch_crowd_control |= (uint32_t)CROWD_CONTROL::CHAT;
        reader.read<uint8_t>();
    }

    this->ch_mail             = reader.read<uint8_t>();
    this->ch_fast_move        = reader.read<bool>();
    this->unknown_option_bits = reader.read<uint32_t>();
}

template void update_internal<CLIENT_VERSION::v550>::deserialize(fb::stream_reader<big_endian>&);
template void update_internal<CLIENT_VERSION::v565>::deserialize(fb::stream_reader<big_endian>&);
#endif

} // namespace fb::protocol::game::response
