#ifndef __FB_PROTOCOL_DB___
#define __FB_PROTOCOL_DB___

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/origin/db_generated.h>
#include <fb/protocol/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace protocol { namespace db { 


class nullopt_ushort
{

public:
    uint16_t value;
    bool empty;

public:
    nullopt_ushort()
    { }

    nullopt_ushort(const nullopt_ushort& x)
        : value(x.value), empty(x.empty)
    { }

    nullopt_ushort(uint16_t value, bool empty)
        : value(value), empty(empty)
    { }
    nullopt_ushort(const fb::protocol::db::origin::nullopt_ushort& raw)
        : value(raw.value()), empty(raw.empty())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::origin::Createnullopt_ushort(builder,
            this->value,
            this->empty);
    }

};

class nullopt_ubyte
{

public:
    uint8_t value;
    bool empty;

public:
    nullopt_ubyte()
    { }

    nullopt_ubyte(const nullopt_ubyte& x)
        : value(x.value), empty(x.empty)
    { }

    nullopt_ubyte(uint8_t value, bool empty)
        : value(value), empty(empty)
    { }
    nullopt_ubyte(const fb::protocol::db::origin::nullopt_ubyte& raw)
        : value(raw.value()), empty(raw.empty())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::origin::Createnullopt_ubyte(builder,
            this->value,
            this->empty);
    }

};

class nullopt_uint
{

public:
    uint32_t value;
    bool empty;

public:
    nullopt_uint()
    { }

    nullopt_uint(const nullopt_uint& x)
        : value(x.value), empty(x.empty)
    { }

    nullopt_uint(uint32_t value, bool empty)
        : value(value), empty(empty)
    { }
    nullopt_uint(const fb::protocol::db::origin::nullopt_uint& raw)
        : value(raw.value()), empty(raw.empty())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::origin::Createnullopt_uint(builder,
            this->value,
            this->empty);
    }

};

class Position
{

public:
    uint16_t x;
    uint16_t y;

public:
    Position()
    { }

    Position(const Position& x)
        : x(x.x), y(x.y)
    { }

    Position(uint16_t x, uint16_t y)
        : x(x), y(y)
    { }
    Position(const fb::protocol::db::origin::Position& raw)
        : x(raw.x()), y(raw.y())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::origin::CreatePosition(builder,
            this->x,
            this->y);
    }

};

class Character
{

public:
    uint32_t id;
    std::string name;
    std::string last_login;
    bool admin;
    uint16_t look;
    uint16_t color;
    uint16_t sex;
    uint16_t nation;
    nullopt_ushort creature;
    uint32_t map;
    Position position;
    uint8_t direction;
    uint8_t state;
    uint8_t class_type;
    uint8_t promotion;
    uint32_t exp;
    uint32_t money;
    uint32_t deposited_money;
    nullopt_ushort disguise;
    uint32_t hp;
    uint32_t base_hp;
    uint32_t additional_hp;
    uint32_t mp;
    uint32_t base_mp;
    uint32_t additional_mp;
    nullopt_ubyte weapon_color;
    nullopt_ubyte helmet_color;
    nullopt_ubyte armor_color;
    nullopt_ubyte shield_color;
    nullopt_ubyte ring_left_color;
    nullopt_ubyte ring_right_color;
    nullopt_ubyte aux_top_color;
    nullopt_ubyte aux_bot_color;
    nullopt_uint clan;

public:
    Character()
    { }

    Character(const Character& x)
        : id(x.id), name(x.name), last_login(x.last_login), admin(x.admin), look(x.look), color(x.color), sex(x.sex), nation(x.nation), creature(x.creature), map(x.map), position(x.position), direction(x.direction), state(x.state), class_type(x.class_type), promotion(x.promotion), exp(x.exp), money(x.money), deposited_money(x.deposited_money), disguise(x.disguise), hp(x.hp), base_hp(x.base_hp), additional_hp(x.additional_hp), mp(x.mp), base_mp(x.base_mp), additional_mp(x.additional_mp), weapon_color(x.weapon_color), helmet_color(x.helmet_color), armor_color(x.armor_color), shield_color(x.shield_color), ring_left_color(x.ring_left_color), ring_right_color(x.ring_right_color), aux_top_color(x.aux_top_color), aux_bot_color(x.aux_bot_color), clan(x.clan)
    { }

    Character(uint32_t id, const std::string& name, const std::string& last_login, bool admin, uint16_t look, uint16_t color, uint16_t sex, uint16_t nation, const nullopt_ushort& creature, uint32_t map, const Position& position, uint8_t direction, uint8_t state, uint8_t class_type, uint8_t promotion, uint32_t exp, uint32_t money, uint32_t deposited_money, const nullopt_ushort& disguise, uint32_t hp, uint32_t base_hp, uint32_t additional_hp, uint32_t mp, uint32_t base_mp, uint32_t additional_mp, const nullopt_ubyte& weapon_color, const nullopt_ubyte& helmet_color, const nullopt_ubyte& armor_color, const nullopt_ubyte& shield_color, const nullopt_ubyte& ring_left_color, const nullopt_ubyte& ring_right_color, const nullopt_ubyte& aux_top_color, const nullopt_ubyte& aux_bot_color, const nullopt_uint& clan)
        : id(id), name(name), last_login(last_login), admin(admin), look(look), color(color), sex(sex), nation(nation), creature(creature), map(map), position(position), direction(direction), state(state), class_type(class_type), promotion(promotion), exp(exp), money(money), deposited_money(deposited_money), disguise(disguise), hp(hp), base_hp(base_hp), additional_hp(additional_hp), mp(mp), base_mp(base_mp), additional_mp(additional_mp), weapon_color(weapon_color), helmet_color(helmet_color), armor_color(armor_color), shield_color(shield_color), ring_left_color(ring_left_color), ring_right_color(ring_right_color), aux_top_color(aux_top_color), aux_bot_color(aux_bot_color), clan(clan)
    { }
    Character(const fb::protocol::db::origin::Character& raw)
        : id(raw.id()), name(raw.name() != nullptr ? raw.name()->c_str() : ""), last_login(raw.last_login() != nullptr ? raw.last_login()->c_str() : ""), admin(raw.admin()), look(raw.look()), color(raw.color()), sex(raw.sex()), nation(raw.nation()), creature(*raw.creature()), map(raw.map()), position(*raw.position()), direction(raw.direction()), state(raw.state()), class_type(raw.class_type()), promotion(raw.promotion()), exp(raw.exp()), money(raw.money()), deposited_money(raw.deposited_money()), disguise(*raw.disguise()), hp(raw.hp()), base_hp(raw.base_hp()), additional_hp(raw.additional_hp()), mp(raw.mp()), base_mp(raw.base_mp()), additional_mp(raw.additional_mp()), weapon_color(*raw.weapon_color()), helmet_color(*raw.helmet_color()), armor_color(*raw.armor_color()), shield_color(*raw.shield_color()), ring_left_color(*raw.ring_left_color()), ring_right_color(*raw.ring_right_color()), aux_top_color(*raw.aux_top_color()), aux_bot_color(*raw.aux_bot_color()), clan(*raw.clan())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::origin::CreateCharacter(builder,
            this->id,
            builder.CreateString(this->name),
            builder.CreateString(this->last_login),
            this->admin,
            this->look,
            this->color,
            this->sex,
            this->nation,
            this->creature.Build(builder),
            this->map,
            this->position.Build(builder),
            this->direction,
            this->state,
            this->class_type,
            this->promotion,
            this->exp,
            this->money,
            this->deposited_money,
            this->disguise.Build(builder),
            this->hp,
            this->base_hp,
            this->additional_hp,
            this->mp,
            this->base_mp,
            this->additional_mp,
            this->weapon_color.Build(builder),
            this->helmet_color.Build(builder),
            this->armor_color.Build(builder),
            this->shield_color.Build(builder),
            this->ring_left_color.Build(builder),
            this->ring_right_color.Build(builder),
            this->aux_top_color.Build(builder),
            this->aux_bot_color.Build(builder),
            this->clan.Build(builder));
    }

};

class Item
{

public:
    uint32_t user;
    int16_t index;
    int16_t parts;
    int16_t deposited;
    uint32_t model;
    uint16_t count;
    nullopt_uint durability;
    std::string custom_name;

public:
    Item()
    { }

    Item(const Item& x)
        : user(x.user), index(x.index), parts(x.parts), deposited(x.deposited), model(x.model), count(x.count), durability(x.durability), custom_name(x.custom_name)
    { }

    Item(uint32_t user, int16_t index, int16_t parts, int16_t deposited, uint32_t model, uint16_t count, const nullopt_uint& durability, const std::string& custom_name)
        : user(user), index(index), parts(parts), deposited(deposited), model(model), count(count), durability(durability), custom_name(custom_name)
    { }
    Item(const fb::protocol::db::origin::Item& raw)
        : user(raw.user()), index(raw.index()), parts(raw.parts()), deposited(raw.deposited()), model(raw.model()), count(raw.count()), durability(*raw.durability()), custom_name(raw.custom_name() != nullptr ? raw.custom_name()->c_str() : "")
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::origin::CreateItem(builder,
            this->user,
            this->index,
            this->parts,
            this->deposited,
            this->model,
            this->count,
            this->durability.Build(builder),
            builder.CreateString(this->custom_name));
    }

};

class Spell
{

public:
    uint32_t user;
    uint8_t slot;
    uint32_t id;

public:
    Spell()
    { }

    Spell(const Spell& x)
        : user(x.user), slot(x.slot), id(x.id)
    { }

    Spell(uint32_t user, uint8_t slot, uint32_t id)
        : user(user), slot(slot), id(id)
    { }
    Spell(const fb::protocol::db::origin::Spell& raw)
        : user(raw.user()), slot(raw.slot()), id(raw.id())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::origin::CreateSpell(builder,
            this->user,
            this->slot,
            this->id);
    }

};

class ArticleSummary
{

public:
    uint32_t id;
    uint32_t user;
    std::string user_name;
    std::string title;
    std::string created_date;

public:
    ArticleSummary()
    { }

    ArticleSummary(const ArticleSummary& x)
        : id(x.id), user(x.user), user_name(x.user_name), title(x.title), created_date(x.created_date)
    { }

    ArticleSummary(uint32_t id, uint32_t user, const std::string& user_name, const std::string& title, const std::string& created_date)
        : id(id), user(user), user_name(user_name), title(title), created_date(created_date)
    { }
    ArticleSummary(const fb::protocol::db::origin::ArticleSummary& raw)
        : id(raw.id()), user(raw.user()), user_name(raw.user_name() != nullptr ? raw.user_name()->c_str() : ""), title(raw.title() != nullptr ? raw.title()->c_str() : ""), created_date(raw.created_date() != nullptr ? raw.created_date()->c_str() : "")
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::origin::CreateArticleSummary(builder,
            this->id,
            this->user,
            builder.CreateString(this->user_name),
            builder.CreateString(this->title),
            builder.CreateString(this->created_date));
    }

};

class Article
{

public:
    uint32_t id;
    uint32_t user;
    std::string user_name;
    std::string title;
    std::string contents;
    std::string created_date;

public:
    Article()
    { }

    Article(const Article& x)
        : id(x.id), user(x.user), user_name(x.user_name), title(x.title), contents(x.contents), created_date(x.created_date)
    { }

    Article(uint32_t id, uint32_t user, const std::string& user_name, const std::string& title, const std::string& contents, const std::string& created_date)
        : id(id), user(user), user_name(user_name), title(title), contents(contents), created_date(created_date)
    { }
    Article(const fb::protocol::db::origin::Article& raw)
        : id(raw.id()), user(raw.user()), user_name(raw.user_name() != nullptr ? raw.user_name()->c_str() : ""), title(raw.title() != nullptr ? raw.title()->c_str() : ""), contents(raw.contents() != nullptr ? raw.contents()->c_str() : ""), created_date(raw.created_date() != nullptr ? raw.created_date()->c_str() : "")
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::origin::CreateArticle(builder,
            this->id,
            this->user,
            builder.CreateString(this->user_name),
            builder.CreateString(this->title),
            builder.CreateString(this->contents),
            builder.CreateString(this->created_date));
    }

};

 }  }  } 

#endif