#ifndef __PROTOCOL_RESPONSE_LOGIN_H__
#define __PROTOCOL_RESPONSE_LOGIN_H__

#include <fb/protocol/protocol.h>

namespace fb { namespace protocol { namespace login { namespace response {

class agreement : public fb::protocol::base::header
{
public:
    const std::string       contents;

public:
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
};

class message : public fb::protocol::base::header
{
public:
    const std::string       text;
    const uint8_t           type;

public:
    message(const std::string& text, uint8_t type) : fb::protocol::base::header(0x02),
        text(text), type(type)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(type)
                  .write(this->text);
    }
};

} } } }

#endif // !__PROTOCOL_RESPONSE_LOGIN_H__