#ifndef __MODEL_PREPROCESSOR_H__
#define __MODEL_PREPROCESSOR_H__

#ifdef DELETE
#undef DELETE
#endif

#define OVERRIDE_OBJECT_TYPE(v)                                                                                                 \
enum_value::OBJECT_TYPE what() const override                                                                                   \
{                                                                                                                               \
    return (v);                                                                                                                 \
}

#define BEGIN_PREPROCESSOR                                                                                                      \
namespace fb { namespace model { namespace enum_value {                                                                         \
                                                                                                                                \
enum class DIRECTION;                                                                                                           \
                                                                                                                                \
} } }                                                                                                                           \
                                                                                                                                \
namespace fb { namespace model {                                                                                                \
class recipe_node;                                                                                                              \
} }                                                                                                                             \
                                                                                                                                \
namespace fb { namespace game {                                                                                                 \
                                                                                                                                \
class context;                                                                                                                  \
class map;                                                                                                                      \
class session;                                                                                                                  \
class buffs;                                                                                                                    \
class objects;                                                                                                                  \
class item;                                                                                                                     \
class items;                                                                                                                    \
class sector;                                                                                                                   \
class sectors;                                                                                                                  \
struct listener;                                                                                                                \
                                                                                                                                \
} }

#define DECLARE_AFTER_TYPE                                                                                                      \
using point8_t = point<uint8_t>;                                                                                                \
using point16_t = point<uint16_t>;                                                                                              \
using point32_t = point<uint32_t>;                                                                                              \
using point64_t = point<uint64_t>;                                                                                              \
                                                                                                                                \
using size8_t = size<uint8_t>;                                                                                                  \
using size16_t = size<uint16_t>;                                                                                                \
using size32_t = size<uint32_t>;                                                                                                \
using size64_t = size<uint64_t>;                                                                                                \
                                                                                                                                \
using range8_t = range<uint8_t>;                                                                                                \
using range16_t = range<uint16_t>;                                                                                              \
using range32_t = range<uint32_t>;                                                                                              \
using range64_t = range<uint64_t>;

#define DECLARE_POINT_EXTENSION                                                                                                 \
public:                                                                                                                         \
    struct point<T>& move(fb::model::enum_value::DIRECTION direction, T step = 1);                                              \
    struct point<T>& forward(fb::model::enum_value::DIRECTION direction, T step = 1);                                           \
    struct point<T>& backward(fb::model::enum_value::DIRECTION direction, T step = 1);                                          \
    struct point<T>& left(fb::model::enum_value::DIRECTION direction, T step = 1);                                              \
    struct point<T>& right(fb::model::enum_value::DIRECTION direction, T step = 1);

#define DECLARE_SIZE_EXTENSION                                                                                                  \
public:                                                                                                                         \
    bool empty() const { return this->width == 0 && this->height == 0; }


#define DECLARE_OBJECT_INHERIT  : public fb::game::lua::luable

#define DECLARE_SPELL_INHERIT  : public fb::game::lua::luable

#define DECLARE_OBJECT_EXTENSION                                                                                                \
public:                                                                                                                         \
    LUA_PROTOTYPE                                                                                                               \
                                                                                                                                \
public:                                                                                                                         \
    template <typename T, typename... Args>                                                                                     \
    T* make(fb::game::context& context, Args&& ... args) const;                                                                 \
                                                                                                                                \
protected:                                                                                                                      \
    uint8_t                                 dialog_look_type() const;                                                           \
                                                                                                                                \
public:                                                                                                                         \
    virtual enum_value::OBJECT_TYPE         what() const;                                                                       \
    bool                                    operator == (const object&) const;                                                  \
    bool                                    operator != (const object&) const;                                                  \
                                                                                                                                \
public:                                                                                                                         \
    static int                              builtin_name(lua_State* lua);                                                       \
    static int                              builtin_look(lua_State* lua);                                                       \
    static int                              builtin_color(lua_State* lua);                                                      \
    static int                              builtin_dialog(lua_State* lua);

#define DECLARE_LIFE_EXTENSION                                                                                                  \
public:                                                                                                                         \
    LUA_PROTOTYPE                                                                                                               \
                                                                                                                                \
public:                                                                                                                         \
    static int                              builtin_hp(lua_State* lua);                                                         \
    static int                              builtin_mp(lua_State* lua);

#define DECLARE_ITEM_EXTENSION                                                                                                  \
public:                                                                                                                         \
    LUA_PROTOTYPE                                                                                                               \
                                                                                                                                \
public:                                                                                                                         \
    OVERRIDE_OBJECT_TYPE(enum_value::OBJECT_TYPE::ITEM)                                                                         \
                                                                                                                                \
public:                                                                                                                         \
    virtual enum_value::ITEM_ATTRIBUTE      attr() const;                                                                       \
    bool                                    attr(enum_value::ITEM_ATTRIBUTE flag) const;                                        \
                                                                                                                                \
public:                                                                                                                         \
    virtual fb::game::item* make(fb::game::context& context, uint16_t count = 1) const;                                         \
                                                                                                                                \
public:                                                                                                                         \
    static int                              builtin_make(lua_State* lua);                                                       \
    static int                              builtin_attr(lua_State* lua);                                                       \
    static int                              builtin_capacity(lua_State* lua);                                                   \
    static int                              builtin_durability(lua_State* lua);                                                 \
    static int                              builtin_price(lua_State* lua);                                                      \
    static int                              builtin_repair_price(lua_State* lua);                                               \
    static int                              builtin_rename_price(lua_State* lua);                                               \
    static int                              builtin_deposit_price(lua_State* lua);

#define DECLARE_CASH_EXTENSION                                                                                                  \
public:                                                                                                                         \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final;                                           \
                                                                                                                                \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                                                       \
    {                                                                                                                           \
        return enum_value::ITEM_ATTRIBUTE::CASH;                                                                                \
    }

#define DECLARE_CONSUME_EXTENSION                                                                                               \
public:                                                                                                                         \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final;                                           \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                                                       \
    {                                                                                                                           \
        auto attr = enum_value::ITEM_ATTRIBUTE::CONSUME;                                                                        \
        if(this->capacity > 1)                                                                                                  \
            attr = enum_value::ITEM_ATTRIBUTE(attr | enum_value::ITEM_ATTRIBUTE::BUNDLE);                                       \
                                                                                                                                \
        return attr;                                                                                                            \
    }

#define DECLARE_PACK_EXTENSION                                                                                                  \
public:                                                                                                                         \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final;                                           \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                                                       \
    {                                                                                                                           \
        return enum_value::ITEM_ATTRIBUTE::PACK;                                                                                \
    }

#define DECLARE_EQUIPMENT_EXTENSION                                                                                             \
public:                                                                                                                         \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                                                       \
    {                                                                                                                           \
        return enum_value::ITEM_ATTRIBUTE::EQUIPMENT;                                                                           \
    }


#define DECLARE_WEAPON_EXTENSION                                                                                                \
public:                                                                                                                         \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final;                                           \
    virtual enum_value::ITEM_ATTRIBUTE      attr() const                                                                        \
    {                                                                                                                           \
        return enum_value::ITEM_ATTRIBUTE::WEAPON;                                                                              \
    }                                                                                                                           \
    enum_value::WEAPON_TYPE              weapon_type() const                                                                    \
    {                                                                                                                           \
        switch(this->look / 10000)                                                                                              \
        {                                                                                                                       \
        case 0:                                                                                                                 \
            return enum_value::WEAPON_TYPE::NORMAL;                                                                             \
                                                                                                                                \
        case 1:                                                                                                                 \
            return enum_value::WEAPON_TYPE::SPEAR;                                                                              \
                                                                                                                                \
        case 2:                                                                                                                 \
            return enum_value::WEAPON_TYPE::BOW;                                                                                \
                                                                                                                                \
        case 3:                                                                                                                 \
            return enum_value::WEAPON_TYPE::FAN;                                                                                \
                                                                                                                                \
        default:                                                                                                                \
            return enum_value::WEAPON_TYPE::UNKNOWN;                                                                            \
        }                                                                                                                       \
    }


#define DECLARE_ARMOR_EXTENSION                                                                                                 \
public:                                                                                                                         \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final;                                           \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                                                       \
    {                                                                                                                           \
        return enum_value::ITEM_ATTRIBUTE::ARMOR;                                                                               \
    }


#define DECLARE_HELMET_EXTENSION                                                                                                \
public:                                                                                                                         \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final;                                           \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                                                       \
    {                                                                                                                           \
        return enum_value::ITEM_ATTRIBUTE::HELMET;                                                                              \
    }

#define DECLARE_SHIELD_EXTENSION                                                                                                \
public:                                                                                                                         \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final;                                           \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                                                       \
    {                                                                                                                           \
        return enum_value::ITEM_ATTRIBUTE::SHIELD;                                                                              \
    }

#define DECLARE_RING_EXTENSION                                                                                                  \
public:                                                                                                                         \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final;                                           \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                                                       \
    {                                                                                                                           \
        return enum_value::ITEM_ATTRIBUTE::RING;                                                                                \
    }

#define DECLARE_AUXILIARY_EXTENSION                                                                                             \
public:                                                                                                                         \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final;                                           \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                                                       \
    {                                                                                                                           \
        return enum_value::ITEM_ATTRIBUTE::AUXILIARY;                                                                           \
    }

#define DECLARE_BOW_EXTENSION                                                                                                   \
public:                                                                                                                         \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final;                                           \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                                                       \
    {                                                                                                                           \
        return enum_value::ITEM_ATTRIBUTE::ARROW;                                                                               \
    }

#define DECLARE_NPC_EXTENSION                                                                                                   \
public:                                                                                                                         \
    LUA_PROTOTYPE                                                                                                               \
                                                                                                                                \
public:                                                                                                                         \
    OVERRIDE_OBJECT_TYPE(enum_value::OBJECT_TYPE::NPC)                                                                          \
                                                                                                                                \
public:                                                                                                                         \
    static int                  builtin_input(lua_State* lua);                                                                  \
    static int                  builtin_menu(lua_State* lua);                                                                   \
    static int                  builtin_item(lua_State* lua);                                                                   \
    static int                  builtin_slot(lua_State* lua);                                                                   \
    static int                  builtin_sell(lua_State* lua);                                                                   \
    static int                  builtin_buy(lua_State* lua);                                                                    \
    static int                  builtin_repair(lua_State* lua);                                                                 \
    static int                  builtin_repair_all(lua_State* lua);                                                             \
    static int                  builtin_hold_money(lua_State* lua);                                                             \
    static int                  builtin_hold_item(lua_State* lua);                                                              \
    static int                  builtin_return_money(lua_State* lua);                                                           \
    static int                  builtin_return_item(lua_State* lua);                                                            \
    static int                  builtin_rename_weapon(lua_State* lua);

#define DECLARE_MOB_EXTENSION                                                                                                   \
public:                                                                                                                         \
    LUA_PROTOTYPE                                                                                                               \
                                                                                                                                \
public:                                                                                                                         \
    OVERRIDE_OBJECT_TYPE(enum_value::OBJECT_TYPE::MOB)                                                                          \
                                                                                                                                \
public:                                                                                                                         \
    static int                  builtin_speed(lua_State* lua);


#define DECLARE_MOB_CONTAINER_EXTENSION                                                                                         \
public:                                                                                                                         \
    fb::model::mob*             name2mob(const std::string& name) const;


#define DECLARE_NPC_CONTAINER_EXTENSION                                                                                         \
public:                                                                                                                         \
    fb::model::npc*             name2npc(const std::string& name) const;

#define DECLARE_ITEM_CONTAINER_EXTENSION                                                                                        \
public:                                                                                                                         \
    fb::model::item*            name2item(const std::string& name) const;


#define DECLARE_MAP_INHERIT : public fb::game::lua::luable


#define DECLARE_MAP_EXTENSION                                                                                                   \
public:                                                                                                                         \
    LUA_PROTOTYPE                                                                                                               \
                                                                                                                                \
public:                                                                                                                         \
    static int                      builtin_name(lua_State* lua);                                                               \
    static int                      builtin_width(lua_State* lua);                                                              \
    static int                      builtin_height(lua_State* lua);                                                             \
    static int                      builtin_area(lua_State* lua);

                                                                                              
#define DECLARE_MAP_CONTAINER_EXTENSION                                                                                         \
public:                                                                                                                         \
    fb::model::map*                 name2map(const std::string& name) const;

#define DECLARE_DOOR_EXTENSION                                                                                                  \
public:                                                                                                                         \
    const uint16_t width;                                                                                                       \
public:                                                                                                                         \
    bool matched(const fb::game::map& map, const point16_t& position, bool is_open) const;                                      \
    bool find(const fb::game::map& map, point16_t& position, bool is_open) const;

#define DECLARE_DOOR_INITIALIZER ,                                                                                              \
    width(static_cast<uint16_t>(this->pairs.size()))

#define DECLARE_SPELL_CONTAINER_EXTENSION                                                                                       \
public:                                                                                                                         \
    fb::model::spell*               name2spell(const std::string& name) const;


#define DECLARE_SPELL_EXTENSION                                                                                                 \
public:                                                                                                                         \
    interface listener;                                                                                                         \
public:                                                                                                                         \
    LUA_PROTOTYPE                                                                                                               \
                                                                                                                                \
public:                                                                                                                         \
    static int                  builtin_type(lua_State* lua);                                                                   \
    static int                  builtin_name(lua_State* lua);                                                                   \
    static int                  builtin_message(lua_State* lua);


#define DECLARE_PROMOTION_CONTAINER_EXTENSION                                                                                   \
public:                                                                                                                         \
    const promotion* operator () (enum_value::CLASS cls, uint8_t promotion) const;                                              \
    bool name2class(const std::string& name, enum_value::CLASS& cls, uint8_t& promotion) const;

#define DECLARE_CONST_REGEX_EXTENSION                                                                                           \
public:                                                                                                                         \
    static bool match_sell_message(const std::string& message, std::string& item, std::optional<uint16_t>& count);              \
    static bool match_buy_message(const std::string& message, std::string& item, uint16_t& count);                              \
    static bool match_repair_message(const std::string& message, std::string& item);                                            \
    static bool match_deposit_money_message(const std::string& message, std::optional<uint32_t>& money);                        \
    static bool match_withdraw_money_message(const std::string& message, std::optional<uint32_t>& money);                       \
    static bool match_deposit_item_message(const std::string& message, std::string& item, std::optional<uint16_t>& count);      \
    static bool match_withdraw_item_message(const std::string& message, std::string& item, std::optional<uint16_t>& count);     \
    static bool match_sell_list(const std::string& message);                                                                    \
    static bool match_buy_list(const std::string& message);                                                                     \
    static bool match_sell_price(const std::string& message, std::string& item);                                                \
    static bool match_buy_price(const std::string& message, std::string& item);                                                 \
    static bool match_deposited_money(const std::string& message);                                                              \
    static bool match_rename_weapon(const std::string& message, std::string& weapon, std::string& name);                        \
    static bool match_hold_item_list(const std::string& message);                                                               \
    static bool match_hold_item_count(const std::string& message, std::string& item);

#define DECLARE_SELL_CONTAINER_EXTENSION                                                                                        \
public:                                                                                                                         \
    const fb::model::sell* find(uint32_t pursuit, const fb::model::item& item) const;                                           \
    const fb::model::sell* find(const fb::model::npc& npc, const fb::model::item& item) const;

#define DECLARE_BUY_CONTAINER_EXTENSION                                                                                         \
public:                                                                                                                         \
    const fb::model::buy* find(uint32_t pursuit, const fb::model::item& item) const;                                            \
    const fb::model::buy* find(const fb::model::npc& npc, const fb::model::item& item) const;

#define DECLARE_RECIPE_EXTENSION                                                                                                \
public:                                                                                                                         \
    recipe(const std::vector<fb::model::dsl>& source,                                                                           \
        const std::vector<fb::model::dsl>& success,                                                                             \
        const std::vector<fb::model::dsl>& failed,                                                                              \
        double percent) : source(source), success(success), failed(failed), percent(percent)                                    \
    { }

#define DECLARE_RECIPE_CONTAINER_CUSTOM_CONSTRUCTOR                                                                             \
    __recipe();                                                                                                                 \
    __recipe(const __recipe&) = delete;                                                                                         \
    ~__recipe();

#define DECLARE_RECIPE_CONTAINER_EXTENSION                                                                                      \
private:                                                                                                                        \
    fb::model::recipe_node* _root;                                                                                              \
                                                                                                                                \
public:                                                                                                                         \
    const fb::model::recipe* find(const std::vector<fb::model::dsl::item>& dsl) const;

#define DECLARE_AFTER_ENUM                                                                                                      \
template <typename T>                                                                                                           \
struct point<T>& point<T>::move(fb::model::enum_value::DIRECTION direction, T step)                                             \
{                                                                                                                               \
    switch(direction)                                                                                                           \
    {                                                                                                                           \
    case fb::model::enum_value::DIRECTION::TOP:                                                                                 \
        this->y -= step;                                                                                                        \
        break;                                                                                                                  \
                                                                                                                                \
    case fb::model::enum_value::DIRECTION::BOTTOM:                                                                              \
        this->y += step;                                                                                                        \
        break;                                                                                                                  \
                                                                                                                                \
    case fb::model::enum_value::DIRECTION::LEFT:                                                                                \
        this->x -= step;                                                                                                        \
        break;                                                                                                                  \
                                                                                                                                \
    case fb::model::enum_value::DIRECTION::RIGHT:                                                                               \
        this->x += step;                                                                                                        \
        break;                                                                                                                  \
    }                                                                                                                           \
                                                                                                                                \
    return *this;                                                                                                               \
}                                                                                                                               \
                                                                                                                                \
template <typename T>                                                                                                           \
struct point<T>& point<T>::forward(fb::model::enum_value::DIRECTION direction, T step)                                          \
{                                                                                                                               \
    return this->move(direction, step);                                                                                         \
}                                                                                                                               \
                                                                                                                                \
template <typename T>                                                                                                           \
struct point<T>& point<T>::backward(fb::model::enum_value::DIRECTION direction, T step)                                         \
{                                                                                                                               \
    auto newdir = direction;                                                                                                    \
    switch(direction)                                                                                                           \
    {                                                                                                                           \
    case fb::model::enum_value::DIRECTION::TOP:                                                                                 \
        newdir = fb::model::enum_value::DIRECTION::BOTTOM;                                                                      \
        break;                                                                                                                  \
                                                                                                                                \
    case fb::model::enum_value::DIRECTION::BOTTOM:                                                                              \
        newdir = fb::model::enum_value::DIRECTION::TOP;                                                                         \
        break;                                                                                                                  \
                                                                                                                                \
    case fb::model::enum_value::DIRECTION::LEFT:                                                                                \
        newdir = fb::model::enum_value::DIRECTION::RIGHT;                                                                       \
        break;                                                                                                                  \
                                                                                                                                \
    case fb::model::enum_value::DIRECTION::RIGHT:                                                                               \
        newdir = fb::model::enum_value::DIRECTION::LEFT;                                                                        \
        break;                                                                                                                  \
    }                                                                                                                           \
                                                                                                                                \
    return this->move(newdir, step);                                                                                            \
}                                                                                                                               \
                                                                                                                                \
template <typename T>                                                                                                           \
struct point<T>& point<T>::left(fb::model::enum_value::DIRECTION direction, T step)                                             \
{                                                                                                                               \
    auto newdir = direction;                                                                                                    \
    switch(direction)                                                                                                           \
    {                                                                                                                           \
    case fb::model::enum_value::DIRECTION::TOP:                                                                                 \
        newdir = fb::model::enum_value::DIRECTION::LEFT;                                                                        \
        break;                                                                                                                  \
                                                                                                                                \
    case fb::model::enum_value::DIRECTION::BOTTOM:                                                                              \
        newdir = fb::model::enum_value::DIRECTION::RIGHT;                                                                       \
        break;                                                                                                                  \
                                                                                                                                \
    case fb::model::enum_value::DIRECTION::LEFT:                                                                                \
        newdir = fb::model::enum_value::DIRECTION::BOTTOM;                                                                      \
        break;                                                                                                                  \
                                                                                                                                \
    case fb::model::enum_value::DIRECTION::RIGHT:                                                                               \
        newdir = fb::model::enum_value::DIRECTION::TOP;                                                                         \
        break;                                                                                                                  \
    }                                                                                                                           \
                                                                                                                                \
    return this->move(newdir, step);                                                                                            \
}                                                                                                                               \
                                                                                                                                \
template <typename T>                                                                                                           \
struct point<T>& point<T>::right(fb::model::enum_value::DIRECTION direction, T step)                                            \
{                                                                                                                               \
    auto newdir = direction;                                                                                                    \
    switch(direction)                                                                                                           \
    {                                                                                                                           \
    case fb::model::enum_value::DIRECTION::TOP:                                                                                 \
        newdir = fb::model::enum_value::DIRECTION::RIGHT;                                                                       \
        break;                                                                                                                  \
                                                                                                                                \
    case fb::model::enum_value::DIRECTION::BOTTOM:                                                                              \
        newdir = fb::model::enum_value::DIRECTION::LEFT;                                                                        \
        break;                                                                                                                  \
                                                                                                                                \
    case fb::model::enum_value::DIRECTION::LEFT:                                                                                \
        newdir = fb::model::enum_value::DIRECTION::TOP;                                                                         \
        break;                                                                                                                  \
                                                                                                                                \
    case fb::model::enum_value::DIRECTION::RIGHT:                                                                               \
        newdir = fb::model::enum_value::DIRECTION::BOTTOM;                                                                      \
        break;                                                                                                                  \
    }                                                                                                                           \
                                                                                                                                \
    return this->move(newdir, step);                                                                                            \
}                                       
#endif