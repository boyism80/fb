#ifndef __FB_PROTOCOL_DB_RESPONSE___
#define __FB_PROTOCOL_DB_RESPONSE___

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/db.h>
#include <fb/protocol/origin/db.response.account_generated.h>
#include <fb/protocol/origin/db.response.board_delete_generated.h>
#include <fb/protocol/origin/db.response.board_get_generated.h>
#include <fb/protocol/origin/db.response.board_get_list_generated.h>
#include <fb/protocol/origin/db.response.board_write_generated.h>
#include <fb/protocol/origin/db.response.change_pw_generated.h>
#include <fb/protocol/origin/db.response.get_uid_generated.h>
#include <fb/protocol/origin/db.response.init_ch_generated.h>
#include <fb/protocol/origin/db.response.login_generated.h>
#include <fb/protocol/origin/db.response.mk_ch_generated.h>
#include <fb/protocol/origin/db.response.reserver_name_generated.h>
#include <fb/protocol/origin/db.response.save_generated.h>
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
    uint32_t map;
    bool success;

public:
    Account()
    { }

    Account(const Account& x)
        : pw(x.pw), map(x.map), success(x.success)
    { }

    Account(const std::string& pw, uint32_t map, bool success)
        : pw(pw), map(map), success(success)
    { }
    Account(const fb::protocol::db::response::origin::Account& raw)
        : pw(raw.pw() != nullptr ? raw.pw()->c_str() : ""), map(raw.map()), success(raw.success())
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

class DeleteArticle
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::DeleteArticle;

public:
    int result;

public:
    DeleteArticle()
    { }

    DeleteArticle(const DeleteArticle& x)
        : result(x.result)
    { }

    DeleteArticle(int result)
        : result(result)
    { }
    DeleteArticle(const fb::protocol::db::response::origin::DeleteArticle& raw)
        : result(raw.result())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::origin::CreateDeleteArticle(builder,
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

    static DeleteArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::origin::GetDeleteArticle(bytes);
        return DeleteArticle(*raw);
    }
};

class GetArticle
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::GetArticle;

public:
    fb::protocol::db::Article article;
    bool next;
    bool success;

public:
    GetArticle()
    { }

    GetArticle(const GetArticle& x)
        : article(x.article), next(x.next), success(x.success)
    { }

    GetArticle(const fb::protocol::db::Article& article, bool next, bool success)
        : article(article), next(next), success(success)
    { }
    GetArticle(const fb::protocol::db::response::origin::GetArticle& raw)
        : article(*raw.article()), next(raw.next()), success(raw.success())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::origin::CreateGetArticle(builder,
            this->article.Build(builder),
            this->next,
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

    static GetArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::origin::GetGetArticle(bytes);
        return GetArticle(*raw);
    }
};

class GetArticleList
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::GetArticleList;

public:
    std::vector<fb::protocol::db::ArticleSummary> summary_list;

public:
    GetArticleList()
    { }

    GetArticleList(const GetArticleList& x)
        : summary_list(x.summary_list)
    { }

    GetArticleList(const std::vector<fb::protocol::db::ArticleSummary>& summary_list)
        : summary_list(summary_list)
    { }
    GetArticleList(const fb::protocol::db::response::origin::GetArticleList& raw)
    {
        for (int i = 0; i < raw.summary_list()->size(); i++)
            this->summary_list.push_back(*raw.summary_list()->Get(i));
    }

private:
    auto CreateSummary_list(flatbuffers::FlatBufferBuilder& builder) const
    {
        auto result = std::vector<flatbuffers::Offset<fb::protocol::db::origin::ArticleSummary>>();
        for(auto& x : this->summary_list)
        {
            result.push_back(x.Build(builder));
        }

        return builder.CreateVector(result);
    }

public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::origin::CreateGetArticleList(builder,
            this->CreateSummary_list(builder));
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

    static GetArticleList Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::origin::GetGetArticleList(bytes);
        return GetArticleList(*raw);
    }
};

class WriteArticle
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::WriteArticle;

public:
    bool success;

public:
    WriteArticle()
    { }

    WriteArticle(const WriteArticle& x)
        : success(x.success)
    { }

    WriteArticle(bool success)
        : success(success)
    { }
    WriteArticle(const fb::protocol::db::response::origin::WriteArticle& raw)
        : success(raw.success())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::origin::CreateWriteArticle(builder,
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

    static WriteArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::origin::GetWriteArticle(bytes);
        return WriteArticle(*raw);
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

class Login
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::Login;

public:
    fb::protocol::db::Character character;
    std::vector<fb::protocol::db::Item> items;
    std::vector<fb::protocol::db::Spell> spells;

public:
    Login()
    { }

    Login(const Login& x)
        : character(x.character), items(x.items), spells(x.spells)
    { }

    Login(const fb::protocol::db::Character& character, const std::vector<fb::protocol::db::Item>& items, const std::vector<fb::protocol::db::Spell>& spells)
        : character(character), items(items), spells(spells)
    { }
    Login(const fb::protocol::db::response::origin::Login& raw)
        : character(*raw.character())
    {
        for (int i = 0; i < raw.items()->size(); i++)
            this->items.push_back(*raw.items()->Get(i));

        for (int i = 0; i < raw.spells()->size(); i++)
            this->spells.push_back(*raw.spells()->Get(i));
    }

private:
    auto CreateItems(flatbuffers::FlatBufferBuilder& builder) const
    {
        auto result = std::vector<flatbuffers::Offset<fb::protocol::db::origin::Item>>();
        for(auto& x : this->items)
        {
            result.push_back(x.Build(builder));
        }

        return builder.CreateVector(result);
    }

    auto CreateSpells(flatbuffers::FlatBufferBuilder& builder) const
    {
        auto result = std::vector<flatbuffers::Offset<fb::protocol::db::origin::Spell>>();
        for(auto& x : this->spells)
        {
            result.push_back(x.Build(builder));
        }

        return builder.CreateVector(result);
    }

public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::origin::CreateLogin(builder,
            this->character.Build(builder),
            this->CreateItems(builder),
            this->CreateSpells(builder));
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

    static Login Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::origin::GetLogin(bytes);
        return Login(*raw);
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

class Save
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::Save;

public:
    bool success;

public:
    Save()
    { }

    Save(const Save& x)
        : success(x.success)
    { }

    Save(bool success)
        : success(success)
    { }
    Save(const fb::protocol::db::response::origin::Save& raw)
        : success(raw.success())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::origin::CreateSave(builder,
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

    static Save Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::origin::GetSave(bytes);
        return Save(*raw);
    }
};

 }  }  }  } 

#endif