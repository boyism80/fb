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

enum class bundle_type
{
    none = 0, 
    bundle = 1, 
    package = 2
};

template <>
inline bundle_type enum_parse<bundle_type>(const std::string k)
{
    static const std::unordered_map<std::string, bundle_type> enums
    {
        { "none", bundle_type::none }, 
        { "bundle", bundle_type::bundle }, 
        { "package", bundle_type::package }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class classes
{
    none = 0, 
    warrior = 1, 
    thief = 2, 
    magicion = 3, 
    ascetic = 4
};

template <>
inline classes enum_parse<classes>(const std::string k)
{
    static const std::unordered_map<std::string, classes> enums
    {
        { "none", classes::none }, 
        { "warrior", classes::warrior }, 
        { "thief", classes::thief }, 
        { "magicion", classes::magicion }, 
        { "ascetic", classes::ascetic }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class death_penalty
{
    none = 0, 
    drop = 1, 
    destroy = 2
};

template <>
inline death_penalty enum_parse<death_penalty>(const std::string k)
{
    static const std::unordered_map<std::string, death_penalty> enums
    {
        { "none", death_penalty::none }, 
        { "drop", death_penalty::drop }, 
        { "destroy", death_penalty::destroy }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class dsl_type
{
    item = 0
};

template <>
inline dsl_type enum_parse<dsl_type>(const std::string k)
{
    static const std::unordered_map<std::string, dsl_type> enums
    {
        { "item", dsl_type::item }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class item_type
{
    stuff = 1, 
    consume = 2, 
    weapon = 3, 
    armor = 4, 
    helmet = 5, 
    ring = 6, 
    shield = 7, 
    auxiliary = 8, 
    bow = 9
};

template <>
inline item_type enum_parse<item_type>(const std::string k)
{
    static const std::unordered_map<std::string, item_type> enums
    {
        { "stuff", item_type::stuff }, 
        { "consume", item_type::consume }, 
        { "weapon", item_type::weapon }, 
        { "armor", item_type::armor }, 
        { "helmet", item_type::helmet }, 
        { "ring", item_type::ring }, 
        { "shield", item_type::shield }, 
        { "auxiliary", item_type::auxiliary }, 
        { "bow", item_type::bow }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class sex
{
    man = 1, 
    woman = 2, 
    all = man | woman
};

template <>
inline sex enum_parse<sex>(const std::string k)
{
    static const std::unordered_map<std::string, sex> enums
    {
        { "man", sex::man }, 
        { "woman", sex::woman }, 
        { "all", sex::all }
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
    const std::string header;
    const std::vector<std::any> params;

private:
    static std::vector<std::any> parse_params(const Json::Value& json);

public:
    dsl(const std::string& header, const std::vector<std::any>& params) : header(header), params(params)
    { }
    dsl(const Json::Value& json) : header(build<std::string>(json["Type"])), params(parse_params(json))
    { }
    ~dsl()
    { }
};

class fb::model::dsl::item
{
public:
    const uint32_t id;
    const uint32_t count;

public:
    item(uint32_t id, uint32_t count) : 
        id(id),
        count(count)
    { }
    item(const Json::Value& json) : 
        id(fb::model::build<uint32_t>(json[0])),
        count(fb::model::build<uint32_t>(json[1]))
    { }
    item(const std::vector<std::any>& parameters) : 
        id(any_cast<uint32_t>(parameters[0])),
        count(any_cast<uint32_t>(parameters[1]))
    { }

public:
    fb::model::dsl to_dsl()
    {
        return fb::model::dsl("item", {id, count});
    }
};


inline std::vector<std::any> fb::model::dsl::parse_params(const Json::Value& json)
{
    static auto data = std::unordered_map<std::string, std::function<std::vector<std::any>(const Json::Value&)>>
    {
        {"item", [](const Json::Value& json) { return fb::model::dsl::item(json).to_dsl().params; }}
    };

    auto header = build<std::string>(json["Type"]);
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
    const fb::model::classes parent;
    const uint8_t level;
    const uint8_t dexteritry;
    const uint8_t intelligence;
    const uint8_t strength;
    const uint32_t exp;
    const uint32_t hp;
    const uint32_t mp;

public:
    ability(const Json::Value& json) : 
    	parent(fb::model::build<fb::model::classes>(json["parent"])),
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
    const fb::model::classes id;

public:
    ability_attribute(const Json::Value& json) : 
    	id(fb::model::build<fb::model::classes>(json["id"]))
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
    const std::string spell;

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
    	spell(fb::model::build<std::string>(json["spell"]))
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
    const fb::model::item_type type;
    const fb::model::bundle_type bundle;
    const std::string desc;
    const fb::model::death_penalty death_penalty;
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
    	type(fb::model::build<fb::model::item_type>(json["type"])),
    	bundle(fb::model::build<fb::model::bundle_type>(json["bundle"])),
    	desc(fb::model::build<std::string>(json["desc"])),
    	death_penalty(fb::model::build<fb::model::death_penalty>(json["death_penalty"])),
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
    const fb::model::sex sex;
    const uint8_t strength;
    const uint8_t dexterity;
    const uint8_t intelligence;
    const uint8_t level;
    const std::optional<fb::model::classes> class_id;
    const uint8_t promotion;

public:
    item_condition(const Json::Value& json) : 
    	id(fb::model::build<std::string>(json["id"])),
    	sex(fb::model::build<fb::model::sex>(json["sex"])),
    	strength(fb::model::build<uint8_t>(json["strength"])),
    	dexterity(fb::model::build<uint8_t>(json["dexterity"])),
    	intelligence(fb::model::build<uint8_t>(json["intelligence"])),
    	level(fb::model::build<uint8_t>(json["level"])),
    	class_id(fb::model::build<std::optional<fb::model::classes>>(json["class_id"])),
    	promotion(fb::model::build<uint8_t>(json["promotion"]))
	{ }
    ~item_condition()
	{ }
};
class item_mix
{
public:
    const fb::model::dsl source;
    const fb::model::dsl success;
    const fb::model::dsl failed;
    const double percent;

public:
    item_mix(const Json::Value& json) : 
    	source(fb::model::build<dsl>(json["source"])),
    	success(fb::model::build<dsl>(json["success"])),
    	failed(fb::model::build<dsl>(json["failed"])),
    	percent(fb::model::build<double>(json["percent"]))
	{ }
    ~item_mix()
	{ }
};
class promotion
{
public:
    const fb::model::classes parent;
    const uint8_t step;

public:
    promotion(const Json::Value& json) : 
    	parent(fb::model::build<fb::model::classes>(json["parent"])),
    	step(fb::model::build<uint8_t>(json["step"]))
	{ }
    ~promotion()
	{ }
};
class promotion_attribute
{
public:
    const fb::model::classes id;

public:
    promotion_attribute(const Json::Value& json) : 
    	id(fb::model::build<fb::model::classes>(json["id"]))
	{ }
    ~promotion_attribute()
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
            throw std::out_of_range();

        return *found;
    }
};

class __ability : public fb::model::kv_container<fb::model::classes, fb::model::kv_container<uint8_t, fb::model::ability>>
{
public:
    __ability() : fb::model::kv_container<fb::model::classes, fb::model::kv_container<uint8_t, fb::model::ability>>(std::string("json/ability.json"))
    { }
    ~__ability()
    { }
};

class __ability_attribute : public fb::model::kv_container<fb::model::classes, fb::model::ability_attribute>
{
public:
    __ability_attribute() : fb::model::kv_container<fb::model::classes, fb::model::ability_attribute>(std::string("json/ability_attribute.json"))
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

class __item_mix : public fb::model::array_container<fb::model::item_mix>
{
public:
    __item_mix() : fb::model::array_container<fb::model::item_mix>(std::string("json/item_mix.json"))
    { }
    ~__item_mix()
    { }
};

class __promotion : public fb::model::kv_container<fb::model::classes, fb::model::kv_container<uint8_t, fb::model::promotion>>
{
public:
    __promotion() : fb::model::kv_container<fb::model::classes, fb::model::kv_container<uint8_t, fb::model::promotion>>(std::string("json/promotion.json"))
    { }
    ~__promotion()
    { }
};

class __promotion_attribute : public fb::model::kv_container<fb::model::classes, fb::model::promotion_attribute>
{
public:
    __promotion_attribute() : fb::model::kv_container<fb::model::classes, fb::model::promotion_attribute>(std::string("json/promotion_attribute.json"))
    { }
    ~__promotion_attribute()
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
    fb::model::__door door;
    fb::model::__door_pair door_pair;
    fb::model::__equipment_option equipment_option;
    fb::model::__item item;
    fb::model::__item_condition item_condition;
    fb::model::__item_mix item_mix;
    fb::model::__promotion promotion;
    fb::model::__promotion_attribute promotion_attribute;

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
        queue.push([this]() { this->door.load(); });
        queue.push([this]() { this->door_pair.load(); });
        queue.push([this]() { this->equipment_option.load(); });
        queue.push([this]() { this->item.load(); });
        queue.push([this]() { this->item_condition.load(); });
        queue.push([this]() { this->item_mix.load(); });
        queue.push([this]() { this->promotion.load(); });
        queue.push([this]() { this->promotion_attribute.load(); });
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
        return T(enum_parse<T>(json.asCString()));
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
    auto result = json.isString() ? std::atoi(json.asCString()) : json.asInt();
    return static_cast<int8_t>(result);
}

template <> uint8_t build<uint8_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(json.asCString()) : json.asUInt();
    return static_cast<uint8_t>(result);
}

template <> int16_t build<int16_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(json.asCString()) : json.asInt();
    return static_cast<int16_t>(result);
}

template <> uint16_t build<uint16_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(json.asCString()) : json.asUInt();
    return static_cast<uint16_t>(result);
}

template <> int build<int>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(json.asCString()) : json.asInt();
    return static_cast<int>(result);
}

template <> uint32_t build<uint32_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(json.asCString()) : json.asUInt();
    return static_cast<uint32_t>(result);
}

template <> int64_t build<int64_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(json.asCString()) : json.asInt64();
    return static_cast<int64_t>(result);
}

template <> uint64_t build<uint64_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(json.asCString()) : json.asUInt64();
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
        return boost::posix_time::time_from_string(json.asCString());
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