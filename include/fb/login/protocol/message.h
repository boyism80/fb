#ifndef __PROTOCOL_LOGIN_MESSAGE_H__
#define __PROTOCOL_LOGIN_MESSAGE_H__

namespace fb::protocol::login::response {

class message : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x02;

public:
#ifdef BOT
    std::string text;
    uint8_t     type;
#else
    const std::string text;
    const uint8_t     type;
#endif

public:
#ifdef BOT
    message() = default;
#else
    message(const std::string& text, uint8_t type) :
        text(text),
        type(type)
    { }
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->type);
        writer.write<std::string, uint8_t>(this->text);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->type = reader.read<uint8_t>();
        this->text = reader.read<std::string, uint8_t>();
    }
#endif
};

} // namespace fb::protocol::login::response

#endif