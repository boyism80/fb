#include <fb/game/protocol/item/combine.h>

namespace fb::protocol::game::request {

#ifdef BOT
void item_combine::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->indices.size()));
    for (const auto& index : this->indices)
    {
        writer.write<uint8_t>(index + 1);
    }
}
#else
void item_combine::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    auto count = reader.read<uint8_t>();
    for (int i = 0; i < count; i++)
    {
        this->indices.push_back(reader.read<uint8_t>() - 1);
    }
}
#endif
} // namespace fb::protocol::game::request
