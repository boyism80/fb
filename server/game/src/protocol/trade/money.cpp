#include <fb/game/protocol/trade/money.h>

namespace fb::protocol::game::response {

#ifndef BOT
trade_money::trade_money(uint32_t money, bool mine) :
    money(money),
    mine(mine)
{ }
#endif

#ifndef BOT
void trade_money::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x03);
    writer.write<bool>(!this->mine);
    writer.write<uint32_t>(this->money);
    writer.write<uint8_t>(0x00);
}
#else
void trade_money::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // 0x03
    this->mine  = !reader.read<bool>();
    this->money = reader.read<uint32_t>();
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
