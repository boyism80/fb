#include <fb/game/protocol/life/update_hp.h>

namespace fb::protocol::game::response {

#ifndef BOT
update_hp::update_hp(const fb::game::life& me, uint32_t damage, bool critical) :
    me(me),
    damage(damage),
    critical(critical),
    percentage(static_cast<uint8_t>(this->me.hp() / float(this->me.base_hp()) * 100))
{ }
#endif

#ifndef BOT
async::task<void> update_hp::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint32_t>(this->me.sequence());
    writer.write<uint8_t>(this->critical);
    writer.write<uint8_t>(this->percentage);
    writer.write<uint32_t>(this->damage);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> update_hp::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response
