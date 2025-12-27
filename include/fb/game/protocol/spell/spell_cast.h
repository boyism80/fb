#ifndef __PROTOCOL_GAME_USE_H__
#define __PROTOCOL_GAME_USE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

class spell_cast : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x0F;

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
               const std::string&                message,
               uint32_t                          oid,
               const fb::model::point<uint16_t>& position) :
        type(type),
        slot(slot),
        message(message),
        oid(oid),
        position(position)
    { }
#endif

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif

    void parse(SPELL_TYPE type);
};

} // namespace fb::protocol::game::request

#endif