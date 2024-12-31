#include <fb/game/protocol/trade/trade_upload.h>

namespace fb::protocol::game::response {

#ifndef BOT
trade_upload::trade_upload(const fb::game::character& me, uint8_t index, bool mine) :
    me(me),
    index(index),
    mine(mine)
{ }
#endif

#ifndef BOT
async::task<void> trade_upload::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    const auto item = this->me.trade.item(this->index);

    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x02);
    writer.write<uint8_t>(this->mine ? 0x00 : 0x01);
    writer.write<uint8_t>(this->index); // trade slot index
    writer.write<uint16_t>(item->look());
    writer.write<uint8_t>(item->color());
    writer.write<std::string>(item->trade_name());
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> trade_upload::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response
