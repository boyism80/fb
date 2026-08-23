#ifndef __PROTOCOL_HEADER_H__
#define __PROTOCOL_HEADER_H__

#include <fb/stream_reader.h>
#include <fb/stream_writer.h>

namespace fb::protocol {

class header
{
protected:
    header() = default;

public:
    virtual ~header() = default;

public:
    virtual void serialize(fb::stream_writer<big_endian>& writer) const
    { }
    virtual void deserialize(fb::stream_reader<big_endian>& reader)
    { }
};

// Wire framing: whether the acceptor decrypts this opcode before deserialize.
// Protocol types may set `static constexpr bool decrypt = false;` to opt out.
template <typename ProtocolType>
constexpr bool protocol_decrypt()
{
    if constexpr (requires { ProtocolType::decrypt; })
        return ProtocolType::decrypt;
    else
        return true;
}

} // namespace fb::protocol

#endif
