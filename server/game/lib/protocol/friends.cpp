#include <fb/game/protocol/friends.h>

namespace fb::protocol::game::request {

#ifdef BOT
async::task<void> friends::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
}
#else
async::task<void> friends::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);

    auto count = reader.read<uint8_t>();
    char buffer[12];
    for (int i = 0; i < count; i++)
    {
        auto len = reader.read<uint8_t>();
        if (len != 12)
            co_return;

        reader.read(buffer, sizeof(buffer));
        auto name = std::string(buffer, sizeof(buffer));
        this->names.push_back(name);
    }
}
#endif

} // namespace fb::protocol::game::request
