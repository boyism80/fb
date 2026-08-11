#include <fb/game/protocol/character/update_internal.h>
#include <fb/game/client_amount.h>

namespace fb::protocol::game::response {

#ifndef BOT
update_internal::update_internal(const fb::game::character& ch, UPDATE_STATE_LEVEL level) :
    ch(ch),
    level(level)
{ }
#endif

#ifndef BOT
void update_internal::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);

    // Section bits come from level; flag bits are derived from character state.
    // FOLLOW_CAMERA is inverted vs FIXED_MOVE: set 0x02 when FIXED_MOVE is off.
    auto flags = this->level;
    if (this->ch.option(OPTION::FIXED_MOVE) == false)
        flags |= UPDATE_STATE_LEVEL::FOLLOW_CAMERA;
    if (this->ch.option(OPTION::LOCK_WALK_SPEED))
        flags |= UPDATE_STATE_LEVEL::LOCK_WALK_SPEED;
    // if (this->ch.role() >= ROLE::ADMIN)
    //     flags |= UPDATE_STATE_LEVEL::GM;
    writer.write<uint8_t>(static_cast<uint8_t>(flags));

    auto [encoded_hp, encoded_maxhp] = fb::game::encode_client_pool(this->ch.stat.hp(), this->ch.stat.maxhp());
    auto [encoded_mp, encoded_maxmp] = fb::game::encode_client_pool(this->ch.stat.mp(), this->ch.stat.maxmp());

    if (ENUM_IN(this->level, UPDATE_STATE_LEVEL::BASED))
    {
        writer.write<uint8_t>(static_cast<uint8_t>(this->ch.nation()));       // nation
        writer.write<uint8_t>(static_cast<uint8_t>(this->ch.divine_beast())); // divine_beast
        writer.write<uint8_t>(0x00);                                          // mid-skip pad (unread)
        writer.write<uint8_t>(this->ch.level());                              // level
        writer.write<uint32_t>(encoded_maxhp);                                // base hp
        writer.write<uint32_t>(encoded_maxmp);                                // base mp
        writer.write<uint8_t>(this->ch.stat.str());
        writer.write<uint8_t>(this->ch.stat.intelligence());
        writer.write<uint8_t>(0x03); // mid-skip pad (unread)
        writer.write<uint8_t>(0x03); // mid-skip pad (unread)
        writer.write<uint8_t>(this->ch.stat.dex());
        writer.write<uint8_t>(0x03);  // mid-skip pad (unread)
        writer.write<uint32_t>(0x00); // mid-skip pad (unread)
        writer.write<uint8_t>(0x00);  // mid-skip pad (unread)
    }

    if (ENUM_IN(this->level, UPDATE_STATE_LEVEL::HP_MP))
    {
        writer.write<uint32_t>(encoded_hp); // current hp
        writer.write<uint32_t>(encoded_mp); // current mp
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
#else
void update_internal::deserialize(fb::stream_reader<big_endian>& reader)
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
#endif

} // namespace fb::protocol::game::response
