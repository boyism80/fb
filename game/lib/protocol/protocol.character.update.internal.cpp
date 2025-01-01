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

    if (enum_in(this->level, STATE_LEVEL::BASED))
    {
        writer.write<uint8_t>(static_cast<uint8_t>(this->ch.nation()));   // nation
        writer.write<uint8_t>(static_cast<uint8_t>(this->ch.creature())); // creature
        writer.write<uint8_t>(0x00);                                      // Unknown (clan?)
        writer.write<uint8_t>(this->ch.level());                          // level
        writer.write<uint32_t>(this->ch.base_hp());                       // base hp
        writer.write<uint32_t>(this->ch.base_mp());                       // base mp
        writer.write<uint8_t>(this->ch.strength());
        writer.write<uint8_t>(this->ch.intelligence());
        writer.write<uint8_t>(0x03);
        writer.write<uint8_t>(0x03);
        writer.write<uint8_t>(this->ch.dexteritry());
        writer.write<uint8_t>(0x03);
        writer.write<uint32_t>(0x00);
        writer.write<uint8_t>(0x00);
    }

    if (enum_in(this->level, STATE_LEVEL::HP_MP))
    {
        writer.write<uint32_t>(this->ch.hp()); // current hp
        writer.write<uint32_t>(this->ch.mp()); // current mp
    }

    if (enum_in(this->level, STATE_LEVEL::EXP_MONEY))
    {
        writer.write<uint32_t>(this->ch.exp());                            // exp
        writer.write<uint32_t>(this->ch.money() - this->ch.trade.money()); // money
    }

    if (enum_in(this->level, STATE_LEVEL::CONDITION))
    {
        writer.write<uint8_t>(this->ch.condition_contains(CONDITION::MOVE));  // condition::move
        writer.write<uint8_t>(this->ch.condition_contains(CONDITION::SIGHT)); // condition::sight
        writer.write<uint8_t>(this->ch.condition_contains(CONDITION::HEAR));  // condition::hear?
        writer.write<uint8_t>(this->ch.condition_contains(CONDITION::ORAL));  // condition:oral
        writer.write<uint8_t>(this->ch.condition_contains(CONDITION::MAP));   // condition:map?
    }

    writer.write<uint8_t>(this->ch.mailed); // mail count
    writer.write<uint8_t>(true);            // fast move
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> update_internal::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->level = static_cast<fb::model::enum_value::STATE_LEVEL>(reader.read<uint8_t>());
    if (enum_in(this->level, STATE_LEVEL::BASED))
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
        this->ch_dexteritry = reader.read<uint8_t>();
        reader.read<uint8_t>();
        reader.read<uint32_t>();
        reader.read<uint8_t>();
    }

    if (enum_in(this->level, STATE_LEVEL::HP_MP))
    {
        this->ch_hp = reader.read<uint32_t>();
        this->ch_mp = reader.read<uint32_t>();
    }

    if (enum_in(this->level, STATE_LEVEL::EXP_MONEY))
    {
        this->ch_exp   = reader.read<uint32_t>();
        this->ch_money = reader.read<uint32_t>();
    }

    if (enum_in(this->level, STATE_LEVEL::CONDITION))
    {
        if (reader.read<uint8_t>())
            this->ch_condition |= (uint32_t)CONDITION::MOVE;
        if (reader.read<uint8_t>())
            this->ch_condition |= (uint32_t)CONDITION::SIGHT;
        if (reader.read<uint8_t>())
            this->ch_condition |= (uint32_t)CONDITION::HEAR;
        if (reader.read<uint8_t>())
            this->ch_condition |= (uint32_t)CONDITION::ORAL;
        if (reader.read<uint8_t>())
            this->ch_condition |= (uint32_t)CONDITION::MAP;
    }

    this->ch_mail      = reader.read<uint8_t>();
    this->ch_fast_move = reader.read<uint8_t>();
    std::ignore        = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
