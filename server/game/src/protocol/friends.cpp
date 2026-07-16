#include <fb/game/protocol/friends.h>

namespace fb::protocol::game::request {

#ifdef BOT
void friends::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}
#else
void friends::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);

    auto count = reader.read<uint8_t>();
    for (int i = 0; i < count; i++)
    {
        auto name = reader.read<std::string, uint8_t>();
        this->names.push_back(name);
    }
}
#endif

} // namespace fb::protocol::game::request
