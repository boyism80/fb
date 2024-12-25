#ifndef __PROTOCOL_REQUEST_GAME_MAP_H__
#define __PROTOCOL_REQUEST_GAME_MAP_H__

#include <fb/protocol/protocol.h>

namespace fb::protocol::game::request::map {

class update : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x05;

public:
#ifndef BOT
    point16_t position;
    size8_t   size;
    uint16_t  crc;
#else

#endif

public:
    update() = default;

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        // TODO: serialize bytes
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->position.x  = reader.read<uint16_t>();
        this->position.y  = reader.read<uint16_t>();
        this->size.width  = reader.read<uint8_t>();
        this->size.height = reader.read<uint8_t>();
        this->crc         = reader.read<uint16_t>();
    }
#endif
};

class world : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x3F;

public:
#ifndef BOT
    uint16_t value;
    uint16_t before;
    uint16_t after;
#else

#endif

public:
    world() = default;

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        // TODO: serialize bytes
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->value  = reader.read<uint16_t>();
        this->before = reader.read<uint16_t>();
        this->after  = reader.read<uint16_t>();
    }
#endif
};

} // namespace fb::protocol::game::request::map

#endif // !__PROTOCOL_REQUEST_GAME_MAP_H__