#ifndef __PROTOCOL_REQUEST_GAME_ITEM_H__
#define __PROTOCOL_REQUEST_GAME_ITEM_H__

#include <fb/protocol/protocol.h>
#include <fb/model/model.h>
#ifndef BOT
#include <item.h>
#endif

#ifndef BOT
using namespace fb::game;
#endif
using namespace fb::model::enum_value;

namespace fb::protocol::game::request::item {

class active : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x1C;

public:
    uint8_t index;

public:
    active() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->index = reader.read<uint8_t>() - 1;
    }
};

class inactive : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x1F;

public:
    EQUIPMENT_PARTS parts;

public:
    inactive() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->parts = EQUIPMENT_PARTS(reader.read<uint8_t>());
    }
};

class drop : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x08;

public:
    uint8_t index;
    bool    all;

public:
    drop() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->index = reader.read<uint8_t>() - 1;
        this->all   = bool(reader.read<uint8_t>());
    }
};

class drop_cash : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x24;

public:
    uint32_t chunk;

public:
    drop_cash() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->chunk = reader.read<uint32_t>();
    }
};

class mix : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x6B;

public:
    std::vector<uint8_t> indices;

public:
    mix() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        auto count = reader.read<uint8_t>();
        for (int i = 0; i < count; i++)
            this->indices.push_back(reader.read<uint8_t>());
    }
};

class throws : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x17;

public:
    bool    all;
    uint8_t index;

public:
    throws() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->all   = reader.read<uint8_t>();
        this->index = reader.read<uint8_t>() - 1;
    }
};

class info : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x66;

public:
    uint16_t position;
    uint8_t  slot;

public:
    info() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->position = reader.read<uint16_t>();
        auto unknown1  = reader.read<uint8_t>();
        auto unknown2  = reader.read<uint8_t>();
        auto unknown3  = reader.read<uint8_t>();
        this->slot     = reader.read<uint8_t>() - 1;
    }
};

} // namespace fb::protocol::game::request::item

#endif // !__PROTOCOL_REQUEST_GAME_ITEM_H__