#ifndef __FB_GAME_FLATBUFFER_INTER_INTERNAL_PING__
#define __FB_GAME_FLATBUFFER_INTER_INTERNAL_PING__

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/flatbuffer/internal.ping_generated.h>
#include <fb/protocol/flatbuffer/model/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace game { namespace flatbuffer { namespace inter { namespace model { 

class Ping
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::Ping;

public:

public:
    Ping()
    { }

    Ping(const Ping& x)
    { }

    Ping(const fb::game::flatbuffer::inter::Ping& raw)
    { }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::game::flatbuffer::inter::CreatePing(builder);
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

    static Ping Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::game::flatbuffer::inter::GetPing(bytes);
        return Ping(*raw);
    }
};

} } } } }

#endif