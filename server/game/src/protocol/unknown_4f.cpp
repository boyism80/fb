#include <fb/game/protocol/unknown_4f.h>

namespace fb::protocol::game::response {

#ifndef BOT
unknown_4f<CLIENT_VERSION::v651>::unknown_4f(uint32_t                      token,
                                             uint32_t                      body_a,
                                             uint8_t                       alloc,
                                             uint8_t                       flag,
                                             uint32_t                      body_b,
                                             std::vector<unknown_4f_entry> entries) :
    token(token),
    body_a(body_a),
    alloc(alloc),
    flag(flag),
    body_b(body_b),
    entries(std::move(entries))
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void unknown_4f<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
}

void unknown_4f<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // 1
    this->token = reader.read<uint32_t>();
    reader.read<uint8_t>(); // 0
    this->body_a = reader.read<uint32_t>();
    this->alloc  = reader.read<uint8_t>();
    auto n       = reader.read<uint8_t>();
    this->flag   = reader.read<uint8_t>();
    this->body_b = reader.read<uint32_t>();
    this->entries.clear();
    this->entries.reserve(n);
    for (uint8_t i = 0; i < n; ++i)
    {
        unknown_4f_entry e;
        e.id = reader.read<uint32_t>();
        if (e.id == 0)
        {
            e.pad = reader.read<uint32_t>();
        }
        else
        {
            e.field   = reader.read<uint16_t>();
            e.kind    = reader.read<uint8_t>();
            e.name    = reader.read<std::string, uint8_t>();
            e.percent = reader.read<uint8_t>();
            e.a       = reader.read<uint32_t>();
            e.b       = reader.read<uint32_t>();
            e.c       = reader.read<uint32_t>();
        }
        this->entries.push_back(std::move(e));
    }
}
#else
template <CLIENT_VERSION V>
void unknown_4f<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
}

void unknown_4f<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(1);
    writer.write<uint32_t>(this->token);
    writer.write<uint8_t>(0);
    writer.write<uint32_t>(this->body_a);
    writer.write<uint8_t>(this->alloc);
    writer.write<uint8_t>(static_cast<uint8_t>(this->entries.size()));
    writer.write<uint8_t>(this->flag);
    writer.write<uint32_t>(this->body_b);
    for (const auto& e : this->entries)
    {
        writer.write<uint32_t>(e.id);
        if (e.id == 0)
        {
            writer.write<uint32_t>(e.pad);
        }
        else
        {
            writer.write<uint16_t>(e.field);
            writer.write<uint8_t>(e.kind);
            writer.write<std::string, uint8_t>(e.name);
            writer.write<uint8_t>(e.percent);
            writer.write<uint32_t>(e.a);
            writer.write<uint32_t>(e.b);
            writer.write<uint32_t>(e.c);
        }
    }
}
#endif

template class unknown_4f<CLIENT_VERSION::v550>;
template class unknown_4f<CLIENT_VERSION::v565>;

} // namespace fb::protocol::game::response
