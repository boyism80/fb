#ifndef __FB_PROTOCOL_FLATBUFFER_INTER_REQUEST_INTERNAL_REQUEST_WHISPER__
#define __FB_PROTOCOL_FLATBUFFER_INTER_REQUEST_INTERNAL_REQUEST_WHISPER__

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/flatbuffer/internal.request.whisper_generated.h>
#include <fb/protocol/flatbuffer/model/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace protocol { namespace flatbuffer { namespace inter { namespace request { namespace model { 

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
    Whisper(const fb::protocol::flatbuffer::inter::request::Whisper& raw)
        : name(raw.name()->c_str()), message(raw.message()->c_str())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::flatbuffer::inter::request::CreateWhisper(builder,
            builder.CreateString(this->name),
            builder.CreateString(this->message));
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

    static Whisper Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::flatbuffer::inter::request::GetWhisper(bytes);
        return Whisper(*raw);
    }
};

} } } } } }

#endif