#ifndef __PROTOCOL_LOGIN_COMPLETE_H__
#define __PROTOCOL_LOGIN_COMPLETE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <fb/protocol/client_version.h>

namespace fb::protocol::login::request {

using namespace fb::model::enum_value;

/**
 * C2S create character complete (opcode 0x04).
 * Primary layout (v550): hair u8 | gender u8 | nation u8 | divine_beast u8.
 * No version delta known for v565/v651.
 */
template <CLIENT_VERSION V>
class complete : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x04;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    uint8_t hair;
    uint8_t gender;
    uint8_t nation;
    uint8_t divine_beast;
#else
    const uint8_t hair;
    const uint8_t gender;
    const uint8_t nation;
    const uint8_t divine_beast;
#endif

public:
#ifndef BOT
    complete() = default;
#else
    complete(uint8_t hair, uint8_t gender, uint8_t nation, uint8_t divine_beast);
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::login::request

#endif