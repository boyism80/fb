#ifndef __PROTOCOL_LOGIN_COMPLETE_H__
#define __PROTOCOL_LOGIN_COMPLETE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::login::request {

using namespace fb::model::enum_value;

class complete : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x04;

public:
#ifndef BOT
    uint8_t hair;
    uint8_t sex;
    uint8_t nation;
    uint8_t creature;
#else
    const uint8_t hair;
    const uint8_t sex;
    const uint8_t nation;
    const uint8_t creature;
#endif

public:
#ifndef BOT
    complete() = default;
#else
    complete(uint8_t hair, uint8_t sex, uint8_t nation, uint8_t creature);
#endif

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::login::request

#endif