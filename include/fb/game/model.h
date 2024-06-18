#ifndef __MODEL_H__
#define __MODEL_H__

#include <string>
#include <map>
#include <vector>
#include <any>
#include <optional>
#include <chrono>
#include <jsoncpp/json/json.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <unordered_map>

namespace fb { namespace model { 

static inline std::function<std::string(const std::string&)> string_encoding_func;

#pragma region base type
class date_range
{
public:
    const std::optional<boost::posix_time::ptime> begin, end;

public:
    date_range(const std::optional<boost::posix_time::ptime>& begin, const std::optional<boost::posix_time::ptime>& end) : 
        begin(begin),
        end(end)
    {}
    ~date_range() = default;
};
#pragma endregion

#pragma region enum
template <typename T>
inline T enum_parse(const std::string k)
{
    throw std::runtime_error("cannot parse enum value");
}

enum class BUNDLE_TYPE
{
    NONE = 0, 
    BUNDLE = 1, 
    PACKAGE = 2
};

template <>
inline BUNDLE_TYPE enum_parse<BUNDLE_TYPE>(const std::string k)
{
    static const std::unordered_map<std::string, BUNDLE_TYPE> enums
    {
        { "NONE", BUNDLE_TYPE::NONE }, 
        { "BUNDLE", BUNDLE_TYPE::BUNDLE }, 
        { "PACKAGE", BUNDLE_TYPE::PACKAGE }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class CLASS
{
    NONE = 0, 
    WARRIOR = 1, 
    THIEF = 2, 
    MAGICION = 3, 
    ASCETIC = 4
};

template <>
inline CLASS enum_parse<CLASS>(const std::string k)
{
    static const std::unordered_map<std::string, CLASS> enums
    {
        { "NONE", CLASS::NONE }, 
        { "WARRIOR", CLASS::WARRIOR }, 
        { "THIEF", CLASS::THIEF }, 
        { "MAGICION", CLASS::MAGICION }, 
        { "ASCETIC", CLASS::ASCETIC }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class DEATH_PENALTY
{
    NONE = 0, 
    DROP = 1, 
    DESTROY = 2
};

template <>
inline DEATH_PENALTY enum_parse<DEATH_PENALTY>(const std::string k)
{
    static const std::unordered_map<std::string, DEATH_PENALTY> enums
    {
        { "NONE", DEATH_PENALTY::NONE }, 
        { "DROP", DEATH_PENALTY::DROP }, 
        { "DESTROY", DEATH_PENALTY::DESTROY }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class DIRECTION
{
    LEFT = 1, 
    TOP = 2, 
    RIGHT = 3, 
    BOTTOM = 4
};

template <>
inline DIRECTION enum_parse<DIRECTION>(const std::string k)
{
    static const std::unordered_map<std::string, DIRECTION> enums
    {
        { "LEFT", DIRECTION::LEFT }, 
        { "TOP", DIRECTION::TOP }, 
        { "RIGHT", DIRECTION::RIGHT }, 
        { "BOTTOM", DIRECTION::BOTTOM }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class DSL
{
    item = 0
};

template <>
inline DSL enum_parse<DSL>(const std::string k)
{
    static const std::unordered_map<std::string, DSL> enums
    {
        { "item", DSL::item }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class ITEM_TYPE
{
    STUFF = 1, 
    CONSUME = 2, 
    WEAPON = 3, 
    ARMOR = 4, 
    HELMET = 5, 
    RING = 6, 
    SHIELD = 7, 
    AUXILIARY = 8, 
    BOW = 9
};

template <>
inline ITEM_TYPE enum_parse<ITEM_TYPE>(const std::string k)
{
    static const std::unordered_map<std::string, ITEM_TYPE> enums
    {
        { "STUFF", ITEM_TYPE::STUFF }, 
        { "CONSUME", ITEM_TYPE::CONSUME }, 
        { "WEAPON", ITEM_TYPE::WEAPON }, 
        { "ARMOR", ITEM_TYPE::ARMOR }, 
        { "HELMET", ITEM_TYPE::HELMET }, 
        { "RING", ITEM_TYPE::RING }, 
        { "SHIELD", ITEM_TYPE::SHIELD }, 
        { "AUXILIARY", ITEM_TYPE::AUXILIARY }, 
        { "BOW", ITEM_TYPE::BOW }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class MOB_ATTACK_TYPE
{
    NONE = 0, 
    COUNTER = 1, 
    CONTAINMENT = 2, 
    RUN_AWAY = 3, 
    NO_MOVE = 4
};

template <>
inline MOB_ATTACK_TYPE enum_parse<MOB_ATTACK_TYPE>(const std::string k)
{
    static const std::unordered_map<std::string, MOB_ATTACK_TYPE> enums
    {
        { "NONE", MOB_ATTACK_TYPE::NONE }, 
        { "COUNTER", MOB_ATTACK_TYPE::COUNTER }, 
        { "CONTAINMENT", MOB_ATTACK_TYPE::CONTAINMENT }, 
        { "RUN_AWAY", MOB_ATTACK_TYPE::RUN_AWAY }, 
        { "NO_MOVE", MOB_ATTACK_TYPE::NO_MOVE }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class MOB_SIZE
{
    SMALL = 1, 
    LARGE = 2, 
    ALL = SMALL | LARGE
};

template <>
inline MOB_SIZE enum_parse<MOB_SIZE>(const std::string k)
{
    static const std::unordered_map<std::string, MOB_SIZE> enums
    {
        { "SMALL", MOB_SIZE::SMALL }, 
        { "LARGE", MOB_SIZE::LARGE }, 
        { "ALL", MOB_SIZE::ALL }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class SEX
{
    MAN = 1, 
    WOMAN = 2, 
    ALL = MAN | WOMAN
};

template <>
inline SEX enum_parse<SEX>(const std::string k)
{
    static const std::unordered_map<std::string, SEX> enums
    {
        { "MAN", SEX::MAN }, 
        { "WOMAN", SEX::WOMAN }, 
        { "ALL", SEX::ALL }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

#pragma endregion

#pragma region const
namespace consts { 


}
#pragma endregion

template<class T>
struct is_optional : std::false_type {};

template<class T>
struct is_optional<std::optional<T>> : std::true_type {};

template<class T>
struct is_vector : std::false_type {};

template<class T>
struct is_vector<std::vector<T>> : std::true_type {};

template <typename T, typename Enabler = void>
struct is_map : std::false_type {};

template <typename T>
struct is_map<T, std::void_t<typename T::mapped_type>> : std::true_type{};

template <typename T>
struct is_default
{
    static constexpr bool value =
        !std::is_enum<T>::value &&
        !fb::model::is_vector<T>::value &&
        !fb::model::is_map<T>::value &&
        !fb::model::is_optional<T>::value;
};

class dsl;

template <typename T> static typename std::enable_if<fb::model::is_default<T>::value, T>::type build(const Json::Value& json);
template <typename T> static typename std::enable_if<std::is_enum<T>::value, T>::type build(const Json::Value& json);
template <typename T> static typename std::enable_if<fb::model::is_vector<T>::value, T>::type build(const Json::Value& json);
template <typename T> static typename std::enable_if<fb::model::is_map<T>::value, T>::type build(const Json::Value& json);
template <typename T> static typename std::enable_if<fb::model::is_optional<T>::value, T>::type build(const Json::Value& json);
template <> static int8_t build<int8_t>(const Json::Value& json);
template <> static uint8_t build<uint8_t>(const Json::Value& json);
template <> static int16_t build<int16_t>(const Json::Value& json);
template <> static uint16_t build<uint16_t>(const Json::Value& json);
template <> static int build<int>(const Json::Value& json);
template <> static uint32_t build<uint32_t>(const Json::Value& json);
template <> static int64_t build<int64_t>(const Json::Value& json);
template <> static uint64_t build<uint64_t>(const Json::Value& json);
template <> static std::string build<std::string>(const Json::Value& json);
template <> static float build<float>(const Json::Value& json);
template <> static double build<double>(const Json::Value& json);
template <> static bool build<bool>(const Json::Value& json);
template <> static boost::posix_time::ptime build<boost::posix_time::ptime>(const Json::Value& json);
template <> static std::chrono::milliseconds build<std::chrono::milliseconds>(const Json::Value& json);
template <> static fb::model::date_range build<fb::model::date_range>(const Json::Value& json);
template <> static fb::model::dsl build<fb::model::dsl>(const Json::Value& json);

#pragma region dsl
class dsl
{
public:
    class item;

public:
    fb::model::DSL header;
    const std::vector<std::any> params;

private:
    static std::vector<std::any> parse_params(const Json::Value& json);

public:
    dsl(fb::model::DSL header, const std::vector<std::any>& params) : header(header), params(params)
    { }
    dsl(const Json::Value& json) : header(build<fb::model::DSL>(json["Type"])), params(parse_params(json))
    { }
    ~dsl()
    { }
};

class fb::model::dsl::item
{
public:
    const uint32_t id;
    const uint32_t count;
    const double percent;

public:
    item(uint32_t id, uint32_t count, double percent) : 
        id(id),
        count(count),
        percent(percent)
    { }
    item(const Json::Value& json) : 
        id(fb::model::build<uint32_t>(json[0])),
        count(fb::model::build<uint32_t>(json[1])),
        percent(fb::model::build<double>(json[2]))
    { }
    item(const std::vector<std::any>& parameters) : 
        id(any_cast<uint32_t>(parameters[0])),
        count(any_cast<uint32_t>(parameters[1])),
        percent(any_cast<double>(parameters[2]))
    { }

public:
    fb::model::dsl to_dsl()
    {
        return fb::model::dsl(fb::model::DSL::item, {id, count, percent});
    }
};


inline std::vector<std::any> fb::model::dsl::parse_params(const Json::Value& json)
{
    static auto data = std::unordered_map<fb::model::DSL, std::function<std::vector<std::any>(const Json::Value&)>>
    {
        {fb::model::DSL::item, [](const Json::Value& json) { return fb::model::dsl::item(json).to_dsl().params; }}
    };

    auto header = build<fb::model::DSL>(json["Type"]);
    auto i = data.find(header);
    if(i == data.end())
        throw std::runtime_error("invalid dsl header");

    return i->second(json["Parameters"]);
}
#pragma endregion

#pragma region data model
class ability
{
public:
    const fb::model::CLASS parent;
    const uint8_t level;
    const uint8_t dexteritry;
    const uint8_t intelligence;
    const uint8_t strength;
    const uint32_t exp;
    const uint32_t hp;
    const uint32_t mp;

public:
    ability(const Json::Value& json) : 
        parent(fb::model::build<fb::model::CLASS>(json["parent"])),
        level(fb::model::build<uint8_t>(json["level"])),
        dexteritry(fb::model::build<uint8_t>(json["dexteritry"])),
        intelligence(fb::model::build<uint8_t>(json["intelligence"])),
        strength(fb::model::build<uint8_t>(json["strength"])),
        exp(fb::model::build<uint32_t>(json["exp"])),
        hp(fb::model::build<uint32_t>(json["hp"])),
        mp(fb::model::build<uint32_t>(json["mp"]))
    { }
    ~ability()
    { }
};
class ability_attribute
{
public:
    const fb::model::CLASS id;

public:
    ability_attribute(const Json::Value& json) : 
        id(fb::model::build<fb::model::CLASS>(json["id"]))
    { }
    ~ability_attribute()
    { }
};
class board
{
public:
    const uint32_t id;
    const std::string name;
    const uint8_t min_level;
    const uint8_t max_level;
    const bool admin;

public:
    board(const Json::Value& json) : 
        id(fb::model::build<uint32_t>(json["id"])),
        name(fb::model::build<std::string>(json["name"])),
        min_level(fb::model::build<uint8_t>(json["min_level"])),
        max_level(fb::model::build<uint8_t>(json["max_level"])),
        admin(fb::model::build<bool>(json["admin"]))
    { }
    ~board()
    { }
};
class buy
{
public:
    const uint32_t parent;
    const uint32_t item;
    const std::optional<uint32_t> price;

public:
    buy(const Json::Value& json) : 
        parent(fb::model::build<uint32_t>(json["parent"])),
        item(fb::model::build<uint32_t>(json["item"])),
        price(fb::model::build<std::optional<uint32_t>>(json["price"]))
    { }
    ~buy()
    { }
};
class buy_attribute
{
public:
    const uint32_t id;

public:
    buy_attribute(const Json::Value& json) : 
        id(fb::model::build<uint32_t>(json["id"]))
    { }
    ~buy_attribute()
    { }
};
class combine
{
public:
    const std::vector<fb::model::dsl> source;
    const std::vector<fb::model::dsl> success;
    const std::vector<fb::model::dsl> failed;
    const double percent;

public:
    combine(const Json::Value& json) : 
        source(fb::model::build<std::vector<fb::model::dsl>>(json["source"])),
        success(fb::model::build<std::vector<fb::model::dsl>>(json["success"])),
        failed(fb::model::build<std::vector<fb::model::dsl>>(json["failed"])),
        percent(fb::model::build<double>(json["percent"]))
    { }
    ~combine()
    { }
};
class door
{
public:
    const uint32_t id;
    const std::vector<uint32_t> pairs;

public:
    door(const Json::Value& json) : 
        id(fb::model::build<uint32_t>(json["id"])),
        pairs(fb::model::build<std::vector<uint32_t>>(json["pairs"]))
    { }
    ~door()
    { }
};
class door_pair
{
public:
    const uint32_t id;
    const uint32_t open;
    const uint32_t close;

public:
    door_pair(const Json::Value& json) : 
        id(fb::model::build<uint32_t>(json["id"])),
        open(fb::model::build<uint32_t>(json["open"])),
        close(fb::model::build<uint32_t>(json["close"]))
    { }
    ~door_pair()
    { }
};
class drop
{
public:
    const std::string id;
    const std::vector<fb::model::dsl> dsl;

public:
    drop(const Json::Value& json) : 
        id(fb::model::build<std::string>(json["id"])),
        dsl(fb::model::build<std::vector<fb::model::dsl>>(json["dsl"]))
    { }
    ~drop()
    { }
};
class equipment_option
{
public:
    const std::string id;
    const uint16_t look;
    const uint32_t durability;
    const std::optional<float> repair;
    const uint32_t rename;
    const int defensive_physical;
    const int defensive_magical;
    const uint32_t damage_s_min;
    const uint32_t damage_s_max;
    const uint32_t damage_l_min;
    const uint32_t damage_l_max;
    const uint8_t healing_cycle;
    const int8_t intelligence;
    const uint32_t mp;
    const int hit;
    const int damage;
    const int hp;
    const uint16_t sound;
    const int8_t strength;
    const int8_t dexterity;
    const double hp_percent;
    const double mp_percent;
    const std::optional<uint32_t> spell;

public:
    equipment_option(const Json::Value& json) : 
        id(fb::model::build<std::string>(json["id"])),
        look(fb::model::build<uint16_t>(json["look"])),
        durability(fb::model::build<uint32_t>(json["durability"])),
        repair(fb::model::build<std::optional<float>>(json["repair"])),
        rename(fb::model::build<uint32_t>(json["rename"])),
        defensive_physical(fb::model::build<int>(json["defensive_physical"])),
        defensive_magical(fb::model::build<int>(json["defensive_magical"])),
        damage_s_min(fb::model::build<uint32_t>(json["damage_s_min"])),
        damage_s_max(fb::model::build<uint32_t>(json["damage_s_max"])),
        damage_l_min(fb::model::build<uint32_t>(json["damage_l_min"])),
        damage_l_max(fb::model::build<uint32_t>(json["damage_l_max"])),
        healing_cycle(fb::model::build<uint8_t>(json["healing_cycle"])),
        intelligence(fb::model::build<int8_t>(json["intelligence"])),
        mp(fb::model::build<uint32_t>(json["mp"])),
        hit(fb::model::build<int>(json["hit"])),
        damage(fb::model::build<int>(json["damage"])),
        hp(fb::model::build<int>(json["hp"])),
        sound(fb::model::build<uint16_t>(json["sound"])),
        strength(fb::model::build<int8_t>(json["strength"])),
        dexterity(fb::model::build<int8_t>(json["dexterity"])),
        hp_percent(fb::model::build<double>(json["hp_percent"])),
        mp_percent(fb::model::build<double>(json["mp_percent"])),
        spell(fb::model::build<std::optional<uint32_t>>(json["spell"]))
    { }
    ~equipment_option()
    { }
};
class item
{
public:
    const uint32_t id;
    const std::string name;
    const uint16_t icon;
    const uint8_t color;
    const uint32_t price;
    const std::optional<uint32_t> deposit_price;
    const bool trade;
    const fb::model::ITEM_TYPE type;
    const fb::model::BUNDLE_TYPE BUNDLE;
    const std::string desc;
    const fb::model::DEATH_PENALTY death_penalty;
    const uint16_t capacity;
    const std::string tooltip;
    const std::string script_active;
    const std::string script_dress;
    const std::string script_undress;
    const std::string condition;
    const std::string equipment_option;

public:
    item(const Json::Value& json) : 
        id(fb::model::build<uint32_t>(json["id"])),
        name(fb::model::build<std::string>(json["name"])),
        icon(fb::model::build<uint16_t>(json["icon"])),
        color(fb::model::build<uint8_t>(json["color"])),
        price(fb::model::build<uint32_t>(json["price"])),
        deposit_price(fb::model::build<std::optional<uint32_t>>(json["deposit_price"])),
        trade(fb::model::build<bool>(json["trade"])),
        type(fb::model::build<fb::model::ITEM_TYPE>(json["type"])),
        BUNDLE(fb::model::build<fb::model::BUNDLE_TYPE>(json["BUNDLE"])),
        desc(fb::model::build<std::string>(json["desc"])),
        death_penalty(fb::model::build<fb::model::DEATH_PENALTY>(json["death_penalty"])),
        capacity(fb::model::build<uint16_t>(json["capacity"])),
        tooltip(fb::model::build<std::string>(json["tooltip"])),
        script_active(fb::model::build<std::string>(json["script_active"])),
        script_dress(fb::model::build<std::string>(json["script_dress"])),
        script_undress(fb::model::build<std::string>(json["script_undress"])),
        condition(fb::model::build<std::string>(json["condition"])),
        equipment_option(fb::model::build<std::string>(json["equipment_option"]))
    { }
    ~item()
    { }
};
class item_condition
{
public:
    const std::string id;
    const fb::model::SEX sex;
    const uint8_t strength;
    const uint8_t dexterity;
    const uint8_t intelligence;
    const uint8_t level;
    const std::optional<fb::model::CLASS> class_id;
    const uint8_t promotion;

public:
    item_condition(const Json::Value& json) : 
        id(fb::model::build<std::string>(json["id"])),
        sex(fb::model::build<fb::model::SEX>(json["sex"])),
        strength(fb::model::build<uint8_t>(json["strength"])),
        dexterity(fb::model::build<uint8_t>(json["dexterity"])),
        intelligence(fb::model::build<uint8_t>(json["intelligence"])),
        level(fb::model::build<uint8_t>(json["level"])),
        class_id(fb::model::build<std::optional<fb::model::CLASS>>(json["class_id"])),
        promotion(fb::model::build<uint8_t>(json["promotion"]))
    { }
    ~item_condition()
    { }
};
class map
{
public:
    const uint32_t id;
    const std::string name;
    const uint32_t root;
    const uint16_t bgm;
    const uint8_t host;
    const bool hunting_ground;
    const bool enable_die_penalty;
    const bool enable_talk;
    const bool building;
    const bool enable_whisper;
    const bool enable_pk;
    const bool enable_spell;

public:
    map(const Json::Value& json) : 
        id(fb::model::build<uint32_t>(json["id"])),
        name(fb::model::build<std::string>(json["name"])),
        root(fb::model::build<uint32_t>(json["root"])),
        bgm(fb::model::build<uint16_t>(json["bgm"])),
        host(fb::model::build<uint8_t>(json["host"])),
        hunting_ground(fb::model::build<bool>(json["hunting_ground"])),
        enable_die_penalty(fb::model::build<bool>(json["enable_die_penalty"])),
        enable_talk(fb::model::build<bool>(json["enable_talk"])),
        building(fb::model::build<bool>(json["building"])),
        enable_whisper(fb::model::build<bool>(json["enable_whisper"])),
        enable_pk(fb::model::build<bool>(json["enable_pk"])),
        enable_spell(fb::model::build<bool>(json["enable_spell"]))
    { }
    ~map()
    { }
};
class mob
{
public:
    const uint32_t id;
    const std::string name;
    const int look;
    const int color;
    const uint32_t hp;
    const uint32_t mp;
    const fb::model::MOB_SIZE size;
    const uint32_t exp;
    const int defensive_physical;
    const int defensive_magical;
    const fb::model::MOB_ATTACK_TYPE attack_type;
    const uint32_t damage_min;
    const uint32_t damage_max;
    const uint32_t speed;
    const std::string drop;
    const std::string attack_script;
    const std::string die_script;

public:
    mob(const Json::Value& json) : 
        id(fb::model::build<uint32_t>(json["id"])),
        name(fb::model::build<std::string>(json["name"])),
        look(fb::model::build<int>(json["look"])),
        color(fb::model::build<int>(json["color"])),
        hp(fb::model::build<uint32_t>(json["hp"])),
        mp(fb::model::build<uint32_t>(json["mp"])),
        size(fb::model::build<fb::model::MOB_SIZE>(json["size"])),
        exp(fb::model::build<uint32_t>(json["exp"])),
        defensive_physical(fb::model::build<int>(json["defensive_physical"])),
        defensive_magical(fb::model::build<int>(json["defensive_magical"])),
        attack_type(fb::model::build<fb::model::MOB_ATTACK_TYPE>(json["attack_type"])),
        damage_min(fb::model::build<uint32_t>(json["damage_min"])),
        damage_max(fb::model::build<uint32_t>(json["damage_max"])),
        speed(fb::model::build<uint32_t>(json["speed"])),
        drop(fb::model::build<std::string>(json["drop"])),
        attack_script(fb::model::build<std::string>(json["attack_script"])),
        die_script(fb::model::build<std::string>(json["die_script"]))
    { }
    ~mob()
    { }
};
class mob_spawn
{
public:
    const uint32_t parent;
    const uint32_t x0;
    const uint32_t x1;
    const uint32_t y0;
    const uint32_t y1;
    const uint32_t count;
    const uint32_t mob;
    const std::chrono::milliseconds rezen;

public:
    mob_spawn(const Json::Value& json) : 
        parent(fb::model::build<uint32_t>(json["parent"])),
        x0(fb::model::build<uint32_t>(json["x0"])),
        x1(fb::model::build<uint32_t>(json["x1"])),
        y0(fb::model::build<uint32_t>(json["y0"])),
        y1(fb::model::build<uint32_t>(json["y1"])),
        count(fb::model::build<uint32_t>(json["count"])),
        mob(fb::model::build<uint32_t>(json["mob"])),
        rezen(fb::model::build<std::chrono::milliseconds>(json["rezen"]))
    { }
    ~mob_spawn()
    { }
};
class mob_spawn_attribute
{
public:
    const uint32_t id;

public:
    mob_spawn_attribute(const Json::Value& json) : 
        id(fb::model::build<uint32_t>(json["id"]))
    { }
    ~mob_spawn_attribute()
    { }
};
class npc
{
public:
    const uint32_t id;
    const std::string name;
    const int look;
    const int color;
    const std::string script;
    const std::vector<uint32_t> sell;
    const std::vector<uint32_t> buy;
    const bool repair;
    const bool hold_money;
    const bool hold_item;
    const bool rename;

public:
    npc(const Json::Value& json) : 
        id(fb::model::build<uint32_t>(json["id"])),
        name(fb::model::build<std::string>(json["name"])),
        look(fb::model::build<int>(json["look"])),
        color(fb::model::build<int>(json["color"])),
        script(fb::model::build<std::string>(json["script"])),
        sell(fb::model::build<std::vector<uint32_t>>(json["sell"])),
        buy(fb::model::build<std::vector<uint32_t>>(json["buy"])),
        repair(fb::model::build<bool>(json["repair"])),
        hold_money(fb::model::build<bool>(json["hold_money"])),
        hold_item(fb::model::build<bool>(json["hold_item"])),
        rename(fb::model::build<bool>(json["rename"]))
    { }
    ~npc()
    { }
};
class npc_spawn
{
public:
    const uint32_t parent;
    const uint32_t npc;
    const uint32_t x;
    const uint32_t y;
    const fb::model::DIRECTION direction;

public:
    npc_spawn(const Json::Value& json) : 
        parent(fb::model::build<uint32_t>(json["parent"])),
        npc(fb::model::build<uint32_t>(json["npc"])),
        x(fb::model::build<uint32_t>(json["x"])),
        y(fb::model::build<uint32_t>(json["y"])),
        direction(fb::model::build<fb::model::DIRECTION>(json["direction"]))
    { }
    ~npc_spawn()
    { }
};
class npc_spawn_attribute
{
public:
    const uint32_t map;

public:
    npc_spawn_attribute(const Json::Value& json) : 
        map(fb::model::build<uint32_t>(json["map"]))
    { }
    ~npc_spawn_attribute()
    { }
};
class promotion
{
public:
    const fb::model::CLASS parent;
    const uint8_t step;

public:
    promotion(const Json::Value& json) : 
        parent(fb::model::build<fb::model::CLASS>(json["parent"])),
        step(fb::model::build<uint8_t>(json["step"]))
    { }
    ~promotion()
    { }
};
class promotion_attribute
{
public:
    const fb::model::CLASS id;

public:
    promotion_attribute(const Json::Value& json) : 
        id(fb::model::build<fb::model::CLASS>(json["id"]))
    { }
    ~promotion_attribute()
    { }
};
class reward
{
public:
    const std::string id;
    const std::vector<fb::model::dsl> dsl;

public:
    reward(const Json::Value& json) : 
        id(fb::model::build<std::string>(json["id"])),
        dsl(fb::model::build<std::vector<fb::model::dsl>>(json["dsl"]))
    { }
    ~reward()
    { }
};
class sell
{
public:
    const uint32_t parent;
    const uint32_t item;
    const std::optional<uint32_t> price;

public:
    sell(const Json::Value& json) : 
        parent(fb::model::build<uint32_t>(json["parent"])),
        item(fb::model::build<uint32_t>(json["item"])),
        price(fb::model::build<std::optional<uint32_t>>(json["price"]))
    { }
    ~sell()
    { }
};
class sell_attribute
{
public:
    const uint32_t id;
    const std::string group;

public:
    sell_attribute(const Json::Value& json) : 
        id(fb::model::build<uint32_t>(json["id"])),
        group(fb::model::build<std::string>(json["group"]))
    { }
    ~sell_attribute()
    { }
};
class spell
{
public:
    const uint32_t id;
    const std::string name;
    const uint8_t type;
    const std::string cast;
    const std::string uncast;
    const std::string concast;
    const std::string message;

public:
    spell(const Json::Value& json) : 
        id(fb::model::build<uint32_t>(json["id"])),
        name(fb::model::build<std::string>(json["name"])),
        type(fb::model::build<uint8_t>(json["type"])),
        cast(fb::model::build<std::string>(json["cast"])),
        uncast(fb::model::build<std::string>(json["uncast"])),
        concast(fb::model::build<std::string>(json["concast"])),
        message(fb::model::build<std::string>(json["message"]))
    { }
    ~spell()
    { }
};

#pragma endregion

#pragma region container
template <typename K, typename V>
class kv_container : private std::unordered_map<K, V>
{
private:
    const std::string _fname;

public:
    using std::unordered_map<K,V>::size;
    using std::unordered_map<K,V>::begin;
    using std::unordered_map<K,V>::end;
    using std::unordered_map<K,V>::cbegin;
    using std::unordered_map<K,V>::cend;

protected:
    kv_container(const std::string& fname) : _fname(fname)
    { }

public:
    kv_container(const Json::Value& json)
    {
        this->load(json);
    }
    ~kv_container() = default;

private:
    void load(const std::string& fname)
    {
        Json::Value             json;
        std::ifstream           ifstream;
        ifstream.open(fname);
        if (ifstream.is_open() == false)
            throw std::runtime_error("cannot find json file");

        Json::Reader            reader;
        if (reader.parse(ifstream, json) == false)
        {
            ifstream.close();
            throw std::runtime_error("cannot parse json file");
        }

        ifstream.close();
        this->load(json);
    }
    void load(const Json::Value& json)
    {
        for (auto i = json.begin(); i != json.end(); i++)
        {
            std::unordered_map<K, V>::insert({ fb::model::build<K>(i.key()), fb::model::build<V>(*i) });
        }
    }

public:
    void load()
    {
        this->load(this->_fname);
    }

    const V* find(const K& k) const
    {
        auto i = std::unordered_map<K, V>::find(k);
        if (i == std::unordered_map<K, V>::cend())
            return nullptr;

        return &i->second;
    }

    const V& operator [] (const K& k) const 
    {
        auto found = this->find(k);
        if(found == nullptr)
            throw std::runtime_error("does not exists");

        return *found;
    }
};

template <typename T>
class array_container : private std::vector<T>
{
private:
    const std::string _fname;

public:
    using std::vector<T>::size;
    using std::vector<T>::begin;
    using std::vector<T>::end;
    using std::vector<T>::cbegin;
    using std::vector<T>::cend;
    using std::vector<T>::at;

protected:
    array_container(const std::string& fname) : _fname(fname)
    { }

public:
    array_container(const Json::Value& json)
    {
        this->load(json);
    }
    ~array_container() = default;

private:
    void load(const std::string& fname)
    {
        Json::Value             json;
        std::ifstream           ifstream;
        ifstream.open(fname);
        if (ifstream.is_open() == false)
            throw std::runtime_error("cannot find json file");

        Json::Reader            reader;
        if (reader.parse(ifstream, json) == false)
        {
            ifstream.close();
            throw std::runtime_error("cannot parse json file");
        }

        ifstream.close();
        this->load(json);
    }
    void load(const Json::Value& json)
    {
        for (auto i = json.begin(); i != json.end(); i++)
        {
            std::vector<T>::push_back(fb::model::build<T>(*i));
        }
    }

public:
    void load()
    {
        this->load(this->_fname);
    }

    const T* find(uint32_t i) const
    {
        if (i > std::vector<T>::size() - 1)
            return nullptr;

        return &std::vector<T>::at(i);
    }

    const T& operator [] (uint32_t i) const
    {
        auto found = this->find(i);
        if (found == nullptr)
            throw std::out_of_range("out of range exception");

        return *found;
    }
};

class __ability : public fb::model::kv_container<fb::model::CLASS, fb::model::kv_container<uint8_t, fb::model::ability>>
{
public:
    __ability() : fb::model::kv_container<fb::model::CLASS, fb::model::kv_container<uint8_t, fb::model::ability>>(std::string("json/ability.json"))
    { }
    ~__ability()
    { }
};

class __ability_attribute : public fb::model::kv_container<fb::model::CLASS, fb::model::ability_attribute>
{
public:
    __ability_attribute() : fb::model::kv_container<fb::model::CLASS, fb::model::ability_attribute>(std::string("json/ability_attribute.json"))
    { }
    ~__ability_attribute()
    { }
};

class __board : public fb::model::kv_container<uint32_t, fb::model::board>
{
public:
    __board() : fb::model::kv_container<uint32_t, fb::model::board>(std::string("json/board.json"))
    { }
    ~__board()
    { }
};

class __buy : public fb::model::kv_container<uint32_t, fb::model::kv_container<uint32_t, fb::model::buy>>
{
public:
    __buy() : fb::model::kv_container<uint32_t, fb::model::kv_container<uint32_t, fb::model::buy>>(std::string("json/buy.json"))
    { }
    ~__buy()
    { }
};

class __buy_attribute : public fb::model::kv_container<uint32_t, fb::model::buy_attribute>
{
public:
    __buy_attribute() : fb::model::kv_container<uint32_t, fb::model::buy_attribute>(std::string("json/buy_attribute.json"))
    { }
    ~__buy_attribute()
    { }
};

class __combine : public fb::model::array_container<fb::model::combine>
{
public:
    __combine() : fb::model::array_container<fb::model::combine>(std::string("json/combine.json"))
    { }
    ~__combine()
    { }
};

class __door : public fb::model::kv_container<uint32_t, fb::model::door>
{
public:
    __door() : fb::model::kv_container<uint32_t, fb::model::door>(std::string("json/door.json"))
    { }
    ~__door()
    { }
};

class __door_pair : public fb::model::kv_container<uint32_t, fb::model::door_pair>
{
public:
    __door_pair() : fb::model::kv_container<uint32_t, fb::model::door_pair>(std::string("json/door_pair.json"))
    { }
    ~__door_pair()
    { }
};

class __drop : public fb::model::kv_container<std::string, fb::model::drop>
{
public:
    __drop() : fb::model::kv_container<std::string, fb::model::drop>(std::string("json/drop.json"))
    { }
    ~__drop()
    { }
};

class __equipment_option : public fb::model::kv_container<std::string, fb::model::equipment_option>
{
public:
    __equipment_option() : fb::model::kv_container<std::string, fb::model::equipment_option>(std::string("json/equipment_option.json"))
    { }
    ~__equipment_option()
    { }
};

class __item : public fb::model::kv_container<uint32_t, fb::model::item>
{
public:
    __item() : fb::model::kv_container<uint32_t, fb::model::item>(std::string("json/item.json"))
    { }
    ~__item()
    { }
};

class __item_condition : public fb::model::kv_container<std::string, fb::model::item_condition>
{
public:
    __item_condition() : fb::model::kv_container<std::string, fb::model::item_condition>(std::string("json/item_condition.json"))
    { }
    ~__item_condition()
    { }
};

class __map : public fb::model::kv_container<uint32_t, fb::model::map>
{
public:
    __map() : fb::model::kv_container<uint32_t, fb::model::map>(std::string("json/map.json"))
    { }
    ~__map()
    { }
};

class __mob : public fb::model::kv_container<uint32_t, fb::model::mob>
{
public:
    __mob() : fb::model::kv_container<uint32_t, fb::model::mob>(std::string("json/mob.json"))
    { }
    ~__mob()
    { }
};

class __mob_spawn : public fb::model::kv_container<uint32_t, fb::model::array_container<fb::model::mob_spawn>>
{
public:
    __mob_spawn() : fb::model::kv_container<uint32_t, fb::model::array_container<fb::model::mob_spawn>>(std::string("json/mob_spawn.json"))
    { }
    ~__mob_spawn()
    { }
};

class __mob_spawn_attribute : public fb::model::kv_container<uint32_t, fb::model::mob_spawn_attribute>
{
public:
    __mob_spawn_attribute() : fb::model::kv_container<uint32_t, fb::model::mob_spawn_attribute>(std::string("json/mob_spawn_attribute.json"))
    { }
    ~__mob_spawn_attribute()
    { }
};

class __npc : public fb::model::kv_container<uint32_t, fb::model::npc>
{
public:
    __npc() : fb::model::kv_container<uint32_t, fb::model::npc>(std::string("json/npc.json"))
    { }
    ~__npc()
    { }
};

class __npc_spawn : public fb::model::kv_container<uint32_t, fb::model::array_container<fb::model::npc_spawn>>
{
public:
    __npc_spawn() : fb::model::kv_container<uint32_t, fb::model::array_container<fb::model::npc_spawn>>(std::string("json/npc_spawn.json"))
    { }
    ~__npc_spawn()
    { }
};

class __npc_spawn_attribute : public fb::model::kv_container<uint32_t, fb::model::npc_spawn_attribute>
{
public:
    __npc_spawn_attribute() : fb::model::kv_container<uint32_t, fb::model::npc_spawn_attribute>(std::string("json/npc_spawn_attribute.json"))
    { }
    ~__npc_spawn_attribute()
    { }
};

class __promotion : public fb::model::kv_container<fb::model::CLASS, fb::model::kv_container<uint8_t, fb::model::promotion>>
{
public:
    __promotion() : fb::model::kv_container<fb::model::CLASS, fb::model::kv_container<uint8_t, fb::model::promotion>>(std::string("json/promotion.json"))
    { }
    ~__promotion()
    { }
};

class __promotion_attribute : public fb::model::kv_container<fb::model::CLASS, fb::model::promotion_attribute>
{
public:
    __promotion_attribute() : fb::model::kv_container<fb::model::CLASS, fb::model::promotion_attribute>(std::string("json/promotion_attribute.json"))
    { }
    ~__promotion_attribute()
    { }
};

class __reward : public fb::model::kv_container<std::string, fb::model::reward>
{
public:
    __reward() : fb::model::kv_container<std::string, fb::model::reward>(std::string("json/reward.json"))
    { }
    ~__reward()
    { }
};

class __sell : public fb::model::kv_container<uint32_t, fb::model::kv_container<uint32_t, fb::model::sell>>
{
public:
    __sell() : fb::model::kv_container<uint32_t, fb::model::kv_container<uint32_t, fb::model::sell>>(std::string("json/sell.json"))
    { }
    ~__sell()
    { }
};

class __sell_attribute : public fb::model::kv_container<uint32_t, fb::model::sell_attribute>
{
public:
    __sell_attribute() : fb::model::kv_container<uint32_t, fb::model::sell_attribute>(std::string("json/sell_attribute.json"))
    { }
    ~__sell_attribute()
    { }
};

class __spell : public fb::model::kv_container<uint32_t, fb::model::spell>
{
public:
    __spell() : fb::model::kv_container<uint32_t, fb::model::spell>(std::string("json/spell.json"))
    { }
    ~__spell()
    { }
};


class container
{
public:
    fb::model::__ability ability;
    fb::model::__ability_attribute ability_attribute;
    fb::model::__board board;
    fb::model::__buy buy;
    fb::model::__buy_attribute buy_attribute;
    fb::model::__combine combine;
    fb::model::__door door;
    fb::model::__door_pair door_pair;
    fb::model::__drop drop;
    fb::model::__equipment_option equipment_option;
    fb::model::__item item;
    fb::model::__item_condition item_condition;
    fb::model::__map map;
    fb::model::__mob mob;
    fb::model::__mob_spawn mob_spawn;
    fb::model::__mob_spawn_attribute mob_spawn_attribute;
    fb::model::__npc npc;
    fb::model::__npc_spawn npc_spawn;
    fb::model::__npc_spawn_attribute npc_spawn_attribute;
    fb::model::__promotion promotion;
    fb::model::__promotion_attribute promotion_attribute;
    fb::model::__reward reward;
    fb::model::__sell sell;
    fb::model::__sell_attribute sell_attribute;
    fb::model::__spell spell;

public:
    void load(const std::function<void(float)>& callback)
    {
        std::queue<std::function<void()>> queue;
#pragma region enqueue
        queue.push([this]() { this->ability.load(); });
        queue.push([this]() { this->ability_attribute.load(); });
        queue.push([this]() { this->board.load(); });
        queue.push([this]() { this->buy.load(); });
        queue.push([this]() { this->buy_attribute.load(); });
        queue.push([this]() { this->combine.load(); });
        queue.push([this]() { this->door.load(); });
        queue.push([this]() { this->door_pair.load(); });
        queue.push([this]() { this->drop.load(); });
        queue.push([this]() { this->equipment_option.load(); });
        queue.push([this]() { this->item.load(); });
        queue.push([this]() { this->item_condition.load(); });
        queue.push([this]() { this->map.load(); });
        queue.push([this]() { this->mob.load(); });
        queue.push([this]() { this->mob_spawn.load(); });
        queue.push([this]() { this->mob_spawn_attribute.load(); });
        queue.push([this]() { this->npc.load(); });
        queue.push([this]() { this->npc_spawn.load(); });
        queue.push([this]() { this->npc_spawn_attribute.load(); });
        queue.push([this]() { this->promotion.load(); });
        queue.push([this]() { this->promotion_attribute.load(); });
        queue.push([this]() { this->reward.load(); });
        queue.push([this]() { this->sell.load(); });
        queue.push([this]() { this->sell_attribute.load(); });
        queue.push([this]() { this->spell.load(); });
#pragma endregion

        auto size = queue.size();
        auto process = 0;
        auto mutex = std::mutex();
        auto fn = [&]() 
        {
            while (true)
            {
                std::function<void()> job;
                {
                    auto _ = std::lock_guard(mutex);
                    if(queue.empty())
                        break;

                    job = queue.front();
                    queue.pop();
                }
                
                job();

                {
                    auto _ = std::lock_guard(mutex);
                    auto percent = (++process * 100) / float(size);
                    callback(percent);
                }
            }
        };

        auto tasks = std::queue<std::future<void>>();
        for(int i = 0; i < std::thread::hardware_concurrency(); i++)
        {
            tasks.push(std::async(std::launch::async, fn));
        }
    }
};
#pragma endregion

#pragma region build function
template <typename T> typename std::enable_if<is_default<T>::value, T>::type build(const Json::Value& json)
{
    return T(json);
}

template <typename T> typename std::enable_if<std::is_enum<T>::value, T>::type build(const Json::Value& json)
{
    if(json.isString())
        return T(enum_parse<T>(build<std::string>(json)));
    else
        return T(json.asInt());
}

template <typename T> typename std::enable_if<is_vector<T>::value, T>::type build(const Json::Value& json)
{
    auto result = std::vector<typename T::value_type>();
    for (auto i = json.begin(); i != json.end(); i++)
    {
        result.push_back(build<typename T::value_type>(*i));
    }
    return result;
}

template <typename T> typename std::enable_if<is_map<T>::value, T>::type build(const Json::Value& json)
{
    return std::map<typename T::key_type, typename T::mapped_type>();
}

template <typename T> typename std::enable_if<is_optional<T>::value, T>::type build(const Json::Value& json)
{
    if (json.isNull())
        return std::nullopt;
    else
        return std::optional<typename T::value_type>(build<typename T::value_type>(json));
}

template <> int8_t build<int8_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(build<std::string>(json).c_str()) : json.asInt();
    return static_cast<int8_t>(result);
}

template <> uint8_t build<uint8_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(build<std::string>(json).c_str()) : json.asUInt();
    return static_cast<uint8_t>(result);
}

template <> int16_t build<int16_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(build<std::string>(json).c_str()) : json.asInt();
    return static_cast<int16_t>(result);
}

template <> uint16_t build<uint16_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(build<std::string>(json).c_str()) : json.asUInt();
    return static_cast<uint16_t>(result);
}

template <> int build<int>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(build<std::string>(json).c_str()) : json.asInt();
    return static_cast<int>(result);
}

template <> uint32_t build<uint32_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(build<std::string>(json).c_str()) : json.asUInt();
    return static_cast<uint32_t>(result);
}

template <> int64_t build<int64_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(build<std::string>(json).c_str()) : json.asInt64();
    return static_cast<int64_t>(result);
}

template <> uint64_t build<uint64_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(build<std::string>(json).c_str()) : json.asUInt64();
    return static_cast<uint64_t>(result);
}

template <> std::string build<std::string>(const Json::Value& json)
{
    if (json.isNull())
        return "";
    else if (string_encoding_func != nullptr)
        return string_encoding_func(json.asString());
    else
        return json.asString();
}

template <> float build<float>(const Json::Value& json)
{
    return json.asFloat();
}

template <> double build<double>(const Json::Value& json)
{
    return json.asDouble();
}

template <> bool build<bool>(const Json::Value& json)
{
    return json.asBool();
}

template <> boost::posix_time::ptime build<boost::posix_time::ptime>(const Json::Value& json)
{
    try
    {
        return boost::posix_time::time_from_string(build<std::string>(json));
    }
    catch (std::exception& e)
    {
        return boost::posix_time::ptime();
    }
}

template <> std::chrono::milliseconds build<std::chrono::milliseconds>(const Json::Value& json)
{
    static const auto regex = boost::xpressive::sregex::compile("((?P<day>\\d+)\.)?(?P<hour>\\d{1,2}):(?P<min>\\d{1,2}):(?P<sec>\\d{1,2})(?:.(?P<ms>\\d+))?");
    auto what = boost::xpressive::smatch();
    auto data = build<std::string>(json);
    if (boost::xpressive::regex_match(data, what, regex) == false)
        throw std::runtime_error("cannot parse timespan");

    auto day = what["day"].matched ? std::stoi(what["day"].str()) : 0;
    auto hours = std::stoi(what["hour"].str());
    auto mins = std::stoi(what["min"].str());
    auto secs = std::stoi(what["sec"].str());
    auto ms = what["ms"].matched ? std::stoi(what["ms"].str()) : 0;

    auto count = day;
    count = count * 24 + hours;
    count = count * 60 + mins;
    count = count * 60 + secs;
    count = count * 1000 + ms;
    return std::chrono::milliseconds(count);
}

template <> dsl build<dsl>(const Json::Value& json)
{
    return dsl(json);
}

template <> date_range build<date_range>(const Json::Value& json)
{
    return date_range(build<std::optional<boost::posix_time::ptime>>(json["Begin"]), build<std::optional<boost::posix_time::ptime>>(json["End"]));
}
#pragma endregion

} } 

#endif