#include <fb/login/protocol/complete.h>
#include <string>

namespace fb::protocol::login::request {

#ifndef BOT
template <CLIENT_VERSION V>
void complete<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->hair         = reader.read<uint8_t>();
    this->gender       = reader.read<uint8_t>();
    this->nation       = reader.read<uint8_t>();
    this->divine_beast = reader.read<uint8_t>();
}

template <>
void complete<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    if (reader.readable_size() >= 6)
    {
        auto pos    = reader.seek();
        auto packed = reader.read<uint16_t>();
        if (packed >= 200 && packed <= 255)
        {
            this->face         = static_cast<uint8_t>(packed - 200);
            this->hair         = reader.read<uint8_t>();
            this->gender       = reader.read<uint8_t>();
            this->nation       = reader.read<uint8_t>();
            this->divine_beast = reader.read<uint8_t>();
            if (reader.readable_size() > 0)
                reader.read<std::string, uint8_t>();
            while (reader.readable_size() > 0)
                reader.read<uint8_t>();
            return;
        }
        reader.seek(pos);
    }

    this->hair         = reader.read<uint8_t>();
    this->gender       = reader.read<uint8_t>();
    this->nation       = reader.read<uint8_t>();
    this->divine_beast = reader.read<uint8_t>();
    this->face         = 0;
    if (reader.readable_size() > 0)
        reader.read<std::string, uint8_t>();
    while (reader.readable_size() > 0)
        reader.read<uint8_t>();
}
#else
template <CLIENT_VERSION V>
complete<V>::complete(uint8_t hair, uint8_t gender, uint8_t nation, uint8_t divine_beast, uint8_t face) :
    hair(hair),
    gender(gender),
    nation(nation),
    divine_beast(divine_beast),
    face(face)
{ }

template <CLIENT_VERSION V>
void complete<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->hair);
    writer.write<uint8_t>(this->gender);
    writer.write<uint8_t>(this->nation);
    writer.write<uint8_t>(this->divine_beast);
}

template <>
void complete<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>(static_cast<uint16_t>(200 + this->face));
    writer.write<uint8_t>(this->hair);
    writer.write<uint8_t>(this->gender);
    writer.write<uint8_t>(this->nation);
    writer.write<uint8_t>(this->divine_beast);
}
#endif

template class complete<CLIENT_VERSION::v550>;
template class complete<CLIENT_VERSION::v565>;
template class complete<CLIENT_VERSION::v651>;

} // namespace fb::protocol::login::request
