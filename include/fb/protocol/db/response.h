#ifndef __FB_PROTOCOL_DB_RESPONSE___
#define __FB_PROTOCOL_DB_RESPONSE___

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/origin/db.response.account_generated.h>
#include <fb/protocol/origin/db.response.change_pw_generated.h>
#include <fb/protocol/origin/db.response.get_uid_generated.h>
#include <fb/protocol/origin/db.response.init_ch_generated.h>
#include <fb/protocol/origin/db.response.mk_ch_generated.h>
#include <fb/protocol/origin/db.response.reserver_name_generated.h>
#include <fb/protocol/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace protocol { namespace db { namespace response { 


class Account
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::Account;

public:
    std::string pw;
    uint16_t map;
    bool success;

public:
    Account()
    { }

    Account(const Account& x)
        : pw(x.pw), map(x.map), success(x.success)
    { }

    Account(const std::string& pw, uint16_t map, bool success)
        : pw(pw), map(map), success(success)
    { }
    Account(const fb::protocol::db::response::origin::Account& raw)
        : pw(raw.pw()->c_str()), map(raw.map()), success(raw.success())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::origin::CreateAccount(builder,
            builder.CreateString(this->pw),
            this->map,
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

    static Account Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::origin::GetAccount(bytes);
        return Account(*raw);
    }
};

class ChangePw
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::ChangePw;

public:
    int result;

public:
    ChangePw()
    { }

    ChangePw(const ChangePw& x)
        : result(x.result)
    { }

    ChangePw(int result)
        : result(result)
    { }
    ChangePw(const fb::protocol::db::response::origin::ChangePw& raw)
        : result(raw.result())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::origin::CreateChangePw(builder,
            this->result);
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
        auto raw = fb::protocol::db::response::origin::GetChangePw(bytes);
        return ChangePw(*raw);
    }
};

class GetUid
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::GetUid;

public:
    uint32_t uid;
    bool success;

public:
    GetUid()
    { }

    GetUid(const GetUid& x)
        : uid(x.uid), success(x.success)
    { }

    GetUid(uint32_t uid, bool success)
        : uid(uid), success(success)
    { }
    GetUid(const fb::protocol::db::response::origin::GetUid& raw)
        : uid(raw.uid()), success(raw.success())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::origin::CreateGetUid(builder,
            this->uid,
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

    static GetUid Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::origin::GetGetUid(bytes);
        return GetUid(*raw);
    }
};

class InitCharacter
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::InitCharacter;

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

class MakeCharacter
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::MakeCharacter;

public:
    bool success;

public:
    MakeCharacter()
    { }

    MakeCharacter(const MakeCharacter& x)
        : success(x.success)
    { }

    MakeCharacter(bool success)
        : success(success)
    { }
    MakeCharacter(const fb::protocol::db::response::origin::MakeCharacter& raw)
        : success(raw.success())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::origin::CreateMakeCharacter(builder,
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

    static MakeCharacter Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::origin::GetMakeCharacter(bytes);
        return MakeCharacter(*raw);
    }
};

class ReserveName
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::ReserveName;

public:
    uint32_t uid;
    bool success;

public:
    ReserveName()
    { }

    ReserveName(const ReserveName& x)
        : uid(x.uid), success(x.success)
    { }

    ReserveName(uint32_t uid, bool success)
        : uid(uid), success(success)
    { }
    ReserveName(const fb::protocol::db::response::origin::ReserveName& raw)
        : uid(raw.uid()), success(raw.success())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::origin::CreateReserveName(builder,
            this->uid,
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

    static ReserveName Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::origin::GetReserveName(bytes);
        return ReserveName(*raw);
    }
};

 }  }  }  } 

#endif