#ifndef __ITEM_BASE_H__
#define __ITEM_BASE_H__

#include <string>
#include <sstream>
#include <iomanip>
#include <optional>
#include "model/object/object.h"
#include "model/container/container.h"

#ifdef small
#undef small
#endif

namespace fb { namespace game {

#pragma region forward declaration
class session;
class items;
#pragma endregion

class item : public object
{
#pragma region friend
    friend class items;
#pragma endregion

#pragma region forward nested declaration
public:
    interface listener;

public:
    class master;
#pragma endregion

#pragma region enum
public:
    enum penalties { NONE, DROP, DESTRUCTION };

    enum delete_attr : uint8_t
    {
        DELETE_REMOVED                  = 0x00,
        DELETE_DROP                     = 0x01,
        DELETE_EAT                      = 0x02,
        DELETE_SMOKE                    = 0x03,
        DELETE_THROW                    = 0x04,
        DELETE_SHOOT                    = 0x05,
        DELETE_REDUCE                   = 0x06,
        DELETE_STICK                    = 0x07,
        DELETE_DECAY                    = 0x08,
        DELETE_GIVE                     = 0x09,
        DELETE_SELL                     = 0x0A,
        DELETE_NONE                     = 0x0C,
        DELETE_DESTROY                  = 0x0D,
    };

    enum attrs : uint32_t
    {
        ITEM_ATTR_NONE                  = 0x00000000,
        ITEM_ATTR_CONSUME               = 0x00000001,
        ITEM_ATTR_BUNDLE                = 0x00000002,
        ITEM_ATTR_SCRIPT                = 0x00000003,
        ITEM_ATTR_CASH                  = 0x00000004,
        ITEM_ATTR_PACK                  = ITEM_ATTR_CONSUME   | 0x00000100,
        ITEM_ATTR_EQUIPMENT             = 0x10000000,
        ITEM_ATTR_WEAPON                = ITEM_ATTR_EQUIPMENT | 0x00000100,
        ITEM_ATTR_ARMOR                 = ITEM_ATTR_EQUIPMENT | 0x00000200,
        ITEM_ATTR_SHIELD                = ITEM_ATTR_EQUIPMENT | 0x00000400,
        ITEM_ATTR_HELMET                = ITEM_ATTR_EQUIPMENT | 0x00000800,
        ITEM_ATTR_RING                  = ITEM_ATTR_EQUIPMENT | 0x00001000,
        ITEM_ATTR_AUXILIARY             = ITEM_ATTR_EQUIPMENT | 0x00002000,
        ITEM_ATTR_ARROW                 = ITEM_ATTR_EQUIPMENT | 0x00004000,
    };
#pragma endregion

#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion

#pragma region structure
public:
    struct trade;
    struct storage;
    struct conditions;

#pragma endregion

#pragma region exception
public:
    DECLARE_EXCEPTION(full_inven_exception, "소지품이 꽉 찼습니다.")
#pragma endregion

#pragma region static const field
    
    static const conditions             DEFAULT_CONDITION;
    static const uint32_t               MAX_SLOT = 52;

#pragma endregion

#pragma region protected field
protected:
    uint16_t                            _count;
    session*                            _owner;
#pragma endregion

#pragma region constructor / destructor
public:
    item(const fb::game::item::master* master, fb::game::item::listener* listener, uint16_t count = 1);
    item(const item& right);
    virtual ~item();
#pragma endregion

#pragma region virtual method

public:
    virtual std::string                 tip_message() const;

public:
    virtual const std::string           name_styled() const;
    virtual const std::string           name_trade() const;
    virtual std::optional<uint16_t>     durability() const;
    virtual void                        durability(std::optional<uint16_t> value);

#pragma endregion

#pragma region public method

public:
    uint16_t                            fill(uint16_t count);
    uint16_t                            reduce(uint16_t count);
    uint16_t                            free_space() const;
    uint16_t                            count() const;
    void                                count(uint16_t value);
    virtual bool                        empty() const;

public:
    attrs                               attr() const;
    bool                                attr(fb::game::item::attrs flag) const;
    fb::game::session*                  owner() const;
    void                                owner(fb::game::session* owner);

#pragma endregion

#pragma region event method
public:
    virtual bool                        active();
    virtual item*                       split(uint16_t count = 1);
    virtual void                        merge(fb::game::item& item);

#pragma endregion
};


#pragma region nested listener
interface item::listener : public virtual fb::game::object::listener
{
    virtual void on_item_remove(session& me, uint8_t index, item::delete_attr attr = item::delete_attr::DELETE_NONE) = 0;
    virtual void on_item_update(session& me, uint8_t index) = 0;
    virtual void on_item_swap(session& me, uint8_t src, uint8_t dst) = 0;
    virtual void on_item_active(session& me, item& item) = 0;
    virtual void on_item_throws(session& me, item& item, const point16_t& to) = 0;
};
#pragma endregion

#pragma region nested structure
struct item::trade
{
public:
    friend class item;

public:
    const bool                          enabled;

public:
    explicit trade(bool enabled) : enabled(enabled) {}
};

struct item::storage
{
public:
    friend class item;

public:
    bool                                enabled;
    uint32_t                            price;

public:
    explicit storage(bool enabled, uint32_t price) : enabled(enabled), price(price) {}
};

struct fb::game::item::conditions
{
public:
    const uint8_t                       level;
    const uint8_t                       strength;
    const uint8_t                       dexteritry;
    const uint8_t                       intelligence;
    const uint8_t                       cls;
    const uint8_t                       promotion;
    const fb::game::sex                 sex;

public:
    conditions();
    conditions(uint8_t level,
               uint8_t strength,
               uint8_t dexteritry,
               uint8_t intelligence,
               uint8_t cls,
               uint8_t promotion,
               fb::game::sex sex);
    conditions(const fb::game::item::conditions& right);
};

class item::master : public fb::game::object::master
{
#pragma region friend
    friend class fb::game::item;
#pragma endregion

#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion

#pragma region public field
public:
    const uint32_t                      id;
    const uint32_t                      price;
    const fb::game::item::conditions    condition;
    const penalties                     penalty;
    const uint16_t                      capacity;
    const fb::game::item::trade         trade;
    const fb::game::item::storage       storage;
    const std::string                   desc;
    const std::string                   active_script;
#pragma endregion

#pragma region constructor / destructor
public:
    master(const std::string&                name, 
           uint16_t                          look, 
           uint8_t                           color,
           uint32_t                          id,
           uint32_t                          price,
           const fb::game::item::conditions& condition,
           penalties                         penalty,
           uint16_t                          capacity,
           const fb::game::item::trade&      trade,
           const fb::game::item::storage&    storage,
           std::string                       desc,
           std::string                       active_script);
    virtual ~master();
#pragma endregion

#pragma region override method
public:
    fb::game::object::types             type() const { return object::ITEM; }
#pragma endregion

#pragma region virtual method
public:
    virtual fb::game::item::attrs       attr() const;
    bool                                attr(fb::game::item::attrs flag) const;
    virtual fb::game::item*             make(fb::game::item::listener* listener) const;
#pragma endregion

#pragma region template method
public:
    template <typename T = fb::game::item>
    T* make(fb::game::item::listener* listener, int count = 1) const 
    {
        auto created = static_cast<T*>(this->make(listener)); 
        created->count(count);
        return created;
    }
#pragma endregion

#pragma region built-in method
public:
    static int                          builtin_make(lua_State* lua);
#pragma endregion

};
#pragma endregion

} }

#endif // !__ITEM_BASE_H__
