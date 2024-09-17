#ifndef __FB_PROTOCOL_DB_REQUEST___
#define __FB_PROTOCOL_DB_REQUEST___

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/flatbuffer/origin/db.request.ping_generated.h>
#include <fb/protocol/flatbuffer/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace protocol { namespace db { namespace request { 


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

    Ping(const fb::protocol::db::request::origin::Ping& raw)
    { }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::origin::CreatePing(builder);
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
        auto raw = fb::protocol::db::request::origin::GetPing(bytes);
        return Ping(*raw);
    }
};

 }  }  }  } 

#endif