#include <fb/gateway/protocol/encryption.h>

namespace fb::protocol::gateway::response {

#ifdef BOT
encryption::encryption(const encryption& other) :
    cryptor(other.cryptor),
    crc(other.crc)
{ }

encryption::encryption(encryption&& other) :
    cryptor(std::move(other.cryptor)),
    crc(other.crc)
{ }
#else
encryption::encryption(const fb::encryption& cryptor, uint32_t crc) :
    cryptor(cryptor),
    crc(crc)
{ }
#endif

#ifndef BOT
async::task<void> encryption::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x00);
    writer.write<uint32_t>(this->crc);
    writer.write<uint8_t>(this->cryptor.pattern());
    writer.write<uint8_t>(fb::encryption::KEY_SIZE);
    writer.write(this->cryptor.iv(), fb::encryption::KEY_SIZE);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> encryption::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    reader.read<uint8_t>();

    auto crc     = reader.read<uint32_t>();
    auto pattern = reader.read<uint8_t>();
    auto size    = reader.read<uint8_t>();
    auto iv      = std::make_unique<uint8_t[]>(size);
    reader.read(iv.get(), size);
    this->cryptor = fb::encryption(pattern, iv.get());
}
#endif

} // namespace fb::protocol::gateway::response
