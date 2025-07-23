#include <fb/game/protocol/trade/trade_money.h>

namespace fb::protocol::game::response {

#ifndef BOT
trade_money::trade_money(uint32_t money, bool mine) :
    money(money),
    mine(mine)
{ }
#endif

#ifndef BOT
async::task<void> trade_money::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x03);
    writer.write<uint8_t>(this->mine ? 0x00 : 0x01);
    writer.write<uint32_t>(this->money);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> trade_money::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    reader.read<uint8_t>(); // 0x03
    uint8_t mine_flag = reader.read<uint8_t>();
    this->mine        = (mine_flag == 0x00);
    this->money       = reader.read<uint32_t>();
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
