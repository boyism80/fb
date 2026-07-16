#include <fb/game/protocol/update_option.h>

namespace fb::protocol::game::request {

#ifdef BOT
void update_option::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->option));
    if (this->option == OPTION::EXTENSION)
    {
        writer.write<bool>(this->ride);
    }
}
#else
void update_option::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->option = static_cast<OPTION>(reader.read<uint8_t>());
    if (this->option == OPTION::EXTENSION)
    {
        this->ride = reader.read<bool>();
    }
}
#endif
} // namespace fb::protocol::game::request
