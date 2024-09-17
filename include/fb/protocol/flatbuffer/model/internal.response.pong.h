#ifndef __FB_PROTOCOL_FLATBUFFER_INTER_RESPONSE_INTERNAL_RESPONSE_PONG__
#define __FB_PROTOCOL_FLATBUFFER_INTER_RESPONSE_INTERNAL_RESPONSE_PONG__

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/flatbuffer/internal.response.pong_generated.h>
#include <fb/protocol/flatbuffer/model/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace protocol { namespace flatbuffer { namespace inter { namespace response { namespace model { 

class Pong
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::Pong;

public:

public:
    Pong()
    { }

    Pong(const Pong& x)
    { }

    Pong(const fb::protocol::flatbuffer::inter::response::Pong& raw)
    { }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::flatbuffer::inter::response::CreatePong(builder);
    }
    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto bytes = builder.GetBufferPointer();
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), bytes, size);
        return result;
    }

    static Pong Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::flatbuffer::inter::response::GetPong(bytes);
        return Pong(*raw);
    }
};

} } } } } }

#endif