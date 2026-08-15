#include <fb/game/protocol/unknown_6f.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
unknown_6f<V>::unknown_6f(uint8_t subtype, uint16_t count) :
    subtype(subtype),
    count(count)
{ }

unknown_6f<CLIENT_VERSION::v651>::unknown_6f(uint8_t                                       subtype,
                                             uint16_t                                      count,
                                             std::string                                   name,
                                             uint32_t                                      value,
                                             std::string                                   body,
                                             std::vector<std::pair<std::string, uint32_t>> names) :
    subtype(subtype),
    count(count),
    name(std::move(name)),
    value(value),
    body(std::move(body)),
    names(std::move(names))
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void unknown_6f<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->subtype = reader.read<uint8_t>();
    this->count   = reader.read<uint16_t>();
}

void unknown_6f<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->subtype = reader.read<uint8_t>();
    if (this->subtype == 0)
    {
        this->name  = reader.read<std::string, uint8_t>();
        this->value = reader.read<uint32_t>();
        this->body  = reader.read<std::string, uint16_t>();
    }
    else
    {
        this->count = reader.read<uint16_t>();
        this->names.clear();
        for (uint16_t i = 0; i < this->count; ++i)
        {
            auto n = reader.read<std::string, uint8_t>();
            auto v = reader.read<uint32_t>();
            this->names.emplace_back(std::move(n), v);
        }
    }
}
#else
template <CLIENT_VERSION V>
void unknown_6f<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->subtype);
    writer.write<uint16_t>(this->count);
}

void unknown_6f<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->subtype);
    if (this->subtype == 0)
    {
        writer.write<std::string, uint8_t>(this->name);
        writer.write<uint32_t>(this->value);
        writer.write<std::string, uint16_t>(this->body);
    }
    else
    {
        auto n = this->names.empty() ? this->count : static_cast<uint16_t>(this->names.size());
        writer.write<uint16_t>(n);
        for (const auto& [nm, v] : this->names)
        {
            writer.write<std::string, uint8_t>(nm);
            writer.write<uint32_t>(v);
        }
    }
}
#endif

template class unknown_6f<CLIENT_VERSION::v550>;
template class unknown_6f<CLIENT_VERSION::v565>;

} // namespace fb::protocol::game::response
