#ifndef __PROTOCOL_GATEWAY_ENCRYPTION_H__
#define __PROTOCOL_GATEWAY_ENCRYPTION_H__

#include <fb/protocol/header.h>
#include <fb/encryption.h>

namespace fb::protocol::gateway::response {

/** Encryption key issuance packet (cmd 0x00). Sent by gateway/login to provide session encryption. */
class encryption : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode  = 0x00;
    static constexpr bool    decrypt = false;

public:
#ifdef BOT
    fb::encryption cryptor;
    uint32_t       crc;
#else
    const fb::encryption cryptor;
    const uint32_t       crc;
#endif

public:
#ifdef BOT
    encryption() = default;
    encryption(const encryption& other);
    encryption(encryption&& other);
#else
    encryption(const fb::encryption& cryptor, uint32_t crc);
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::gateway::response

#endif