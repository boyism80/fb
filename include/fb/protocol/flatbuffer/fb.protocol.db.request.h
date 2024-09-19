#ifndef __FB_PROTOCOL_DB_REQUEST___
#define __FB_PROTOCOL_DB_REQUEST___

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/flatbuffer/fb.protocol.db.h>
#include <fb/protocol/flatbuffer/origin/db.request.character.create_generated.h>
#include <fb/protocol/flatbuffer/origin/db.request.character.finish_generated.h>
#include <fb/protocol/flatbuffer/origin/db.request.character.init_generated.h>
#include <fb/protocol/flatbuffer/origin/db.request.ping_generated.h>
#include <fb/protocol/flatbuffer/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace protocol { namespace db { namespace request { 


class CreateCharacter
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::CreateCharacter;

public:
    std::string name;

public:
    CreateCharacter()
    { }

    CreateCharacter(const CreateCharacter& x)
        : name(x.name)
    { }

    CreateCharacter(const std::string& name)
        : name(name)
    { }
    CreateCharacter(const fb::protocol::db::request::origin::CreateCharacter& raw)
        : name(raw.name()->c_str())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::origin::CreateCreateCharacter(builder,
            builder.CreateString(this->name));
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
        auto raw = fb::protocol::db::request::origin::GetCreateCharacter(bytes);
        return CreateCharacter(*raw);
    }
};

class FinishCharacter
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::FinishCharacter;

public:
    uint32_t pk;
    uint8_t hair;
    uint8_t sex;
    uint8_t nation;
    uint8_t creature;

public:
    FinishCharacter()
    { }

    FinishCharacter(const FinishCharacter& x)
        : pk(x.pk), hair(x.hair), sex(x.sex), nation(x.nation), creature(x.creature)
    { }

    FinishCharacter(uint32_t pk, uint8_t hair, uint8_t sex, uint8_t nation, uint8_t creature)
        : pk(pk), hair(hair), sex(sex), nation(nation), creature(creature)
    { }
    FinishCharacter(const fb::protocol::db::request::origin::FinishCharacter& raw)
        : pk(raw.pk()), hair(raw.hair()), sex(raw.sex()), nation(raw.nation()), creature(raw.creature())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::origin::CreateFinishCharacter(builder,
            this->pk,
            this->hair,
            this->sex,
            this->nation,
            this->creature);
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
        auto raw = fb::protocol::db::request::origin::GetFinishCharacter(bytes);
        return FinishCharacter(*raw);
    }
};

class InitCharacter
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::InitCharacter;

public:
    uint32_t pk;
    std::string name;
    std::string pw;
    uint32_t hp;
    uint32_t mp;
    uint32_t map;
    uint16_t x;
    uint16_t y;
    bool admin;

public:
    InitCharacter()
    { }

    InitCharacter(const InitCharacter& x)
        : pk(x.pk), name(x.name), pw(x.pw), hp(x.hp), mp(x.mp), map(x.map), x(x.x), y(x.y), admin(x.admin)
    { }

    InitCharacter(uint32_t pk, const std::string& name, const std::string& pw, uint32_t hp, uint32_t mp, uint32_t map, uint16_t x, uint16_t y, bool admin)
        : pk(pk), name(name), pw(pw), hp(hp), mp(mp), map(map), x(x), y(y), admin(admin)
    { }
    InitCharacter(const fb::protocol::db::request::origin::InitCharacter& raw)
        : pk(raw.pk()), name(raw.name()->c_str()), pw(raw.pw()->c_str()), hp(raw.hp()), mp(raw.mp()), map(raw.map()), x(raw.x()), y(raw.y()), admin(raw.admin())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::origin::CreateInitCharacter(builder,
            this->pk,
            builder.CreateString(this->name),
            builder.CreateString(this->pw),
            this->hp,
            this->mp,
            this->map,
            this->x,
            this->y,
            this->admin);
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
        auto raw = fb::protocol::db::request::origin::GetInitCharacter(bytes);
        return InitCharacter(*raw);
    }
};

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