#include <fb/game/protocol/unknown_4D.h>

namespace fb::protocol::game::response {

#ifndef BOT
void unknown_4D::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->type);
    if (this->type == 2)
    {
        for (size_t i = 0; i < this->strings.size(); ++i)
        {
            writer.write<std::string, uint8_t>(this->strings[i]);
        }
    }
}
#else
void unknown_4D::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->type = reader.read<uint8_t>();
    if (this->type == 2)
    {
        for (size_t i = 0; i < this->strings.size(); ++i)
        {
            this->strings[i] = reader.read<std::string, uint8_t>();
        }
    }
}
#endif

} // namespace fb::protocol::game::response
