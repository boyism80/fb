#include <fb/game/protocol/life/update_hp.h>

namespace fb::protocol::game::response {

#ifndef BOT
update_hp::update_hp(const fb::game::life& me, uint32_t damage, bool critical) :
    me(me),
    damage(damage),
    critical(critical)
{ }
#endif

#ifndef BOT
void update_hp::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);

    auto percent = (uint8_t)std::ceil((this->me.stat.hp() / static_cast<double>(this->me.stat.base_hp())) * 100);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->me.oid());
    writer.write<uint8_t>(this->critical);
    writer.write<uint8_t>(percent);
    writer.write<uint32_t>(this->damage);
    writer.write<uint8_t>(0x00);
}
#else
void update_hp::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->oid      = reader.read<uint32_t>();
    this->critical = reader.read<uint8_t>();
    this->percent  = reader.read<uint8_t>();
    this->damage   = reader.read<uint32_t>();
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
