#ifndef __PROTOCOL_RESPONSE_LOGIN_H__
#define __PROTOCOL_RESPONSE_LOGIN_H__

#include <fb/protocol/protocol.h>

namespace fb { namespace protocol { namespace login { namespace response {

class agreement : public fb::protocol::base::header
{
public:
#ifdef BOT
    std::string             contents;
#else
    const std::string       contents;
#endif

public:
#ifdef BOT
    agreement() : fb::protocol::base::header(0x60)
    { }
#endif
    agreement(const std::string& contents) : fb::protocol::base::header(0x60),
        contents(contents)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        auto compressed = fb::buffer((uint8_t*)this->contents.data(), this->contents.size()).compress();
        base::header::serialize(out_stream);
        out_stream.write_u8(0x01)
                  .write_u16((uint16_t)compressed.size())
                  .write(compressed.data(), (uint16_t)compressed.size());
    }
#ifdef BOT
    void deserialize(fb::istream& in_stream)
    {
        in_stream.read_u8();
        auto size = in_stream.read_u16();
        auto buffer = new uint8_t[size];
        in_stream.read(buffer, size);

        auto decompressed = fb::buffer(buffer, size).decompress();
        delete[] buffer;

        decompressed.push_back(0);
        this->contents = std::string((const char*)decompressed.data());
    }
#endif
};

class message : public fb::protocol::base::header
{
public:
#ifdef BOT
    std::string             text;
    uint8_t                 type;
#else
    const std::string       text;
    const uint8_t           type;
#endif

public:
#ifdef BOT
    message() : fb::protocol::base::header(0x02)
    { }
#endif
    message(const std::string& text, uint8_t type) : fb::protocol::base::header(0x02),
        text(text), type(type)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->type)
                  .writestr_u8(this->text);
    }
#ifdef BOT
    void deserialize(fb::istream& in_stream)
    {
        this->type = in_stream.read_8();
        this->text = in_stream.readstr_u8();
    }
#endif
};

} } } }

#endif // !__PROTOCOL_RESPONSE_LOGIN_H__