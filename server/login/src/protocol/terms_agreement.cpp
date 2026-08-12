#include <fb/login/protocol/terms_agreement.h>
#include <stdexcept>

namespace fb::protocol::login::request {

#ifndef BOT
template <CLIENT_VERSION V>
void agreement<V>::deserialize(fb::stream_reader<big_endian>& reader)
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

    // Login 6.51 always appends NEW/OLD after the echoed blob.
    if constexpr (V == CLIENT_VERSION::v651)
    {
        auto flag = reader.read<uint8_t>();
        if (try_parse(flag, this->ui_mode) == false)
            throw std::runtime_error("invalid client ui mode in agreement transfer");
    }
}
#else
template <CLIENT_VERSION V>
agreement<V>::agreement(uint8_t        type,
                        uint8_t        ksize,
                        const uint8_t* key,
                        uint8_t        from,
                        CLIENT_VERSION client_version,
                        CLIENT_UI_MODE ui_mode) :
    enc_type(type),
    enc_key_size(ksize),
    from(from),
    client_version(client_version),
    ui_mode(ui_mode)
{
    memcpy(this->enc_key, key, ksize);
}

template <CLIENT_VERSION V>
void agreement<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->enc_type);
    writer.write<uint8_t>(this->enc_key_size);
    writer.write((const void*)this->enc_key, this->enc_key_size);
    writer.write<uint8_t>(this->from);
    writer.write<uint16_t>(static_cast<uint16_t>(this->client_version));
    if constexpr (V == CLIENT_VERSION::v651)
        writer.write<uint8_t>(static_cast<uint8_t>(this->ui_mode));
}
#endif

template class agreement<CLIENT_VERSION::v550>;
template class agreement<CLIENT_VERSION::v565>;
template class agreement<CLIENT_VERSION::v651>;

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
