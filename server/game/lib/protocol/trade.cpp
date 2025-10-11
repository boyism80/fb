#include <fb/game/protocol/trade.h>

namespace fb::protocol::game::request {

#ifdef BOT // bot only
async::task<void> trade::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(static_cast<uint8_t>(this->action));
    writer.write<uint32_t>(this->oid);

    switch (this->action)
    {
    case state::UP_ITEM:
        writer.write<uint8_t>(this->parameter.index);
        break;

    case state::ITEM_COUNT:
        writer.write<uint16_t>(this->parameter.count);
        break;

    case state::UP_MONEY:
        writer.write<uint32_t>(this->parameter.money);
        break;
    }
}
#else // server only
async::task<void> trade::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->action = static_cast<fb::game::trade::state>(reader.read<uint8_t>());
    this->oid    = reader.read<uint32_t>();
    switch (this->action)
    {
    case fb::game::trade::state::UP_ITEM:
        this->parameter.index = reader.read<uint8_t>();
        break;

    case fb::game::trade::state::ITEM_COUNT:
        this->parameter.count = reader.read<uint16_t>();
        break;

    case fb::game::trade::state::UP_MONEY:
        this->parameter.money = reader.read<uint32_t>();
        break;
    }
}
#endif

} // namespace fb::protocol::game::request
