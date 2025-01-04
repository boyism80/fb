#include <fb/gateway/protocol/crypto.h>

namespace fb::protocol::gateway::response {

#ifndef BOT
crypto::crypto(const fb::crypto& crt, uint32_t crc) :
    crt(crt),
    crc(crc)
{ }
#endif

#ifndef BOT
async::task<void> crypto::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x00);
    writer.write<uint32_t>(this->crc);
    writer.write<uint8_t>(this->crt.type());
    writer.write<uint8_t>(0x09);
    writer.write(this->crt.key(), 0x09);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> crypto::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    reader.read<uint8_t>();

    auto crc      = reader.read<uint32_t>();
    auto enc_type = reader.read<uint8_t>();
    auto size     = reader.read<uint8_t>();
    auto enc_key  = new uint8_t[size];
    reader.read(enc_key, size);
    this->crt = fb::crypto(enc_type, enc_key);
    delete[] enc_key;
}
#endif

} // namespace fb::protocol::gateway::response
