#ifndef __PROTOCOL_GAME_USE_H__
#define __PROTOCOL_GAME_USE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class spell_cast : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x0F;

private:
    fb::stream buffer;

public:
    mutable uint8_t                    slot;
    mutable std::string                message;
    mutable uint32_t                   fd;
    mutable fb::model::point<uint16_t> position;

public:
    spell_cast() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);

    void parse(SPELL_TYPE type);
};

} // namespace fb::protocol::game::request

#endif