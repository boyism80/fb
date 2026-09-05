#include <fb/login/protocol/name_list.h>

namespace fb::protocol::login::response {

#ifndef BOT
name_list<CLIENT_VERSION::v651>::name_list(uint8_t                                       subtype,
                                           uint16_t                                      count,
                                           std::string                                   name,
                                           uint32_t                                      crc,
                                           std::string                                   body,
                                           std::vector<std::pair<std::string, uint32_t>> names) :
    subtype(subtype),
    count(count),
    name(std::move(name)),
    crc(crc),
    body(std::move(body)),
    names(std::move(names))
{ }
#endif

#ifdef BOT
void name_list<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->subtype = reader.read<uint8_t>();
    if (this->subtype == 0)
    {
        this->name = reader.read<std::string, uint8_t>();
        this->crc  = reader.read<uint32_t>();
        this->body = reader.read<std::string, uint16_t>();
    }
    else
    {
        this->count = reader.read<uint16_t>();
        this->names.clear();
        for (uint16_t i = 0; i < this->count; ++i)
        {
            auto n = reader.read<std::string, uint8_t>();
            auto c = reader.read<uint32_t>();
            this->names.emplace_back(std::move(n), c);
        }
    }
}
#else
void name_list<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->subtype);
    if (this->subtype == 0)
    {
        writer.write<std::string, uint8_t>(this->name);
        writer.write<uint32_t>(this->crc);
        writer.write<std::string, uint16_t>(this->body);
    }
    else
    {
        auto n = this->names.empty() ? this->count : static_cast<uint16_t>(this->names.size());
        writer.write<uint16_t>(n);
        for (const auto& [nm, c] : this->names)
        {
            writer.write<std::string, uint8_t>(nm);
            writer.write<uint32_t>(c);
        }
    }
}
#endif

} // namespace fb::protocol::login::response
