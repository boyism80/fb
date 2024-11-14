#ifndef __PROTOCOL_RESPONSE_LOGIN_H__
#define __PROTOCOL_RESPONSE_LOGIN_H__

#include <fb/protocol/protocol.h>

namespace fb { namespace protocol { namespace login { namespace response {

class agreement : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x60;

public:
#ifdef BOT
    std::string contents;
#else
    const std::string contents;
#endif

public:
#ifdef BOT
    agreement() = default;
#else
    agreement(const std::string& contents) :
        contents(contents)
    { }
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        auto compressed = fb::stream((uint8_t*)this->contents.data(), this->contents.size()).compress();
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x01);
        writer.write<uint16_t>((uint16_t)compressed.size());
        writer.write(compressed.data(), (uint16_t)compressed.size());
    }
#else
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        reader.read<uint8_t>();
        auto size   = reader.read<uint16_t>();
        auto buffer = new uint8_t[size];
        reader.read(buffer, size);

        auto decompressed = fb::stream(buffer, size).decompress();
        delete[] buffer;

        decompressed.push_back(0);
        this->contents = std::string((const char*)decompressed.data());
    }
#endif
};

class message : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x02;

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
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->type);
        writer.write<std::string, uint8_t>(this->text);
    }
#else
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        this->type = reader.read_8();
        this->text = reader.read<std::string, uint8_t>();
    }
#endif
};

}}}} // namespace fb::protocol::login::response

#endif // !__PROTOCOL_RESPONSE_LOGIN_H__