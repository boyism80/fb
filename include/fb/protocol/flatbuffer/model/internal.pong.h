#ifndef __FB_GAME_FLATBUFFER_INTER_INTERNAL_PONG__
#define __FB_GAME_FLATBUFFER_INTER_INTERNAL_PONG__

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/flatbuffer/internal.pong_generated.h>
#include <fb/protocol/flatbuffer/model/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace game { namespace flatbuffer { namespace inter { namespace model { 

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

    Pong(const fb::game::flatbuffer::inter::Pong& raw)
    { }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder)
    {
        return fb::game::flatbuffer::inter::CreatePong(builder);
    }
    std::vector<uint8_t> Serialize()
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
        auto raw = fb::game::flatbuffer::inter::GetPong(bytes);
        return Pong(*raw);
    }
};

} } } } }

#endif