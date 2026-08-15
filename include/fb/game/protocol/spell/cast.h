#ifndef __PROTOCOL_GAME_USE_H__
#define __PROTOCOL_GAME_USE_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <string_view>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class spell_cast : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x0F;
    FB_PROTOCOL_VERSION_TAGS(V);

private:
    fb::stream buffer;

public:
#ifdef BOT
    const SPELL_TYPE                 type;
    const uint8_t                    slot;
    const std::string                message;
    const uint32_t                   oid;
    const fb::model::point<uint16_t> position;
#else
    uint8_t                    slot;
    std::string                message;
    uint32_t                   oid;
    fb::model::point<uint16_t> position;
#endif

public:
#ifndef BOT
    spell_cast() = default;
#else
    spell_cast(SPELL_TYPE                        type,
               uint8_t                           slot,
               std::string_view                  message,
               uint32_t                          oid,
               const fb::model::point<uint16_t>& position);
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif

    void parse(SPELL_TYPE type);
};

} // namespace fb::protocol::game::request

#endif