#ifndef __FB_PROTOCOL_DB_RESPONSE___
#define __FB_PROTOCOL_DB_RESPONSE___

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/flatbuffer/origin/db.response.character.create_generated.h>
#include <fb/protocol/flatbuffer/origin/db.response.character.finnish_generated.h>
#include <fb/protocol/flatbuffer/origin/db.response.character.init_generated.h>
#include <fb/protocol/flatbuffer/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace protocol { namespace db { namespace response { 


class CreateCharacter
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::CreateCharacter;

public:
    uint32_t pk;
    bool success;

public:
    CreateCharacter()
    { }

    CreateCharacter(const CreateCharacter& x)
        : pk(x.pk), success(x.success)
    { }

    CreateCharacter(uint32_t pk, bool success)
        : pk(pk), success(success)
    { }
    CreateCharacter(const fb::protocol::db::response::origin::CreateCharacter& raw)
        : pk(raw.pk()), success(raw.success())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::origin::CreateCreateCharacter(builder,
            this->pk,
            this->success);
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

    static CreateCharacter Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::origin::GetCreateCharacter(bytes);
        return CreateCharacter(*raw);
    }
};

class FinishCharacter
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::FinishCharacter;

public:
    bool success;

public:
    FinishCharacter()
    { }

    FinishCharacter(const FinishCharacter& x)
        : success(x.success)
    { }

    FinishCharacter(bool success)
        : success(success)
    { }
    FinishCharacter(const fb::protocol::db::response::origin::FinishCharacter& raw)
        : success(raw.success())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::origin::CreateFinishCharacter(builder,
            this->success);
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

    static FinishCharacter Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::origin::GetFinishCharacter(bytes);
        return FinishCharacter(*raw);
    }
};

class InitCharacter
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::InitCharacter;

public:
    bool success;

public:
    InitCharacter()
    { }

    InitCharacter(const InitCharacter& x)
        : success(x.success)
    { }

    InitCharacter(bool success)
        : success(success)
    { }
    InitCharacter(const fb::protocol::db::response::origin::InitCharacter& raw)
        : success(raw.success())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::origin::CreateInitCharacter(builder,
            this->success);
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

    static InitCharacter Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::origin::GetInitCharacter(bytes);
        return InitCharacter(*raw);
    }
};

 }  }  }  } 

#endif