#include <fb/game/protocol/user_list.h>

namespace fb::protocol::game::request {

async::task<void> user_list::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    auto unknown = reader.read<uint8_t>();
}

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

#ifndef BOT
user_list::user_list(const character& me, container& sockets) :
    me(me),
    sockets(sockets)
{ }
#endif

#ifndef BOT
async::task<void> user_list::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    this->sockets.lock([&writer, &me = this->me](auto& container) {
        writer.write<uint16_t>((uint16_t)container.size());
        writer.write<uint16_t>((uint16_t)container.size());
        writer.write<uint8_t>(0x00);

        for (auto& [fd, socket] : container)
        {
            auto  ch   = socket->data();
            auto& name = ch->name();

            writer.write<uint8_t>(0x10 * static_cast<int>(ch->nation()));
            writer.write<uint8_t>(0x10 * static_cast<int>(ch->promotion()));
            writer.write<uint8_t>((&me == ch) ? 0x88 : 0x0F);
            writer.write<std::string, uint8_t>(name);
        }
    });
}
#else
async::task<void> user_list::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response