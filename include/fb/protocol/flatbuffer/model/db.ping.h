#ifndef __FB_GAME_FLATBUFFER_DB_DB_PING__
#define __FB_GAME_FLATBUFFER_DB_DB_PING__

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/flatbuffer/db.ping_generated.h>
#include <fb/protocol/flatbuffer/model/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace game { namespace flatbuffer { namespace db { namespace model { 

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

    Ping(const fb::game::flatbuffer::db::Ping& raw)
    { }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder)
    {
        return fb::game::flatbuffer::db::CreatePing(builder);
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

    static Ping Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::game::flatbuffer::db::GetPing(bytes);
        return Ping(*raw);
    }
};

} } } } }

#endif