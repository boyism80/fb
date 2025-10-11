#include <fb/game/protocol/trade/trade_upload.h>

namespace fb::protocol::game::response {

#ifndef BOT
trade_upload::trade_upload(uint8_t index, const fb::game::item& item, bool mine) :
    index(index),
    item(item),
    mine(mine)
{ }
#endif

#ifndef BOT
async::task<void> trade_upload::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x02);
    writer.write<uint8_t>(this->mine ? 0x00 : 0x01);
    writer.write<uint8_t>(this->index); // trade slot index
    writer.write<uint16_t>(this->item.look());
    writer.write<uint8_t>(this->item.color());
    writer.write<std::string>(this->item.trade_name());
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> trade_upload::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    reader.read<uint8_t>(); // 0x02
    uint8_t mine_flag = reader.read<uint8_t>();
    this->mine        = (mine_flag == 0x00);
    this->index       = reader.read<uint8_t>();
    this->look        = reader.read<uint16_t>();
    this->color       = reader.read<uint8_t>();
    this->name        = reader.read<std::string, uint8_t>();
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
