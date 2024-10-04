#ifndef __FB_PROTOCOL_DB_REQUEST___
#define __FB_PROTOCOL_DB_REQUEST___

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/origin/db.request.account_generated.h>
#include <fb/protocol/origin/db.request.change_pw_generated.h>
#include <fb/protocol/origin/db.request.init_ch_generated.h>
#include <fb/protocol/origin/db.request.mk_ch_generated.h>
#include <fb/protocol/origin/db.request.reverse_name_generated.h>
#include <fb/protocol/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace protocol { namespace db { namespace request { 


class Account
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::Account;

public:
    uint32_t uid;

public:
    Account()
    { }

    Account(const Account& x)
        : uid(x.uid)
    { }

    Account(uint32_t uid)
        : uid(uid)
    { }
    Account(const fb::protocol::db::request::origin::Account& raw)
        : uid(raw.uid())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::origin::CreateAccount(builder,
            this->uid);
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

    static Account Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::origin::GetAccount(bytes);
        return Account(*raw);
    }
};

class ChangePw
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::ChangePw;

public:
    uint32_t uid;
    std::string before;
    std::string after;
    uint32_t birthday;

public:
    ChangePw()
    { }

    ChangePw(const ChangePw& x)
        : uid(x.uid), before(x.before), after(x.after), birthday(x.birthday)
    { }

    ChangePw(uint32_t uid, const std::string& before, const std::string& after, uint32_t birthday)
        : uid(uid), before(before), after(after), birthday(birthday)
    { }
    ChangePw(const fb::protocol::db::request::origin::ChangePw& raw)
        : uid(raw.uid()), before(raw.before()->c_str()), after(raw.after()->c_str()), birthday(raw.birthday())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::origin::CreateChangePw(builder,
            this->uid,
            builder.CreateString(this->before),
            builder.CreateString(this->after),
            this->birthday);
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

    static ChangePw Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::origin::GetChangePw(bytes);
        return ChangePw(*raw);
    }
};

class InitCharacter
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::InitCharacter;

public:
    uint32_t uid;
    std::string name;
    std::string pw;
    uint32_t hp;
    uint32_t mp;
    uint16_t map;
    uint16_t x;
    uint16_t y;
    bool admin;

public:
    InitCharacter()
    { }

    InitCharacter(const InitCharacter& x)
        : uid(x.uid), name(x.name), pw(x.pw), hp(x.hp), mp(x.mp), map(x.map), x(x.x), y(x.y), admin(x.admin)
    { }

    InitCharacter(uint32_t uid, const std::string& name, const std::string& pw, uint32_t hp, uint32_t mp, uint16_t map, uint16_t x, uint16_t y, bool admin)
        : uid(uid), name(name), pw(pw), hp(hp), mp(mp), map(map), x(x), y(y), admin(admin)
    { }
    InitCharacter(const fb::protocol::db::request::origin::InitCharacter& raw)
        : uid(raw.uid()), name(raw.name()->c_str()), pw(raw.pw()->c_str()), hp(raw.hp()), mp(raw.mp()), map(raw.map()), x(raw.x()), y(raw.y()), admin(raw.admin())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::origin::CreateInitCharacter(builder,
            this->uid,
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

class MakeCharacter
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::MakeCharacter;

public:
    uint32_t uid;
    uint16_t hair;
    uint8_t sex;
    uint8_t nation;
    uint8_t creature;

public:
    MakeCharacter()
    { }

    MakeCharacter(const MakeCharacter& x)
        : uid(x.uid), hair(x.hair), sex(x.sex), nation(x.nation), creature(x.creature)
    { }

    MakeCharacter(uint32_t uid, uint16_t hair, uint8_t sex, uint8_t nation, uint8_t creature)
        : uid(uid), hair(hair), sex(sex), nation(nation), creature(creature)
    { }
    MakeCharacter(const fb::protocol::db::request::origin::MakeCharacter& raw)
        : uid(raw.uid()), hair(raw.hair()), sex(raw.sex()), nation(raw.nation()), creature(raw.creature())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::origin::CreateMakeCharacter(builder,
            this->uid,
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

    static MakeCharacter Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::origin::GetMakeCharacter(bytes);
        return MakeCharacter(*raw);
    }
};

class ReserveName
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::ReserveName;

public:
    std::string name;

public:
    ReserveName()
    { }

    ReserveName(const ReserveName& x)
        : name(x.name)
    { }

    ReserveName(const std::string& name)
        : name(name)
    { }
    ReserveName(const fb::protocol::db::request::origin::ReserveName& raw)
        : name(raw.name()->c_str())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::origin::CreateReserveName(builder,
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

    static ReserveName Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::origin::GetReserveName(bytes);
        return ReserveName(*raw);
    }
};

 }  }  }  } 

#endif