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
    enum class penalties { NONE, DROP, DESTRUCTION };

    enum class delete_attr : uint8_t
    {
        REMOVED     = 0x00,
        DROP        = 0x01,
        EAT         = 0x02,
        SMOKE       = 0x03,
        THROW       = 0x04,
        SHOOT       = 0x05,
        REDUCE      = 0x06,
        STICK       = 0x07,
        DECAY       = 0x08,
        GIVE        = 0x09,
        SELL        = 0x0A,
        NONE        = 0x0C,
        DESTROY     = 0x0D,
    };

    enum class attrs : uint32_t
    {
        NONE        = 0x00000000,
        CONSUME     = 0x00000001,
        BUNDLE      = 0x00000002,
        SCRIPT      = 0x00000003,
        CASH        = 0x00000004,
        PACK        = CONSUME   | 0x00000100,
        EQUIPMENT   = 0x10000000,
        WEAPON      = EQUIPMENT | 0x00000100,
        ARMOR       = EQUIPMENT | 0x00000200,
        SHIELD      = EQUIPMENT | 0x00000400,
        HELMET      = EQUIPMENT | 0x00000800,
        RING        = EQUIPMENT | 0x00001000,
        AUXILIARY   = EQUIPMENT | 0x00002000,
        ARROW       = EQUIPMENT | 0x00004000,
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
    
    static const conditions                 DEFAULT_CONDITION;
    static constexpr uint32_t               MAX_SLOT = 52;

#pragma endregion

#pragma region protected field
protected:
    uint16_t                                _count = 0;
    session*                                _owner = nullptr;
#pragma endregion

#pragma region constructor / destructor
public:
    item(fb::game::context* context, const fb::game::item::master* master, uint16_t count = 1);
    item(const item& right);
    virtual ~item();
#pragma endregion

#pragma region virtual method

public:
    virtual std::string                     tip_message() const;

public:
    virtual const std::string               name_styled() const;
    virtual const std::string               name_trade() const;
    virtual std::optional<uint16_t>         durability() const;
    virtual void                            durability(std::optional<uint16_t> value);

#pragma endregion

#pragma region public method

public:
    uint16_t                                fill(uint16_t count);
    uint16_t                                reduce(uint16_t count);
    uint16_t                                free_space() const;
    uint16_t                                count() const;
    void                                    count(uint16_t value);
    virtual bool                            empty() const;

public:
    attrs                                   attr() const;
    bool                                    attr(fb::game::item::attrs flag) const;
    fb::game::session*                      owner() const;
    void                                    owner(fb::game::session* owner);

#pragma endregion

#pragma region event method
public:
    virtual bool                            active();
    virtual item*                           split(uint16_t count = 1);
    virtual void                            merge(fb::game::item& item);

#pragma endregion
};


#pragma region nested listener
interface item::listener : public virtual fb::game::object::listener
{
    virtual void on_item_remove(session& me, uint8_t index, item::delete_attr attr = item::delete_attr::NONE) = 0;
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
    const bool                              enabled = false;

public:
    explicit trade(bool enabled) : enabled(enabled) { }
};

struct item::storage
{
public:
    friend class item;

public:
    const bool                              enabled = false;
    const uint32_t                          price   = 0;

public:
    storage(bool enabled, uint32_t price) : enabled(enabled), price(price) { }
};

struct fb::game::item::conditions
{
public:
    const uint8_t                           level           = 0;
    const uint8_t                           strength        = 0;
    const uint8_t                           dexteritry      = 0;
    const uint8_t                           intelligence    = 0;
    const uint8_t                           cls             = 0;
    const uint8_t                           promotion       = 0;
    const fb::game::sex                     sex             = fb::game::sex::BOTH;

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
    const uint32_t                           id;
    const uint32_t                           price;
    const fb::game::item::conditions         condition;
    const penalties                          penalty;
    const uint16_t                           capacity;
    const fb::game::item::trade              trade;
    const fb::game::item::storage            storage;
    const std::string                        desc;
    const std::string                        active_script;
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
    fb::game::object::types                  type() const override { return object::types::ITEM; }
#pragma endregion

#pragma region virtual method
public:
    virtual fb::game::item::attrs            attr() const;
    bool                                     attr(fb::game::item::attrs flag) const;
    virtual fb::game::item*                  make(fb::game::context* context) const;
#pragma endregion

#pragma region template method
public:
    template <typename T = fb::game::item>
    T* make(fb::game::context* context, int count = 1) const 
    {
        auto created = static_cast<T*>(this->make(context)); 
        created->count(count);
        return created;
    }
#pragma endregion

#pragma region built-in method
public:
    static int                               builtin_make(lua_State* lua);
#pragma endregion

};
#pragma endregion

} }

#endif // !__ITEM_BASE_H__
