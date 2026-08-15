#include <fb/game/protocol/user_info.h>

namespace fb::protocol::game::response {

#ifndef BOT
user_info::user_info(uint8_t type, const std::array<std::string, 8>& strings) :
    type(type),
    strings(strings)
{ }
#endif

#ifndef BOT
void user_info::serialize(fb::stream_writer<big_endian>& writer) const
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
void user_info::deserialize(fb::stream_reader<big_endian>& reader)
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
