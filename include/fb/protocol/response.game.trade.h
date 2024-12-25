#ifndef __PROTOCOL_RESPONSE_GAME_TRADE_H__
#define __PROTOCOL_RESPONSE_GAME_TRADE_H__

#include <fb/protocol/protocol.h>
#include <fb/model/model.h>
#ifndef BOT
#include <trade.h>
#endif

namespace fb::protocol::game::response::trade {

class dialog : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x42;

public:
#ifndef BOT
    const fb::game::character& me;
    const fb::model::model&    model;
#else

#endif

public:
#ifndef BOT
    dialog(const fb::game::character& me, const fb::model::model& model) :
        me(me),
        model(model)
    { }
#else
    dialog() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        const auto& cname = model.promotion[me.cls()][me.promotion()].name;

        std::stringstream sstream;
        sstream << this->me.name() << '(' << cname.c_str() << ')';

        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x00);
        writer.write<uint32_t>(this->me.sequence());
        writer.write<std::string>(sstream.str());
        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

class upload : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x42;

public:
#ifndef BOT
    const fb::game::character& me;
    const uint8_t              index;
    const bool                 mine;
#else

#endif

public:
#ifndef BOT
    upload(const fb::game::character& me, uint8_t index, bool mine) :
        me(me),
        index(index),
        mine(mine)
    { }
#else
    upload() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        const auto item = this->me.trade.item(this->index);

        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x02);
        writer.write<uint8_t>(this->mine ? 0x00 : 0x01);
        writer.write<uint8_t>(this->index); // trade slot index
        writer.write<uint16_t>(item->look());
        writer.write<uint8_t>(item->color());
        writer.write<std::string>(item->trade_name());
        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

class bundle : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x42;

public:
    bundle() = default;

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x01);
        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

class money : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x42;

public:
#ifndef BOT
    const fb::game::character& me;
    const bool                 mine;
#else

#endif

public:
#ifndef BOT
    money(const fb::game::character& me, bool mine) :
        me(me),
        mine(mine)
    { }
#else
    money() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x03);
        writer.write<uint8_t>(this->mine ? 0x00 : 0x01);
        writer.write<uint32_t>(this->me.trade.money());
        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

class close : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x42;

public:
#ifndef BOT
    const std::string& message;
#else

#endif

public:
#ifndef BOT
    close(const std::string& message) :
        message(message)
    { }
#else
    close() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x04);
        writer.write<std::string, uint16_t>(this->message);
        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

class lock : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x42;

public:
    lock() = default;

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x05);
        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

} // namespace fb::protocol::game::response::trade

#endif // !__PROTOCOL_RESPONSE_GAME_TRADE_H__