#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/flatbuffer/db.ping_generated.h>
#include <string>
#include <vector>

namespace fb { namespace game { namespace flatbuffer { namespace db { namespace model { 

class Ping
{
public:

public:
    Ping()
    { }

    Ping(const Ping& x)

    { }

    Ping(const fb::game::flatbuffer::db::Ping& raw)
    {
    }


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