#ifndef __MODEL_PREPROCESSOR_H__
#define __MODEL_PREPROCESSOR_H__

#define ROOT_PREPROCESSOR                                                                     \
namespace fb { namespace game {                                                               \
                                                                                              \
class context;                                                                                \
class map;                                                                                    \
class session;                                                                                \
class buffs;                                                                                  \
class objects;                                                                                \
class item;                                                                                   \
class items;                                                                                  \
class sector;                                                                                 \
class sectors;                                                                                \
                                                                                              \
} }


#define DECLARE_OBJECT_INHERIT  : public fb::game::lua::luable

#define DECLARE_OBJECT_EXTENSION                                                              \
public:                                                                                       \
    LUA_PROTOTYPE                                                                             \
                                                                                              \
public:                                                                                       \
    template <typename T, typename... Args>                                                   \
    T* make(fb::game::context& context, Args... args) const                                   \
    {                                                                                         \
        return new T(context, static_cast<const typename T::model*>(this), args...);          \
    }                                                                                         \
                                                                                              \
protected:                                                                                    \
    uint8_t                                 dialog_look_type() const;                         \
                                                                                              \
public:                                                                                       \
    virtual enum_value::OBJECT_TYPE         type() const;                                     \
                                                                                              \
public:                                                                                       \
    static int                              builtin_name(lua_State* lua);                     \
    static int                              builtin_look(lua_State* lua);                     \
    static int                              builtin_color(lua_State* lua);                    \
    static int                              builtin_dialog(lua_State* lua);

#define DECLARE_LIFE_EXTENSION                                                                \
public:                                                                                       \
    LUA_PROTOTYPE                                                                             \
                                                                                              \
public:                                                                                       \
    static int                              builtin_hp(lua_State* lua);                       \
    static int                              builtin_mp(lua_State* lua);

#define DECLARE_ITEM_EXTENSION                                                                \
public:                                                                                       \
    LUA_PROTOTYPE                                                                             \
                                                                                              \
public:                                                                                       \
    virtual enum_value::ITEM_ATTRIBUTE           attr() const;                                \
    bool                                    attr(enum_value::ITEM_ATTRIBUTE flag) const;      \
                                                                                              \
public:                                                                                       \
    virtual fb::game::item* make(fb::game::context& context, uint16_t count = 1) const        \
    {                                                                                         \
        return nullptr;                                                                       \
    }                                                                                         \
                                                                                              \
public:                                                                                       \
    static int                              builtin_make(lua_State* lua);                     \
    static int                              builtin_attr(lua_State* lua);                     \
    static int                              builtin_capacity(lua_State* lua);                 \
    static int                              builtin_durability(lua_State* lua);               \
    static int                              builtin_price(lua_State* lua);                    \
    static int                              builtin_repair_price(lua_State* lua);             \
    static int                              builtin_rename_price(lua_State* lua);             \
    static int                              builtin_deposit_price(lua_State* lua);

#define DECLARE_CASH_EXTENSION                                                                \
public:                                                                                       \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final          \
    {                                                                                         \
        return nullptr;                                                                       \
    }                                                                                         \
                                                                                              \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                     \
    {                                                                                         \
        return enum_value::ITEM_ATTRIBUTE::CASH;                                              \
    }

#define DECLARE_CONSUME_EXTENSION                                                             \
public:                                                                                       \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final          \
    {                                                                                         \
        return nullptr;                                                                       \
    }                                                                                         \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                     \
    {                                                                                         \
        auto attr = enum_value::ITEM_ATTRIBUTE::CONSUME;                                      \
        if(this->capacity > 1)                                                                \
            attr = enum_value::ITEM_ATTRIBUTE(attr | enum_value::ITEM_ATTRIBUTE::BUNDLE);     \
                                                                                              \
        return attr;                                                                          \
    }

#define DECLARE_PACK_EXTENSION                                                                \
public:                                                                                       \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final          \
    {                                                                                         \
        return nullptr;                                                                       \
    }                                                                                         \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                     \
    {                                                                                         \
        return enum_value::ITEM_ATTRIBUTE::PACK;                                              \
    }

#define DECLARE_EQUIPMENT_EXTENSION                                                           \
public:                                                                                       \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                     \
    {                                                                                         \
        return enum_value::ITEM_ATTRIBUTE::EQUIPMENT;                                         \
    }


#define DECLARE_WEAPON_EXTENSION                                                              \
public:                                                                                       \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final          \
    {                                                                                         \
        return nullptr;                                                                       \
    }                                                                                         \
    virtual enum_value::ITEM_ATTRIBUTE      attr() const                                      \
    {                                                                                         \
        return enum_value::ITEM_ATTRIBUTE::WEAPON;                                            \
    }                                                                                         \
    enum_value::WEAPON_TYPE              weapon_type() const                                  \
    {                                                                                         \
        switch(this->look / 10000)                                                            \
        {                                                                                     \
        case 0:                                                                               \
            return enum_value::WEAPON_TYPE::NORMAL;                                           \
                                                                                              \
        case 1:                                                                               \
            return enum_value::WEAPON_TYPE::SPEAR;                                            \
                                                                                              \
        case 2:                                                                               \
            return enum_value::WEAPON_TYPE::BOW;                                              \
                                                                                              \
        case 3:                                                                               \
            return enum_value::WEAPON_TYPE::FAN;                                              \
                                                                                              \
        default:                                                                              \
            return enum_value::WEAPON_TYPE::UNKNOWN;                                          \
        }                                                                                     \
    }


#define DECLARE_ARMOR_EXTENSION                                                               \
public:                                                                                       \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final          \
    {                                                                                         \
        return nullptr;                                                                       \
    }                                                                                         \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                     \
    {                                                                                         \
        return enum_value::ITEM_ATTRIBUTE::ARMOR;                                             \
    }


#define DECLARE_HELMET_EXTENSION                                                              \
public:                                                                                       \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final          \
    {                                                                                         \
        return nullptr;                                                                       \
    }                                                                                         \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                     \
    {                                                                                         \
        return enum_value::ITEM_ATTRIBUTE::HELMET;                                            \
    }

#define DECLARE_SHIELD_EXTENSION                                                              \
public:                                                                                       \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final          \
    {                                                                                         \
        return nullptr;                                                                       \
    }                                                                                         \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                     \
    {                                                                                         \
        return enum_value::ITEM_ATTRIBUTE::SHIELD;                                            \
    }

#define DECLARE_RING_EXTENSION                                                                \
public:                                                                                       \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final          \
    {                                                                                         \
        return nullptr;                                                                       \
    }                                                                                         \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                     \
    {                                                                                         \
        return enum_value::ITEM_ATTRIBUTE::RING;                                              \
    }

#define DECLARE_AUXILIARY_EXTENSION                                                           \
public:                                                                                       \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final          \
    {                                                                                         \
        return nullptr;                                                                       \
    }                                                                                         \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                     \
    {                                                                                         \
        return enum_value::ITEM_ATTRIBUTE::AUXILIARY;                                         \
    }

#define DECLARE_BOW_EXTENSION                                                                 \
public:                                                                                       \
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final          \
    {                                                                                         \
        return nullptr;                                                                       \
    }                                                                                         \
    virtual enum_value::ITEM_ATTRIBUTE       attr() const                                     \
    {                                                                                         \
        return enum_value::ITEM_ATTRIBUTE::ARROW;                                             \
    }

#define DECLARE_NPC_EXTENSION                                                                 \
public:                                                                                       \
    LUA_PROTOTYPE                                                                             \
                                                                                              \
public:                                                                                       \
    static int                  builtin_input(lua_State* lua);                                \
    static int                  builtin_menu(lua_State* lua);                                 \
    static int                  builtin_item(lua_State* lua);                                 \
    static int                  builtin_slot(lua_State* lua);                                 \
    static int                  builtin_sell(lua_State* lua);                                 \
    static int                  builtin_buy(lua_State* lua);                                  \
    static int                  builtin_repair(lua_State* lua);                               \
    static int                  builtin_repair_all(lua_State* lua);                           \
    static int                  builtin_hold_money(lua_State* lua);                           \
    static int                  builtin_hold_item(lua_State* lua);                            \
    static int                  builtin_return_money(lua_State* lua);                         \
    static int                  builtin_return_item(lua_State* lua);                          \
    static int                  builtin_rename_weapon(lua_State* lua);

#define DECLARE_MOB_EXTENSION                                                                 \
public:                                                                                       \
    LUA_PROTOTYPE                                                                             \
                                                                                              \
public:                                                                                       \
    enum_value::OBJECT_TYPE     type() const                                                  \
    {                                                                                         \
        return enum_value::OBJECT_TYPE::MOB;                                                  \
    }                                                                                         \
                                                                                              \
public:                                                                                       \
    static int                  builtin_speed(lua_State* lua);                                

#endif