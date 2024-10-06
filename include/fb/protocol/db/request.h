#ifndef __FB_PROTOCOL_DB_REQUEST___
#define __FB_PROTOCOL_DB_REQUEST___

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/db.h>
#include <fb/protocol/origin/db.request.account_generated.h>
#include <fb/protocol/origin/db.request.board_delete_generated.h>
#include <fb/protocol/origin/db.request.board_get_generated.h>
#include <fb/protocol/origin/db.request.board_get_list_generated.h>
#include <fb/protocol/origin/db.request.board_write_generated.h>
#include <fb/protocol/origin/db.request.change_pw_generated.h>
#include <fb/protocol/origin/db.request.init_ch_generated.h>
#include <fb/protocol/origin/db.request.login_generated.h>
#include <fb/protocol/origin/db.request.mk_ch_generated.h>
#include <fb/protocol/origin/db.request.reverse_name_generated.h>
#include <fb/protocol/origin/db.request.save_generated.h>
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

class DeleteArticle
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::DeleteArticle;

public:
    uint32_t id;
    uint32_t user;

public:
    DeleteArticle()
    { }

    DeleteArticle(const DeleteArticle& x)
        : id(x.id), user(x.user)
    { }

    DeleteArticle(uint32_t id, uint32_t user)
        : id(id), user(user)
    { }
    DeleteArticle(const fb::protocol::db::request::origin::DeleteArticle& raw)
        : id(raw.id()), user(raw.user())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::origin::CreateDeleteArticle(builder,
            this->id,
            this->user);
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
        auto raw = fb::protocol::db::request::origin::GetDeleteArticle(bytes);
        return DeleteArticle(*raw);
    }
};

class GetArticle
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::GetArticle;

public:
    uint32_t section;
    uint32_t article;

public:
    GetArticle()
    { }

    GetArticle(const GetArticle& x)
        : section(x.section), article(x.article)
    { }

    GetArticle(uint32_t section, uint32_t article)
        : section(section), article(article)
    { }
    GetArticle(const fb::protocol::db::request::origin::GetArticle& raw)
        : section(raw.section()), article(raw.article())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::origin::CreateGetArticle(builder,
            this->section,
            this->article);
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
        auto raw = fb::protocol::db::request::origin::GetGetArticle(bytes);
        return GetArticle(*raw);
    }
};

class GetArticleList
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::GetArticleList;

public:
    uint32_t section;
    uint32_t position;

public:
    GetArticleList()
    { }

    GetArticleList(const GetArticleList& x)
        : section(x.section), position(x.position)
    { }

    GetArticleList(uint32_t section, uint32_t position)
        : section(section), position(position)
    { }
    GetArticleList(const fb::protocol::db::request::origin::GetArticleList& raw)
        : section(raw.section()), position(raw.position())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::origin::CreateGetArticleList(builder,
            this->section,
            this->position);
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
        auto raw = fb::protocol::db::request::origin::GetGetArticleList(bytes);
        return GetArticleList(*raw);
    }
};

class WriteArticle
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::WriteArticle;

public:
    uint32_t section;
    uint32_t user;
    std::string title;
    std::string contents;

public:
    WriteArticle()
    { }

    WriteArticle(const WriteArticle& x)
        : section(x.section), user(x.user), title(x.title), contents(x.contents)
    { }

    WriteArticle(uint32_t section, uint32_t user, const std::string& title, const std::string& contents)
        : section(section), user(user), title(title), contents(contents)
    { }
    WriteArticle(const fb::protocol::db::request::origin::WriteArticle& raw)
        : section(raw.section()), user(raw.user()), title(raw.title() != nullptr ? raw.title()->c_str() : ""), contents(raw.contents() != nullptr ? raw.contents()->c_str() : "")
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::origin::CreateWriteArticle(builder,
            this->section,
            this->user,
            builder.CreateString(this->title),
            builder.CreateString(this->contents));
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
        auto raw = fb::protocol::db::request::origin::GetWriteArticle(bytes);
        return WriteArticle(*raw);
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
        : uid(raw.uid()), before(raw.before() != nullptr ? raw.before()->c_str() : ""), after(raw.after() != nullptr ? raw.after()->c_str() : ""), birthday(raw.birthday())
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
        : uid(raw.uid()), name(raw.name() != nullptr ? raw.name()->c_str() : ""), pw(raw.pw() != nullptr ? raw.pw()->c_str() : ""), hp(raw.hp()), mp(raw.mp()), map(raw.map()), x(raw.x()), y(raw.y()), admin(raw.admin())
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

class Login
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::Login;

public:
    uint32_t uid;

public:
    Login()
    { }

    Login(const Login& x)
        : uid(x.uid)
    { }

    Login(uint32_t uid)
        : uid(uid)
    { }
    Login(const fb::protocol::db::request::origin::Login& raw)
        : uid(raw.uid())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::origin::CreateLogin(builder,
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

    static Login Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::origin::GetLogin(bytes);
        return Login(*raw);
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
        : name(raw.name() != nullptr ? raw.name()->c_str() : "")
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

class Save
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::Save;

public:
    fb::protocol::db::Character character;
    std::vector<fb::protocol::db::Item> items;
    std::vector<fb::protocol::db::Spell> spells;

public:
    Save()
    { }

    Save(const Save& x)
        : character(x.character), items(x.items), spells(x.spells)
    { }

    Save(const fb::protocol::db::Character& character, const std::vector<fb::protocol::db::Item>& items, const std::vector<fb::protocol::db::Spell>& spells)
        : character(character), items(items), spells(spells)
    { }
    Save(const fb::protocol::db::request::origin::Save& raw)
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
        return fb::protocol::db::request::origin::CreateSave(builder,
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

    static Save Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::origin::GetSave(bytes);
        return Save(*raw);
    }
};

 }  }  }  } 

#endif