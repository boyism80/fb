#include <fb/login/protocol/terms_agreement.h>
#include <stdexcept>

namespace fb::protocol::login::request {

#ifndef BOT
void agreement::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->enc_type     = reader.read<uint8_t>();
    this->enc_key_size = reader.read<uint8_t>();
    reader.read(this->enc_key, this->enc_key_size);

    // Trailing transfer fields echoed by the stock client (gateway→login hop).
    this->from  = reader.read<uint8_t>();
    auto packed = reader.read<uint16_t>();
    if (try_parse(packed, this->client_version) == false)
        throw std::runtime_error("invalid client version in agreement transfer");
}
#else
agreement::agreement(uint8_t type, uint8_t ksize, const uint8_t* key, uint8_t from, CLIENT_VERSION client_version) :
    enc_type(type),
    enc_key_size(ksize),
    from(from),
    client_version(client_version)
{
    memcpy(this->enc_key, key, ksize);
}

void agreement::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->enc_type);
    writer.write<uint8_t>(this->enc_key_size);
    writer.write((const void*)this->enc_key, this->enc_key_size);
    writer.write<uint8_t>(this->from);
    writer.write<uint16_t>(static_cast<uint16_t>(this->client_version));
}
#endif

} // namespace fb::protocol::login::request

namespace fb::protocol::login::response {

#ifndef BOT
terms_agreement::terms_agreement(std::string_view contents) :
    contents(std::string(contents))
{ }
#endif

#ifndef BOT
void terms_agreement::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    auto compressed = fb::stream((uint8_t*)this->contents.data(), this->contents.size()).compress();
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x01);
    writer.write<uint16_t>((uint16_t)compressed.size());
    writer.write(compressed.data(), (uint16_t)compressed.size());
}
#else
void terms_agreement::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>();
    auto size   = reader.read<uint16_t>();
    auto buffer = std::vector<uint8_t>(size); // RAII
    reader.read(buffer.data(), size);

    auto decompressed = fb::stream(buffer.data(), size).decompress();

    decompressed.push_back(0);
    this->contents = std::string((const char*)decompressed.data());
}
#endif

} // namespace fb::protocol::login::response
