#include <fb/game/protocol/user_info_submit.h>

namespace fb::protocol::game::request {

#ifdef BOT
void user_info_submit::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->sub);
    for (size_t i = 0; i < this->strings.size(); ++i)
    {
        writer.write<std::string, uint8_t>(this->strings[i]);
    }
}
#else
void user_info_submit::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->sub = reader.read<uint8_t>();
    for (size_t i = 0; i < this->strings.size(); ++i)
    {
        this->strings[i] = reader.read<std::string, uint8_t>();
    }
}
#endif

} // namespace fb::protocol::game::request
