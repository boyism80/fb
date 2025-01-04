#include <fb/game/protocol/item/item_update.h>

namespace fb::protocol::game::response {

#ifndef BOT
item_update::item_update(const fb::game::character& me, uint8_t index) :
    me(me),
    index(index)
{ }
#else

#endif

#ifndef BOT
async::task<void> item_update::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    auto item = this->me.items.at(index);
    if (item == nullptr)
        co_return;

    writer.write<uint8_t>(header);
    writer.write<uint8_t>(this->index + 1);
    writer.write<uint16_t>(item->look());
    writer.write<uint8_t>(item->color());
    writer.write<std::string, uint8_t>(item->inven_name());
    writer.write<uint32_t>(item->count() - item->trade_count());
    writer.write<uint8_t>(0x00);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> item_update::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response
