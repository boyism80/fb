#include <fb/game/protocol/character/update_internal.h>

namespace fb::protocol::game::response {

#ifndef BOT
update_internal::update_internal(const fb::game::character& ch, STATE_LEVEL level) :
    ch(ch),
    level(level)
{ }
#endif

#ifndef BOT
async::task<void> update_internal::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(static_cast<uint8_t>(this->level));

    if (ENUM_IN(this->level, STATE_LEVEL::BASED))
    {
        writer.write<uint8_t>(static_cast<uint8_t>(this->ch.nation()));   // nation
        writer.write<uint8_t>(static_cast<uint8_t>(this->ch.creature())); // creature
        writer.write<uint8_t>(0x00);                                      // Unknown (clan?)
        writer.write<uint8_t>(this->ch.level());                          // level
        writer.write<uint32_t>(this->ch.stat.maxhp());                    // base hp
        writer.write<uint32_t>(this->ch.stat.maxmp());                    // base mp
        writer.write<uint8_t>(this->ch.stat.str());
        writer.write<uint8_t>(this->ch.stat.intelligence());
        writer.write<uint8_t>(0x03);
        writer.write<uint8_t>(0x03);
        writer.write<uint8_t>(this->ch.stat.dex());
        writer.write<uint8_t>(0x03);
        writer.write<uint32_t>(0x00);
        writer.write<uint8_t>(0x00);
    }

    if (ENUM_IN(this->level, STATE_LEVEL::HP_MP))
    {
        writer.write<uint32_t>(this->ch.stat.hp()); // current hp
        writer.write<uint32_t>(this->ch.stat.mp()); // current mp
    }

    if (ENUM_IN(this->level, STATE_LEVEL::EXP_MONEY))
    {
        writer.write<uint32_t>(this->ch.exp());                            // exp
        writer.write<uint32_t>(this->ch.money() - this->ch.trade.money()); // money
    }

    if (ENUM_IN(this->level, STATE_LEVEL::CROWD_CONTROL))
    {
        writer.write<uint8_t>(this->ch.cc.contains(CROWD_CONTROL::DIRECTION));
        writer.write<uint8_t>(this->ch.cc.contains(CROWD_CONTROL::SIGHT));
        writer.write<uint8_t>(this->ch.cc.contains(CROWD_CONTROL::HEAR));
        writer.write<uint8_t>(this->ch.cc.contains(CROWD_CONTROL::CHAT));
        writer.write<uint8_t>(this->ch.cc.contains(CROWD_CONTROL::MAP));
    }

    writer.write<uint8_t>(this->ch.unread_mail());
    writer.write<uint8_t>(this->ch.option(OPTION::FAST_MOVE));
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> update_internal::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->level = static_cast<fb::model::enum_value::STATE_LEVEL>(reader.read<uint8_t>());
    if (ENUM_IN(this->level, STATE_LEVEL::BASED))
    {
        this->ch_nation   = reader.read<uint8_t>();
        this->ch_creature = reader.read<uint8_t>();
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

    if (ENUM_IN(this->level, STATE_LEVEL::HP_MP))
    {
        this->ch_hp = reader.read<uint32_t>();
        this->ch_mp = reader.read<uint32_t>();
    }

    if (ENUM_IN(this->level, STATE_LEVEL::EXP_MONEY))
    {
        this->ch_exp   = reader.read<uint32_t>();
        this->ch_money = reader.read<uint32_t>();
    }

    if (ENUM_IN(this->level, STATE_LEVEL::CROWD_CONTROL))
    {
        if (reader.read<uint8_t>())
            this->ch_crowd_control |= (uint32_t)CROWD_CONTROL::DIRECTION;
        if (reader.read<uint8_t>())
            this->ch_crowd_control |= (uint32_t)CROWD_CONTROL::SIGHT;
        if (reader.read<uint8_t>())
            this->ch_crowd_control |= (uint32_t)CROWD_CONTROL::HEAR;
        if (reader.read<uint8_t>())
            this->ch_crowd_control |= (uint32_t)CROWD_CONTROL::CHAT;
        if (reader.read<uint8_t>())
            this->ch_crowd_control |= (uint32_t)CROWD_CONTROL::MAP;
    }

    this->ch_mail      = reader.read<uint8_t>();
    this->ch_fast_move = reader.read<uint8_t>();
    std::ignore        = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
