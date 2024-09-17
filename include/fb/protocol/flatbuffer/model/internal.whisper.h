#ifndef __FB_GAME_FLATBUFFER_INTER_INTERNAL_WHISPER__
#define __FB_GAME_FLATBUFFER_INTER_INTERNAL_WHISPER__

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/flatbuffer/internal.whisper_generated.h>
#include <fb/protocol/flatbuffer/model/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace game { namespace flatbuffer { namespace inter { namespace model { 

class Whisper
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::Whisper;

public:
    std::string name;
    std::string message;

public:
    Whisper()
    { }

    Whisper(const Whisper& x)
        : name(x.name), message(x.message)
    { }

    Whisper(const std::string& name, const std::string& message)
        : name(name), message(message)
    { }
    Whisper(const fb::game::flatbuffer::inter::Whisper& raw)
        : name(raw.name()->c_str()), message(raw.message()->c_str())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder)
    {
        return fb::game::flatbuffer::inter::CreateWhisper(builder,
            builder.CreateString(this->name),
            builder.CreateString(this->message));
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

    static Whisper Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::game::flatbuffer::inter::GetWhisper(bytes);
        return Whisper(*raw);
    }
};

} } } } }

#endif