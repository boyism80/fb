#include <fb/game/protocol/attack.h>
#include <algorithm>

namespace fb::protocol::game::request {

#ifndef BOT
template <CLIENT_VERSION V>
void attack<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
}

void attack<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    if (reader.readable_size() == 0)
        return;

    this->kind = reader.read<uint8_t>();
    if (this->kind == 2 || this->kind == 3)
    {
        this->delay = reader.read<uint8_t>();
        auto n      = reader.read<uint8_t>();
        auto count  = std::min<uint8_t>(n, 9);
        this->oids.clear();
        this->oids.reserve(count);
        for (uint8_t i = 0; i < count; ++i)
            this->oids.push_back(reader.read<uint32_t>());
    }
}
#else
template <CLIENT_VERSION V>
void attack<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}

void attack<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(4);
}
#endif

template class attack<CLIENT_VERSION::v550>;
template class attack<CLIENT_VERSION::v565>;

} // namespace fb::protocol::game::request
