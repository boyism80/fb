#include <fb/game/protocol/friends.h>

namespace fb::protocol::game::request {

#ifdef BOT
async::task<void> friends::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#else
async::task<void> friends::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);

    auto count = reader.read<uint8_t>();
    for (int i = 0; i < count; i++)
    {
        auto name = reader.read<std::string, uint8_t>();
        this->names.push_back(name);
    }
}
#endif

} // namespace fb::protocol::game::request
