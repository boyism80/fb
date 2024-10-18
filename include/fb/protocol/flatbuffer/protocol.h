#ifndef __FLATBUFFER_PROTOCOL_H__
#define __FLATBUFFER_PROTOCOL_H__

#include <string>
#include <optional>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.position_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.character_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.item_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.spell_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.articlesummary_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.article_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.account_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.changepw_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.initcharacter_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.login_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.makecharacter_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.reservename_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.save_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.getarticle_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.getarticlelist_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.writearticle_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.request.deletearticle_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.account_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.deletearticle_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.getarticle_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.getarticlelist_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.writearticle_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.changepw_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.getuid_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.initcharacter_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.login_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.makecharacter_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.reservename_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.db.response.save_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.service_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.transferresult_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.login_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.logout_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.ping_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.transfer_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.whisper_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.request.kickout_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.kickout_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.login_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.logout_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.pong_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.transfer_generated.h>
#include <fb/protocol/flatbuffer/raw/fb.protocol.internal.response.whisper_generated.h>

namespace fb { namespace protocol { namespace internal { 
enum class Service : int8_t
{
    Gateway = fb::protocol::internal::raw::Service::Service_Gateway,
    Login = fb::protocol::internal::raw::Service::Service_Login,
    Game = fb::protocol::internal::raw::Service::Service_Game,
};
enum class TransferResult : int8_t
{
    Success = fb::protocol::internal::raw::TransferResult::TransferResult_Success,
    Failed = fb::protocol::internal::raw::TransferResult::TransferResult_Failed,
    LoggedIn = fb::protocol::internal::raw::TransferResult::TransferResult_LoggedIn,
};
} } } 

namespace fb { namespace protocol { namespace db { 

enum class FlatBufferProtocolType
{
    Position,
    Character,
    Item,
    Spell,
    ArticleSummary,
    Article,
};

class Position
{
public:
    static inline fb::protocol::db::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::FlatBufferProtocolType::Position;

public:
    uint16_t x = 0;
    uint16_t y = 0;

public:
    Position() = default;

    Position(const Position& x)
        : x(x.x), y(x.y)
    { }

    Position(uint16_t x, uint16_t y)
        : x(x), y(y)
    { }

    Position(const fb::protocol::db::raw::Position& raw)
        : x(raw.x()), y(raw.y())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::raw::CreatePosition(builder,
            this->x,
            this->y);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Position Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::raw::GetPosition(bytes);
        return Position(*raw);
    }
};
class Character
{
public:
    static inline fb::protocol::db::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::FlatBufferProtocolType::Character;

public:
    uint32_t id = 0;
    std::string name = "";
    std::string last_login = "";
    bool admin = false;
    uint16_t look = 0;
    uint16_t color = 0;
    uint16_t sex = 0;
    uint16_t nation = 0;
    std::optional<uint16_t> creature = std::nullopt;
    uint32_t map = 0;
    fb::protocol::db::Position position;
    uint8_t direction = 0;
    uint8_t state = 0;
    uint8_t class_type = 0;
    uint8_t promotion = 0;
    uint32_t exp = 0;
    uint32_t money = 0;
    uint32_t deposited_money = 0;
    std::optional<uint16_t> disguise = std::nullopt;
    uint32_t hp = 0;
    uint32_t base_hp = 0;
    uint32_t additional_hp = 0;
    uint32_t mp = 0;
    uint32_t base_mp = 0;
    uint32_t additional_mp = 0;
    std::optional<uint8_t> weapon_color = std::nullopt;
    std::optional<uint8_t> helmet_color = std::nullopt;
    std::optional<uint8_t> armor_color = std::nullopt;
    std::optional<uint8_t> shield_color = std::nullopt;
    std::optional<uint8_t> ring_left_color = std::nullopt;
    std::optional<uint8_t> ring_right_color = std::nullopt;
    std::optional<uint8_t> aux_top_color = std::nullopt;
    std::optional<uint8_t> aux_bot_color = std::nullopt;
    std::optional<uint32_t> clan = std::nullopt;

public:
    Character() = default;

    Character(const Character& x)
        : id(x.id), name(x.name), last_login(x.last_login), admin(x.admin), look(x.look), color(x.color), sex(x.sex), nation(x.nation), creature(x.creature), map(x.map), position(x.position), direction(x.direction), state(x.state), class_type(x.class_type), promotion(x.promotion), exp(x.exp), money(x.money), deposited_money(x.deposited_money), disguise(x.disguise), hp(x.hp), base_hp(x.base_hp), additional_hp(x.additional_hp), mp(x.mp), base_mp(x.base_mp), additional_mp(x.additional_mp), weapon_color(x.weapon_color), helmet_color(x.helmet_color), armor_color(x.armor_color), shield_color(x.shield_color), ring_left_color(x.ring_left_color), ring_right_color(x.ring_right_color), aux_top_color(x.aux_top_color), aux_bot_color(x.aux_bot_color), clan(x.clan)
    { }

    Character(uint32_t id, const std::string& name, const std::string& last_login, bool admin, uint16_t look, uint16_t color, uint16_t sex, uint16_t nation, const std::optional<uint16_t>& creature, uint32_t map, const fb::protocol::db::Position& position, uint8_t direction, uint8_t state, uint8_t class_type, uint8_t promotion, uint32_t exp, uint32_t money, uint32_t deposited_money, const std::optional<uint16_t>& disguise, uint32_t hp, uint32_t base_hp, uint32_t additional_hp, uint32_t mp, uint32_t base_mp, uint32_t additional_mp, const std::optional<uint8_t>& weapon_color, const std::optional<uint8_t>& helmet_color, const std::optional<uint8_t>& armor_color, const std::optional<uint8_t>& shield_color, const std::optional<uint8_t>& ring_left_color, const std::optional<uint8_t>& ring_right_color, const std::optional<uint8_t>& aux_top_color, const std::optional<uint8_t>& aux_bot_color, const std::optional<uint32_t>& clan)
        : id(id), name(name), last_login(last_login), admin(admin), look(look), color(color), sex(sex), nation(nation), creature(creature), map(map), position(position), direction(direction), state(state), class_type(class_type), promotion(promotion), exp(exp), money(money), deposited_money(deposited_money), disguise(disguise), hp(hp), base_hp(base_hp), additional_hp(additional_hp), mp(mp), base_mp(base_mp), additional_mp(additional_mp), weapon_color(weapon_color), helmet_color(helmet_color), armor_color(armor_color), shield_color(shield_color), ring_left_color(ring_left_color), ring_right_color(ring_right_color), aux_top_color(aux_top_color), aux_bot_color(aux_bot_color), clan(clan)
    { }

    Character(const fb::protocol::db::raw::Character& raw)
        : id(raw.id()), name(raw.name()->c_str()), last_login(raw.last_login()->c_str()), admin(raw.admin()), look(raw.look()), color(raw.color()), sex(raw.sex()), nation(raw.nation()), creature(raw.creature() != nullptr ? raw.creature()->value() : std::optional<uint16_t>()), map(raw.map()), position(*raw.position()), direction(raw.direction()), state(raw.state()), class_type(raw.class_type()), promotion(raw.promotion()), exp(raw.exp()), money(raw.money()), deposited_money(raw.deposited_money()), disguise(raw.disguise() != nullptr ? raw.disguise()->value() : std::optional<uint16_t>()), hp(raw.hp()), base_hp(raw.base_hp()), additional_hp(raw.additional_hp()), mp(raw.mp()), base_mp(raw.base_mp()), additional_mp(raw.additional_mp()), weapon_color(raw.weapon_color() != nullptr ? raw.weapon_color()->value() : std::optional<uint8_t>()), helmet_color(raw.helmet_color() != nullptr ? raw.helmet_color()->value() : std::optional<uint8_t>()), armor_color(raw.armor_color() != nullptr ? raw.armor_color()->value() : std::optional<uint8_t>()), shield_color(raw.shield_color() != nullptr ? raw.shield_color()->value() : std::optional<uint8_t>()), ring_left_color(raw.ring_left_color() != nullptr ? raw.ring_left_color()->value() : std::optional<uint8_t>()), ring_right_color(raw.ring_right_color() != nullptr ? raw.ring_right_color()->value() : std::optional<uint8_t>()), aux_top_color(raw.aux_top_color() != nullptr ? raw.aux_top_color()->value() : std::optional<uint8_t>()), aux_bot_color(raw.aux_bot_color() != nullptr ? raw.aux_bot_color()->value() : std::optional<uint8_t>()), clan(raw.clan() != nullptr ? raw.clan()->value() : std::optional<uint32_t>())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::raw::CreateCharacter(builder,
            this->id,
            builder.CreateString(this->name),
            builder.CreateString(this->last_login),
            this->admin,
            this->look,
            this->color,
            this->sex,
            this->nation,
            this->creature.has_value() ? nullable::Createnullable_ushort(builder, this->creature.value()) : 0,
            this->map,
            this->position.Build(builder),
            this->direction,
            this->state,
            this->class_type,
            this->promotion,
            this->exp,
            this->money,
            this->deposited_money,
            this->disguise.has_value() ? nullable::Createnullable_ushort(builder, this->disguise.value()) : 0,
            this->hp,
            this->base_hp,
            this->additional_hp,
            this->mp,
            this->base_mp,
            this->additional_mp,
            this->weapon_color.has_value() ? nullable::Createnullable_ubyte(builder, this->weapon_color.value()) : 0,
            this->helmet_color.has_value() ? nullable::Createnullable_ubyte(builder, this->helmet_color.value()) : 0,
            this->armor_color.has_value() ? nullable::Createnullable_ubyte(builder, this->armor_color.value()) : 0,
            this->shield_color.has_value() ? nullable::Createnullable_ubyte(builder, this->shield_color.value()) : 0,
            this->ring_left_color.has_value() ? nullable::Createnullable_ubyte(builder, this->ring_left_color.value()) : 0,
            this->ring_right_color.has_value() ? nullable::Createnullable_ubyte(builder, this->ring_right_color.value()) : 0,
            this->aux_top_color.has_value() ? nullable::Createnullable_ubyte(builder, this->aux_top_color.value()) : 0,
            this->aux_bot_color.has_value() ? nullable::Createnullable_ubyte(builder, this->aux_bot_color.value()) : 0,
            this->clan.has_value() ? nullable::Createnullable_uint(builder, this->clan.value()) : 0);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Character Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::raw::GetCharacter(bytes);
        return Character(*raw);
    }
};
class Item
{
public:
    static inline fb::protocol::db::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::FlatBufferProtocolType::Item;

public:
    uint32_t user = 0;
    int16_t index = 0;
    int16_t parts = 0;
    int16_t deposited = 0;
    uint32_t model = 0;
    uint16_t count = 0;
    std::optional<uint32_t> durability = std::nullopt;
    std::optional<std::string> custom_name = std::nullopt;

public:
    Item() = default;

    Item(const Item& x)
        : user(x.user), index(x.index), parts(x.parts), deposited(x.deposited), model(x.model), count(x.count), durability(x.durability), custom_name(x.custom_name)
    { }

    Item(uint32_t user, int16_t index, int16_t parts, int16_t deposited, uint32_t model, uint16_t count, const std::optional<uint32_t>& durability, const std::optional<std::string>& custom_name)
        : user(user), index(index), parts(parts), deposited(deposited), model(model), count(count), durability(durability), custom_name(custom_name)
    { }

    Item(const fb::protocol::db::raw::Item& raw)
        : user(raw.user()), index(raw.index()), parts(raw.parts()), deposited(raw.deposited()), model(raw.model()), count(raw.count()), durability(raw.durability() != nullptr ? raw.durability()->value() : std::optional<uint32_t>()), custom_name(raw.custom_name() != nullptr ? raw.custom_name()->c_str() : std::optional<std::string>())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::raw::CreateItem(builder,
            this->user,
            this->index,
            this->parts,
            this->deposited,
            this->model,
            this->count,
            this->durability.has_value() ? nullable::Createnullable_uint(builder, this->durability.value()) : 0,
            this->custom_name.has_value() ? builder.CreateString(this->custom_name.value()) : 0);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Item Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::raw::GetItem(bytes);
        return Item(*raw);
    }
};
class Spell
{
public:
    static inline fb::protocol::db::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::FlatBufferProtocolType::Spell;

public:
    uint32_t user = 0;
    uint8_t slot = 0;
    uint32_t model = 0;

public:
    Spell() = default;

    Spell(const Spell& x)
        : user(x.user), slot(x.slot), model(x.model)
    { }

    Spell(uint32_t user, uint8_t slot, uint32_t model)
        : user(user), slot(slot), model(model)
    { }

    Spell(const fb::protocol::db::raw::Spell& raw)
        : user(raw.user()), slot(raw.slot()), model(raw.model())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::raw::CreateSpell(builder,
            this->user,
            this->slot,
            this->model);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Spell Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::raw::GetSpell(bytes);
        return Spell(*raw);
    }
};
class ArticleSummary
{
public:
    static inline fb::protocol::db::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::FlatBufferProtocolType::ArticleSummary;

public:
    uint32_t id = 0;
    uint32_t user = 0;
    std::string user_name = "";
    std::string title = "";
    std::string created_date = "";

public:
    ArticleSummary() = default;

    ArticleSummary(const ArticleSummary& x)
        : id(x.id), user(x.user), user_name(x.user_name), title(x.title), created_date(x.created_date)
    { }

    ArticleSummary(uint32_t id, uint32_t user, const std::string& user_name, const std::string& title, const std::string& created_date)
        : id(id), user(user), user_name(user_name), title(title), created_date(created_date)
    { }

    ArticleSummary(const fb::protocol::db::raw::ArticleSummary& raw)
        : id(raw.id()), user(raw.user()), user_name(raw.user_name()->c_str()), title(raw.title()->c_str()), created_date(raw.created_date()->c_str())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::raw::CreateArticleSummary(builder,
            this->id,
            this->user,
            builder.CreateString(this->user_name),
            builder.CreateString(this->title),
            builder.CreateString(this->created_date));
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static ArticleSummary Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::raw::GetArticleSummary(bytes);
        return ArticleSummary(*raw);
    }
};
class Article
{
public:
    static inline fb::protocol::db::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::FlatBufferProtocolType::Article;

public:
    uint32_t id = 0;
    uint32_t user = 0;
    std::string user_name = "";
    std::string title = "";
    std::string contents = "";
    std::string created_date = "";

public:
    Article() = default;

    Article(const Article& x)
        : id(x.id), user(x.user), user_name(x.user_name), title(x.title), contents(x.contents), created_date(x.created_date)
    { }

    Article(uint32_t id, uint32_t user, const std::string& user_name, const std::string& title, const std::string& contents, const std::string& created_date)
        : id(id), user(user), user_name(user_name), title(title), contents(contents), created_date(created_date)
    { }

    Article(const fb::protocol::db::raw::Article& raw)
        : id(raw.id()), user(raw.user()), user_name(raw.user_name()->c_str()), title(raw.title()->c_str()), contents(raw.contents()->c_str()), created_date(raw.created_date()->c_str())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::raw::CreateArticle(builder,
            this->id,
            this->user,
            builder.CreateString(this->user_name),
            builder.CreateString(this->title),
            builder.CreateString(this->contents),
            builder.CreateString(this->created_date));
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Article Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::raw::GetArticle(bytes);
        return Article(*raw);
    }
};
} } } 
namespace fb { namespace protocol { namespace db { namespace request { 

enum class FlatBufferProtocolType
{
    Account,
    ChangePw,
    InitCharacter,
    Login,
    MakeCharacter,
    ReserveName,
    Save,
    GetArticle,
    GetArticleList,
    WriteArticle,
    DeleteArticle,
};

class Account
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::Account;

public:
    uint32_t uid = 0;

public:
    Account() = default;

    Account(const Account& x)
        : uid(x.uid)
    { }

    Account(uint32_t uid)
        : uid(uid)
    { }

    Account(const fb::protocol::db::request::raw::Account& raw)
        : uid(raw.uid())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::raw::CreateAccount(builder,
            this->uid);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Account Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetAccount(bytes);
        return Account(*raw);
    }
};
class ChangePw
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::ChangePw;

public:
    uint32_t uid = 0;
    std::string before = "";
    std::string after = "";
    uint32_t birthday = 0;

public:
    ChangePw() = default;

    ChangePw(const ChangePw& x)
        : uid(x.uid), before(x.before), after(x.after), birthday(x.birthday)
    { }

    ChangePw(uint32_t uid, const std::string& before, const std::string& after, uint32_t birthday)
        : uid(uid), before(before), after(after), birthday(birthday)
    { }

    ChangePw(const fb::protocol::db::request::raw::ChangePw& raw)
        : uid(raw.uid()), before(raw.before()->c_str()), after(raw.after()->c_str()), birthday(raw.birthday())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::raw::CreateChangePw(builder,
            this->uid,
            builder.CreateString(this->before),
            builder.CreateString(this->after),
            this->birthday);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static ChangePw Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetChangePw(bytes);
        return ChangePw(*raw);
    }
};
class InitCharacter
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::InitCharacter;

public:
    uint32_t uid = 0;
    std::string name = "";
    std::string pw = "";
    uint32_t hp = 0;
    uint32_t mp = 0;
    uint16_t map = 0;
    uint16_t x = 0;
    uint16_t y = 0;
    bool admin = false;

public:
    InitCharacter() = default;

    InitCharacter(const InitCharacter& x)
        : uid(x.uid), name(x.name), pw(x.pw), hp(x.hp), mp(x.mp), map(x.map), x(x.x), y(x.y), admin(x.admin)
    { }

    InitCharacter(uint32_t uid, const std::string& name, const std::string& pw, uint32_t hp, uint32_t mp, uint16_t map, uint16_t x, uint16_t y, bool admin)
        : uid(uid), name(name), pw(pw), hp(hp), mp(mp), map(map), x(x), y(y), admin(admin)
    { }

    InitCharacter(const fb::protocol::db::request::raw::InitCharacter& raw)
        : uid(raw.uid()), name(raw.name()->c_str()), pw(raw.pw()->c_str()), hp(raw.hp()), mp(raw.mp()), map(raw.map()), x(raw.x()), y(raw.y()), admin(raw.admin())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::raw::CreateInitCharacter(builder,
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
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static InitCharacter Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetInitCharacter(bytes);
        return InitCharacter(*raw);
    }
};
class Login
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::Login;

public:
    uint32_t uid = 0;

public:
    Login() = default;

    Login(const Login& x)
        : uid(x.uid)
    { }

    Login(uint32_t uid)
        : uid(uid)
    { }

    Login(const fb::protocol::db::request::raw::Login& raw)
        : uid(raw.uid())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::raw::CreateLogin(builder,
            this->uid);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Login Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetLogin(bytes);
        return Login(*raw);
    }
};
class MakeCharacter
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::MakeCharacter;

public:
    uint32_t uid = 0;
    uint16_t hair = 0;
    uint8_t sex = 0;
    uint8_t nation = 0;
    uint8_t creature = 0;

public:
    MakeCharacter() = default;

    MakeCharacter(const MakeCharacter& x)
        : uid(x.uid), hair(x.hair), sex(x.sex), nation(x.nation), creature(x.creature)
    { }

    MakeCharacter(uint32_t uid, uint16_t hair, uint8_t sex, uint8_t nation, uint8_t creature)
        : uid(uid), hair(hair), sex(sex), nation(nation), creature(creature)
    { }

    MakeCharacter(const fb::protocol::db::request::raw::MakeCharacter& raw)
        : uid(raw.uid()), hair(raw.hair()), sex(raw.sex()), nation(raw.nation()), creature(raw.creature())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::raw::CreateMakeCharacter(builder,
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
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static MakeCharacter Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetMakeCharacter(bytes);
        return MakeCharacter(*raw);
    }
};
class ReserveName
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::ReserveName;

public:
    std::string name = "";

public:
    ReserveName() = default;

    ReserveName(const ReserveName& x)
        : name(x.name)
    { }

    ReserveName(const std::string& name)
        : name(name)
    { }

    ReserveName(const fb::protocol::db::request::raw::ReserveName& raw)
        : name(raw.name()->c_str())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::raw::CreateReserveName(builder,
            builder.CreateString(this->name));
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static ReserveName Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetReserveName(bytes);
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
    Save() = default;

    Save(const Save& x)
        : character(x.character), items(x.items), spells(x.spells)
    { }

    Save(const fb::protocol::db::Character& character, std::vector<fb::protocol::db::Item> items, std::vector<fb::protocol::db::Spell> spells)
        : character(character), items(items), spells(spells)
    { }

    Save(const fb::protocol::db::request::raw::Save& raw)
        : character(*raw.character())
    {
        for (int i = 0; i < raw.items()->size(); i++)
        {
            this->items.push_back(*raw.items()->Get(i));
        }
        for (int i = 0; i < raw.spells()->size(); i++)
        {
            this->spells.push_back(*raw.spells()->Get(i));
        }
    }

private:
    auto CreateItems(flatbuffers::FlatBufferBuilder& builder) const
    {
        auto result = std::vector<flatbuffers::Offset<fb::protocol::db::raw::Item>>();
        for (auto& x : this->items)
        {
            result.push_back(x.Build(builder));
        }
        return builder.CreateVector(result);
    }
    auto CreateSpells(flatbuffers::FlatBufferBuilder& builder) const
    {
        auto result = std::vector<flatbuffers::Offset<fb::protocol::db::raw::Spell>>();
        for (auto& x : this->spells)
        {
            result.push_back(x.Build(builder));
        }
        return builder.CreateVector(result);
    }

public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::raw::CreateSave(builder,
            this->character.Build(builder),
            this->CreateItems(builder),
            this->CreateSpells(builder));
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Save Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetSave(bytes);
        return Save(*raw);
    }
};
class GetArticle
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::GetArticle;

public:
    uint32_t section = 0;
    uint32_t article = 0;

public:
    GetArticle() = default;

    GetArticle(const GetArticle& x)
        : section(x.section), article(x.article)
    { }

    GetArticle(uint32_t section, uint32_t article)
        : section(section), article(article)
    { }

    GetArticle(const fb::protocol::db::request::raw::GetArticle& raw)
        : section(raw.section()), article(raw.article())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::raw::CreateGetArticle(builder,
            this->section,
            this->article);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static GetArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetGetArticle(bytes);
        return GetArticle(*raw);
    }
};
class GetArticleList
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::GetArticleList;

public:
    uint32_t section = 0;
    uint32_t position = 0;

public:
    GetArticleList() = default;

    GetArticleList(const GetArticleList& x)
        : section(x.section), position(x.position)
    { }

    GetArticleList(uint32_t section, uint32_t position)
        : section(section), position(position)
    { }

    GetArticleList(const fb::protocol::db::request::raw::GetArticleList& raw)
        : section(raw.section()), position(raw.position())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::raw::CreateGetArticleList(builder,
            this->section,
            this->position);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static GetArticleList Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetGetArticleList(bytes);
        return GetArticleList(*raw);
    }
};
class WriteArticle
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::WriteArticle;

public:
    uint32_t section = 0;
    uint32_t user = 0;
    std::string title = "";
    std::string contents = "";

public:
    WriteArticle() = default;

    WriteArticle(const WriteArticle& x)
        : section(x.section), user(x.user), title(x.title), contents(x.contents)
    { }

    WriteArticle(uint32_t section, uint32_t user, const std::string& title, const std::string& contents)
        : section(section), user(user), title(title), contents(contents)
    { }

    WriteArticle(const fb::protocol::db::request::raw::WriteArticle& raw)
        : section(raw.section()), user(raw.user()), title(raw.title()->c_str()), contents(raw.contents()->c_str())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::raw::CreateWriteArticle(builder,
            this->section,
            this->user,
            builder.CreateString(this->title),
            builder.CreateString(this->contents));
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static WriteArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetWriteArticle(bytes);
        return WriteArticle(*raw);
    }
};
class DeleteArticle
{
public:
    static inline fb::protocol::db::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::request::FlatBufferProtocolType::DeleteArticle;

public:
    uint32_t id = 0;
    uint32_t user = 0;

public:
    DeleteArticle() = default;

    DeleteArticle(const DeleteArticle& x)
        : id(x.id), user(x.user)
    { }

    DeleteArticle(uint32_t id, uint32_t user)
        : id(id), user(user)
    { }

    DeleteArticle(const fb::protocol::db::request::raw::DeleteArticle& raw)
        : id(raw.id()), user(raw.user())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::request::raw::CreateDeleteArticle(builder,
            this->id,
            this->user);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static DeleteArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::request::raw::GetDeleteArticle(bytes);
        return DeleteArticle(*raw);
    }
};
} } } } 
namespace fb { namespace protocol { namespace db { namespace response { 

enum class FlatBufferProtocolType
{
    Account,
    DeleteArticle,
    GetArticle,
    GetArticleList,
    WriteArticle,
    ChangePw,
    GetUid,
    InitCharacter,
    Login,
    MakeCharacter,
    ReserveName,
    Save,
};

class Account
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::Account;

public:
    std::string pw = "";
    uint32_t map = 0;
    bool success = false;

public:
    Account() = default;

    Account(const Account& x)
        : pw(x.pw), map(x.map), success(x.success)
    { }

    Account(const std::string& pw, uint32_t map, bool success)
        : pw(pw), map(map), success(success)
    { }

    Account(const fb::protocol::db::response::raw::Account& raw)
        : pw(raw.pw()->c_str()), map(raw.map()), success(raw.success())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::raw::CreateAccount(builder,
            builder.CreateString(this->pw),
            this->map,
            this->success);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Account Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetAccount(bytes);
        return Account(*raw);
    }
};
class DeleteArticle
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::DeleteArticle;

public:
    int result = 0;

public:
    DeleteArticle() = default;

    DeleteArticle(const DeleteArticle& x)
        : result(x.result)
    { }

    DeleteArticle(int result)
        : result(result)
    { }

    DeleteArticle(const fb::protocol::db::response::raw::DeleteArticle& raw)
        : result(raw.result())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::raw::CreateDeleteArticle(builder,
            this->result);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static DeleteArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetDeleteArticle(bytes);
        return DeleteArticle(*raw);
    }
};
class GetArticle
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::GetArticle;

public:
    fb::protocol::db::Article article;
    bool next = false;
    bool success = false;

public:
    GetArticle() = default;

    GetArticle(const GetArticle& x)
        : article(x.article), next(x.next), success(x.success)
    { }

    GetArticle(const fb::protocol::db::Article& article, bool next, bool success)
        : article(article), next(next), success(success)
    { }

    GetArticle(const fb::protocol::db::response::raw::GetArticle& raw)
        : article(*raw.article()), next(raw.next()), success(raw.success())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::raw::CreateGetArticle(builder,
            this->article.Build(builder),
            this->next,
            this->success);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static GetArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetGetArticle(bytes);
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
    GetArticleList() = default;

    GetArticleList(const GetArticleList& x)
        : summary_list(x.summary_list)
    { }

    GetArticleList(std::vector<fb::protocol::db::ArticleSummary> summary_list)
        : summary_list(summary_list)
    { }

    GetArticleList(const fb::protocol::db::response::raw::GetArticleList& raw)
    {
        for (int i = 0; i < raw.summary_list()->size(); i++)
        {
            this->summary_list.push_back(*raw.summary_list()->Get(i));
        }
    }

private:
    auto CreateSummary_list(flatbuffers::FlatBufferBuilder& builder) const
    {
        auto result = std::vector<flatbuffers::Offset<fb::protocol::db::raw::ArticleSummary>>();
        for (auto& x : this->summary_list)
        {
            result.push_back(x.Build(builder));
        }
        return builder.CreateVector(result);
    }

public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::raw::CreateGetArticleList(builder,
            this->CreateSummary_list(builder));
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static GetArticleList Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetGetArticleList(bytes);
        return GetArticleList(*raw);
    }
};
class WriteArticle
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::WriteArticle;

public:
    bool success = false;

public:
    WriteArticle() = default;

    WriteArticle(const WriteArticle& x)
        : success(x.success)
    { }

    WriteArticle(bool success)
        : success(success)
    { }

    WriteArticle(const fb::protocol::db::response::raw::WriteArticle& raw)
        : success(raw.success())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::raw::CreateWriteArticle(builder,
            this->success);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static WriteArticle Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetWriteArticle(bytes);
        return WriteArticle(*raw);
    }
};
class ChangePw
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::ChangePw;

public:
    int result = 0;

public:
    ChangePw() = default;

    ChangePw(const ChangePw& x)
        : result(x.result)
    { }

    ChangePw(int result)
        : result(result)
    { }

    ChangePw(const fb::protocol::db::response::raw::ChangePw& raw)
        : result(raw.result())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::raw::CreateChangePw(builder,
            this->result);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static ChangePw Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetChangePw(bytes);
        return ChangePw(*raw);
    }
};
class GetUid
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::GetUid;

public:
    uint32_t uid = 0;
    bool success = false;

public:
    GetUid() = default;

    GetUid(const GetUid& x)
        : uid(x.uid), success(x.success)
    { }

    GetUid(uint32_t uid, bool success)
        : uid(uid), success(success)
    { }

    GetUid(const fb::protocol::db::response::raw::GetUid& raw)
        : uid(raw.uid()), success(raw.success())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::raw::CreateGetUid(builder,
            this->uid,
            this->success);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static GetUid Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetGetUid(bytes);
        return GetUid(*raw);
    }
};
class InitCharacter
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::InitCharacter;

public:
    bool success = false;

public:
    InitCharacter() = default;

    InitCharacter(const InitCharacter& x)
        : success(x.success)
    { }

    InitCharacter(bool success)
        : success(success)
    { }

    InitCharacter(const fb::protocol::db::response::raw::InitCharacter& raw)
        : success(raw.success())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::raw::CreateInitCharacter(builder,
            this->success);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static InitCharacter Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetInitCharacter(bytes);
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
    Login() = default;

    Login(const Login& x)
        : character(x.character), items(x.items), spells(x.spells)
    { }

    Login(const fb::protocol::db::Character& character, std::vector<fb::protocol::db::Item> items, std::vector<fb::protocol::db::Spell> spells)
        : character(character), items(items), spells(spells)
    { }

    Login(const fb::protocol::db::response::raw::Login& raw)
        : character(*raw.character())
    {
        for (int i = 0; i < raw.items()->size(); i++)
        {
            this->items.push_back(*raw.items()->Get(i));
        }
        for (int i = 0; i < raw.spells()->size(); i++)
        {
            this->spells.push_back(*raw.spells()->Get(i));
        }
    }

private:
    auto CreateItems(flatbuffers::FlatBufferBuilder& builder) const
    {
        auto result = std::vector<flatbuffers::Offset<fb::protocol::db::raw::Item>>();
        for (auto& x : this->items)
        {
            result.push_back(x.Build(builder));
        }
        return builder.CreateVector(result);
    }
    auto CreateSpells(flatbuffers::FlatBufferBuilder& builder) const
    {
        auto result = std::vector<flatbuffers::Offset<fb::protocol::db::raw::Spell>>();
        for (auto& x : this->spells)
        {
            result.push_back(x.Build(builder));
        }
        return builder.CreateVector(result);
    }

public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::raw::CreateLogin(builder,
            this->character.Build(builder),
            this->CreateItems(builder),
            this->CreateSpells(builder));
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Login Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetLogin(bytes);
        return Login(*raw);
    }
};
class MakeCharacter
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::MakeCharacter;

public:
    bool success = false;

public:
    MakeCharacter() = default;

    MakeCharacter(const MakeCharacter& x)
        : success(x.success)
    { }

    MakeCharacter(bool success)
        : success(success)
    { }

    MakeCharacter(const fb::protocol::db::response::raw::MakeCharacter& raw)
        : success(raw.success())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::raw::CreateMakeCharacter(builder,
            this->success);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static MakeCharacter Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetMakeCharacter(bytes);
        return MakeCharacter(*raw);
    }
};
class ReserveName
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::ReserveName;

public:
    uint32_t uid = 0;
    bool success = false;

public:
    ReserveName() = default;

    ReserveName(const ReserveName& x)
        : uid(x.uid), success(x.success)
    { }

    ReserveName(uint32_t uid, bool success)
        : uid(uid), success(success)
    { }

    ReserveName(const fb::protocol::db::response::raw::ReserveName& raw)
        : uid(raw.uid()), success(raw.success())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::raw::CreateReserveName(builder,
            this->uid,
            this->success);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static ReserveName Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetReserveName(bytes);
        return ReserveName(*raw);
    }
};
class Save
{
public:
    static inline fb::protocol::db::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::db::response::FlatBufferProtocolType::Save;

public:
    bool success = false;

public:
    Save() = default;

    Save(const Save& x)
        : success(x.success)
    { }

    Save(bool success)
        : success(success)
    { }

    Save(const fb::protocol::db::response::raw::Save& raw)
        : success(raw.success())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::response::raw::CreateSave(builder,
            this->success);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Save Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::db::response::raw::GetSave(bytes);
        return Save(*raw);
    }
};
} } } } 
namespace fb { namespace protocol { namespace internal { 

enum class FlatBufferProtocolType
{
};

} } } 
namespace fb { namespace protocol { namespace internal { namespace request { 

enum class FlatBufferProtocolType
{
    Login,
    Logout,
    Ping,
    Transfer,
    Whisper,
    KickOut,
};

class Login
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Login;

public:
    uint32_t uid = 0;
    std::string name = "";
    uint16_t map = 0;

public:
    Login() = default;

    Login(const Login& x)
        : uid(x.uid), name(x.name), map(x.map)
    { }

    Login(uint32_t uid, const std::string& name, uint16_t map)
        : uid(uid), name(name), map(map)
    { }

    Login(const fb::protocol::internal::request::raw::Login& raw)
        : uid(raw.uid()), name(raw.name()->c_str()), map(raw.map())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::request::raw::CreateLogin(builder,
            this->uid,
            builder.CreateString(this->name),
            this->map);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Login Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetLogin(bytes);
        return Login(*raw);
    }
};
class Logout
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Logout;

public:
    std::string name = "";

public:
    Logout() = default;

    Logout(const Logout& x)
        : name(x.name)
    { }

    Logout(const std::string& name)
        : name(name)
    { }

    Logout(const fb::protocol::internal::request::raw::Logout& raw)
        : name(raw.name()->c_str())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::request::raw::CreateLogout(builder,
            builder.CreateString(this->name));
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Logout Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetLogout(bytes);
        return Logout(*raw);
    }
};
class Ping
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Ping;

public:
    uint8_t id = 0;
    std::string name = "";
    fb::protocol::internal::Service service;
    std::string ip = "";
    uint16_t port = 0;

public:
    Ping() = default;

    Ping(const Ping& x)
        : id(x.id), name(x.name), service(x.service), ip(x.ip), port(x.port)
    { }

    Ping(uint8_t id, const std::string& name, fb::protocol::internal::Service service, const std::string& ip, uint16_t port)
        : id(id), name(name), service(service), ip(ip), port(port)
    { }

    Ping(const fb::protocol::internal::request::raw::Ping& raw)
        : id(raw.id()), name(raw.name()->c_str()), service((fb::protocol::internal::Service)raw.service()), ip(raw.ip()->c_str()), port(raw.port())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::request::raw::CreatePing(builder,
            this->id,
            builder.CreateString(this->name),
            (fb::protocol::internal::raw::Service)this->service,
            builder.CreateString(this->ip),
            this->port);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Ping Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetPing(bytes);
        return Ping(*raw);
    }
};
class Transfer
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Transfer;

public:
    fb::protocol::internal::Service service;
    uint8_t id = 0;

public:
    Transfer() = default;

    Transfer(const Transfer& x)
        : service(x.service), id(x.id)
    { }

    Transfer(fb::protocol::internal::Service service, uint8_t id)
        : service(service), id(id)
    { }

    Transfer(const fb::protocol::internal::request::raw::Transfer& raw)
        : service((fb::protocol::internal::Service)raw.service()), id(raw.id())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::request::raw::CreateTransfer(builder,
            (fb::protocol::internal::raw::Service)this->service,
            this->id);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Transfer Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetTransfer(bytes);
        return Transfer(*raw);
    }
};
class Whisper
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::Whisper;

public:
    std::string from = "";
    std::string to = "";
    std::string message = "";

public:
    Whisper() = default;

    Whisper(const Whisper& x)
        : from(x.from), to(x.to), message(x.message)
    { }

    Whisper(const std::string& from, const std::string& to, const std::string& message)
        : from(from), to(to), message(message)
    { }

    Whisper(const fb::protocol::internal::request::raw::Whisper& raw)
        : from(raw.from()->c_str()), to(raw.to()->c_str()), message(raw.message()->c_str())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::request::raw::CreateWhisper(builder,
            builder.CreateString(this->from),
            builder.CreateString(this->to),
            builder.CreateString(this->message));
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Whisper Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetWhisper(bytes);
        return Whisper(*raw);
    }
};
class KickOut
{
public:
    static inline fb::protocol::internal::request::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::request::FlatBufferProtocolType::KickOut;

public:
    uint32_t uid = 0;

public:
    KickOut() = default;

    KickOut(const KickOut& x)
        : uid(x.uid)
    { }

    KickOut(uint32_t uid)
        : uid(uid)
    { }

    KickOut(const fb::protocol::internal::request::raw::KickOut& raw)
        : uid(raw.uid())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::request::raw::CreateKickOut(builder,
            this->uid);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static KickOut Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::request::raw::GetKickOut(bytes);
        return KickOut(*raw);
    }
};
} } } } 
namespace fb { namespace protocol { namespace internal { namespace response { 

enum class FlatBufferProtocolType
{
    KickOut,
    Login,
    Logout,
    Pong,
    Transfer,
    Whisper,
};

class KickOut
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::KickOut;

public:
    uint32_t uid = 0;

public:
    KickOut() = default;

    KickOut(const KickOut& x)
        : uid(x.uid)
    { }

    KickOut(uint32_t uid)
        : uid(uid)
    { }

    KickOut(const fb::protocol::internal::response::raw::KickOut& raw)
        : uid(raw.uid())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::response::raw::CreateKickOut(builder,
            this->uid);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static KickOut Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetKickOut(bytes);
        return KickOut(*raw);
    }
};
class Login
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::Login;

public:
    bool success = false;
    bool logon = false;
    std::string ip = "";
    uint16_t port = 0;

public:
    Login() = default;

    Login(const Login& x)
        : success(x.success), logon(x.logon), ip(x.ip), port(x.port)
    { }

    Login(bool success, bool logon, const std::string& ip, uint16_t port)
        : success(success), logon(logon), ip(ip), port(port)
    { }

    Login(const fb::protocol::internal::response::raw::Login& raw)
        : success(raw.success()), logon(raw.logon()), ip(raw.ip()->c_str()), port(raw.port())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::response::raw::CreateLogin(builder,
            this->success,
            this->logon,
            builder.CreateString(this->ip),
            this->port);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Login Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetLogin(bytes);
        return Login(*raw);
    }
};
class Logout
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::Logout;

public:
    bool success = false;

public:
    Logout() = default;

    Logout(const Logout& x)
        : success(x.success)
    { }

    Logout(bool success)
        : success(success)
    { }

    Logout(const fb::protocol::internal::response::raw::Logout& raw)
        : success(raw.success())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::response::raw::CreateLogout(builder,
            this->success);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Logout Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetLogout(bytes);
        return Logout(*raw);
    }
};
class Pong
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::Pong;


public:
    Pong() = default;

    Pong(const Pong& x)
    { }


    Pong(const fb::protocol::internal::response::raw::Pong& raw)
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::response::raw::CreatePong(builder);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Pong Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetPong(bytes);
        return Pong(*raw);
    }
};
class Transfer
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::Transfer;

public:
    fb::protocol::internal::TransferResult code;
    std::string ip = "";
    uint16_t port = 0;

public:
    Transfer() = default;

    Transfer(const Transfer& x)
        : code(x.code), ip(x.ip), port(x.port)
    { }

    Transfer(fb::protocol::internal::TransferResult code, const std::string& ip, uint16_t port)
        : code(code), ip(ip), port(port)
    { }

    Transfer(const fb::protocol::internal::response::raw::Transfer& raw)
        : code((fb::protocol::internal::TransferResult)raw.code()), ip(raw.ip()->c_str()), port(raw.port())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::response::raw::CreateTransfer(builder,
            (fb::protocol::internal::raw::TransferResult)this->code,
            builder.CreateString(this->ip),
            this->port);
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Transfer Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetTransfer(bytes);
        return Transfer(*raw);
    }
};
class Whisper
{
public:
    static inline fb::protocol::internal::response::FlatBufferProtocolType FlatBufferProtocolType = fb::protocol::internal::response::FlatBufferProtocolType::Whisper;

public:
    bool success = false;
    std::string from = "";
    uint32_t to = 0;
    std::string message = "";

public:
    Whisper() = default;

    Whisper(const Whisper& x)
        : success(x.success), from(x.from), to(x.to), message(x.message)
    { }

    Whisper(bool success, const std::string& from, uint32_t to, const std::string& message)
        : success(success), from(from), to(to), message(message)
    { }

    Whisper(const fb::protocol::internal::response::raw::Whisper& raw)
        : success(raw.success()), from(raw.from()->c_str()), to(raw.to()), message(raw.message()->c_str())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::internal::response::raw::CreateWhisper(builder,
            this->success,
            builder.CreateString(this->from),
            this->to,
            builder.CreateString(this->message));
    }

    std::vector<uint8_t> Serialize() const
    {
        auto builder = flatbuffers::FlatBufferBuilder();
        builder.Finish(this->Build(builder));
        auto size = builder.GetSize();
        auto result = std::vector<uint8_t>(size);
        std::memcpy(result.data(), builder.GetBufferPointer(), size);
        return result;
    }

    static Whisper Deserialize(const uint8_t* bytes)
    {
        auto raw = fb::protocol::internal::response::raw::GetWhisper(bytes);
        return Whisper(*raw);
    }
};
} } } } 

#endif