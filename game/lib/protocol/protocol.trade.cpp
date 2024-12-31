#include <fb/game/protocol/trade.h>

namespace fb::protocol::game::request {

#ifdef BOT
async::task<void> trade::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    // TODO: serialize bytes
}
#else
async::task<void> trade::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->action = reader.read<uint8_t>();
    this->fd     = reader.read<uint32_t>();
    switch (static_cast<fb::game::trade::state>(this->action))
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
