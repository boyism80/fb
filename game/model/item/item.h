#ifndef __ITEM_H__
#define __ITEM_H__

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
#pragma region forward declaration
public:
enum delete_attr : uint8_t;
#pragma endregion

#pragma region listener
public:
interface listener : public virtual fb::game::object::listener
{
    virtual void on_item_remove(session& me, uint8_t index, item::delete_attr attr = item::delete_attr::DELETE_NONE) = 0;
    virtual void on_item_update(session& me, uint8_t index) = 0;
    virtual void on_item_swap(session& me, uint8_t src, uint8_t dst) = 0;
    virtual void on_item_active(session& me, item& item) = 0;
    virtual void on_item_throws(session& me, item& item, const point16_t& to) = 0;
};
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
    typedef struct _trade
    {
    private:
        bool                            _enabled;

        friend class item;

    public:
        explicit _trade(bool enabled = true) : _enabled(enabled) {}

    public:
        bool                            enabled() const { return this->_enabled; }
        void                            enabled(bool value) { this->_enabled = value; }
    } trade;

    typedef struct _entrust
    {
    private:
        bool                            _enabled;
        uint32_t                        _price;

        friend class item;

    public:
        explicit _entrust(bool enabled = true, uint32_t price = 0) : _enabled(enabled), _price(price) {}

    public:
        bool                            enabled() const { return this->_enabled; }
        void                            enabled(bool value) { this->_enabled = value; }
        
        uint32_t                        price() const { return this->_price; }
        void                            price(uint32_t value) { this->_price = value; }
    } entrust;

    typedef struct _limit
    {
        uint8_t                         level;
        uint8_t                         strength;
        uint8_t                         dexteritry;
        uint8_t                         intelligence;
        uint8_t                         cls, promotion;
        fb::game::sex                   sex;

    public:
        _limit();
        _limit(const _limit& right);
    } item_limit;

#pragma endregion


#pragma region friend
    friend class items;
#pragma endregion


#pragma region exception
public:
    DECLARE_EXCEPTION(full_inven_exception, "소지품이 꽉 찼습니다.")
#pragma endregion


#pragma region static const field
    
    static const item_limit             DEFAULT_LIMIT;
    static const uint32_t               MAX_SLOT = 52;

#pragma endregion


#pragma region master class
public:
class master : public fb::game::object::master
{
#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion

#pragma region protected field
protected:
    uint32_t                            _id;
    uint32_t                            _price;
    item_limit                          _limit;
    penalties                           _penalty;
    uint16_t                            _capacity;
    item::trade                         _trade;
    item::entrust                       _entrust;
    bool                                _bundle;
    std::string                         _tooltip, _desc;
    std::string                         _active_script;
#pragma endregion


#pragma region friend
    friend class fb::game::item;
#pragma endregion


#pragma region constructor / destructor
public:
    master(uint32_t id, const std::string& name, uint16_t look, uint8_t color = 0, uint16_t capacity = 1, 
        const item_limit& limit = DEFAULT_LIMIT);
    master(const fb::game::object::master& master);
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


#pragma region public method
public:
    uint32_t                            id() const;
    void                                id(uint32_t id);
    uint32_t                            price() const;
    void                                price(uint32_t value);

    uint16_t                            capacity() const;
    void                                capacity(uint16_t value);

    bool                                trade() const;
    void                                trade(bool value);

    bool                                entrust_enabled() const;
    void                                entrust_enabled(bool value);

    uint32_t                            entrust_price() const;
    void                                entrust_price(uint32_t value);

    const item_limit&                   limit() const;
    void                                limit(const item::item_limit& value);

    penalties                           penalty() const;
    void                                penalty(penalties value);

    const std::string&                  desc() const;
    void                                desc(const std::string& value);

    const std::string&                  active_script() const;
    void                                active_script(const std::string& value);

#pragma endregion

#pragma region built-in method
public:
    static int                          builtin_make(lua_State* lua);
#pragma endregion

};
#pragma endregion


#pragma region private field

#pragma endregion


#pragma region protected field

protected:
    uint16_t                            _count;
    session*                            _owner;

#pragma endregion


#pragma region constructor / destructor

public:
    item(const fb::game::item::master* master, listener* listener, uint16_t count = 1);
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
    uint32_t                            price() const;
    uint16_t                            capacity() const;
    bool                                unique() const;
    bool                                entrust_enabled() const;
    uint32_t                            entrust_price() const;
    const item_limit&                   limit() const;
    penalties                           penalty() const;
    const std::string&                  desc() const;
    const std::string&                  active_script() const;
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

//
// class cash
// 게임머니
//
class cash : public item
{
#pragma region master
public:
    class master : public fb::game::item::master
    {
    public:
        master(uint32_t id, const std::string& name, uint16_t look, uint8_t color = 0);
        ~master();

    public:
        virtual fb::game::item::attrs   attr() const;
        virtual fb::game::item*         make(fb::game::item::listener* listener) const;
    };
#pragma endregion

public:
    static const master                 BRONZE, BRONZE_BUNDLE, SILVER, SILVER_BUNDLE, GOLD, GOLD_BUNDLE;

private:
    uint32_t                            _chunk;

public:
    cash(uint32_t chunk, listener* listener);
    ~cash();

public:
    virtual const std::string           name_styled() const;

public:
    uint32_t                            chunk() const;
    fb::game::cash*                     chunk(uint32_t value);
    uint32_t                            chunk_reduce(uint32_t value);

    bool                                empty() const;
};


//
// class consume
// 소비아이템
//
class consume : public item
{
#pragma region master
public:
    class master : public fb::game::item::master
    {
    public:
        friend class consume;

    public:
        master(uint32_t id, const std::string& name, uint16_t look, uint8_t color = 0, uint16_t capacity = 1);
        ~master();

    public:
        virtual fb::game::item::attrs   attr() const;
        fb::game::item*                 make(fb::game::item::listener* listener) const;
    };
#pragma endregion

public:
    consume(const master* master, listener* listener, uint16_t count = 1);
    consume(const consume& right);
    ~consume();

public:
    bool                                active();
};


//
// class pack
// 묶음 아이템 (동동주 같은 아이템)
//
class pack : public item
{
#pragma region master
public:
    class master : public fb::game::item::master
    {
    private:
        uint16_t                        _durability;

        friend class pack;

    public:
        master(uint32_t id, const std::string& name, uint16_t look, uint8_t color = 0, uint16_t durability = 200);
        ~master();

    public:
        virtual fb::game::item::attrs   attr() const;
        virtual fb::game::item*         make(fb::game::item::listener* listener) const;
    };
#pragma endregion

private:
    uint16_t                            _durability;

public:
    pack(const master* master, listener* listener);
    pack(const pack& right);
    ~pack();

public:
    std::optional<uint16_t>             durability() const;
    void                                durability(std::optional<uint16_t> value);
    uint16_t                            base_durability() const;

public:
    const std::string                   name_styled() const;
    bool                                active();
};


//
// class equipment
// 장비
//
class equipment : public item
{
#pragma region enum
public:
    enum slot : uint8_t
    {
        UNKNOWN_SLOT                    = 0,
        WEAPON_SLOT                     = 1,
        ARMOR_SLOT                      = 2,
        SHIELD_SLOT                     = 3,
        HELMET_SLOT                     = 4,
        LEFT_HAND_SLOT                  = 7,
        RIGHT_HAND_SLOT                 = 8,
        LEFT_AUX_SLOT                   = 20,
        RIGHT_AUX_SLOT                  = 21,
    };
#pragma endregion

#pragma region listener
public:
interface listener : public virtual fb::game::item::listener
{
    virtual void on_equipment_on(session& me, item& item, equipment::slot slot) = 0;
    virtual void on_equipment_off(session& me, equipment::slot slot, uint8_t index) = 0;
};
#pragma endregion

public:
    DECLARE_EXCEPTION(not_equipment_exception, "입을 수 없는 물건입니다.")

public:
    enum EQUIPMENT_POSITION : uint8_t { EQUIPMENT_LEFT = 0, EQUIPMENT_RIGHT = 1, };


#pragma region inner structure
public:
    typedef struct _repair
    {
    private:
        bool                            _enabled;
        float                           _price;

        friend class equipment;

    public:
        explicit _repair(bool enabled = true, float price = 0) : _enabled(enabled), _price(price) {}

    public:
        bool                            enabled() const { return this->_enabled; }
        void                            enabled(bool value) { this->_enabled = value; }

        float                           price() const { return this->_price; }
        void                            price(float value) { this->_price = value; }
    } repair;

    typedef struct _rename
    {
    private:
        bool                            _enabled;
        uint32_t                        _price;

        friend class equipment;

    public:
        explicit _rename(bool enabled = true, uint32_t price = 0) : _enabled(enabled), _price(price) {}

    public:
        bool                            enabled() const { return this->_enabled; }
        void                            enabled(bool value) { this->_enabled = value; }

        uint32_t                        price() const { return this->_price; }
        void                            price(uint32_t value) { this->_price = value; }
    } rename;
#pragma endregion

#pragma region master
public:
    class master : public fb::game::item::master
    {
    protected:
        uint16_t                        _dress;
        uint16_t                        _durability;
        fb::game::equipment::repair     _repair;
        fb::game::equipment::rename     _rename;
        std::string                     _dress_script, _undress_script;

        uint8_t                         _hit, _damage;
        uint8_t                         _strength, _intelligence, _dexteritry;
        uint32_t                        _base_hp, _base_mp;
        float                           _hp_percentage, _mp_percentage;
        uint8_t                         _healing_cycle;
        defensive                       _defensive;

        friend class equipment;

    public:
        master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color = 0, uint16_t durability = 100);
        ~master();

    public:
        virtual fb::game::item::attrs   attr() const;
        virtual fb::game::item*         make(fb::game::item::listener* listener) const;

    public:
        uint16_t                        dress() const;

        std::optional<uint16_t>         durability() const;
        void                            durability(uint16_t value);

        bool                            repair_enabled() const;
        void                            repair_enabled(bool value);

        float                           repair_price() const;
        void                            repair_price(float value);

        bool                            rename_enabled() const;
        void                            rename_enabled(bool value);

        uint32_t                        rename_price() const;
        void                            rename_price(uint32_t value);

        int16_t                         defensive_physical() const;
        void                            defensive_physical(int16_t value);

        int16_t                         defensive_magical() const;
        void                            defensive_magical(int16_t value);


        uint8_t                         hit() const;
        void                            hit(uint8_t value);

        uint8_t                         damage() const;
        void                            damage(uint8_t value);

        uint8_t                         strength() const;
        void                            strength(uint8_t value);

        uint8_t                         intelligence() const;
        void                            intelligence(uint8_t value);

        uint8_t                         dexteritry() const;
        void                            dexteritry(uint8_t value);

        uint32_t                        base_hp() const;
        void                            base_hp(uint32_t value);

        uint32_t                        base_mp() const;
        void                            base_mp(uint32_t value);

        float                           hp_percentage() const;
        void                            hp_percentage(float value);

        float                           mp_percentage() const;
        void                            mp_percentage(float value);

        uint8_t                         healing_cycle() const;
        void                            healing_cycle(uint8_t value);

        const std::string&              dress_script() const;
        void                            dress_script(const std::string& value);

        const std::string&              undress_script() const;
        void                            undress_script(const std::string& value);

    };
#pragma endregion

protected:
    uint16_t                            _durability;

public:
    equipment(const fb::game::equipment::master* master, listener* listener);
    equipment(const fb::game::equipment& right);
    virtual ~equipment();

public:
    const std::string                   name_trade() const;
    bool                                active();

public:
    uint16_t                            dress() const;

    std::optional<uint16_t>             durability() const;
    void                                durability(std::optional<uint16_t> value);
    uint16_t                            durability_base() const;

    bool                                repair_enabled() const;
    float                               repair_price() const;

    bool                                rename_enabled() const;
    uint32_t                            rename_price() const;

    int16_t                             defensive_physical() const;
    int16_t                             defensive_magical() const;


    uint8_t                             hit() const;
    uint8_t                             damage() const;
    
    uint8_t                             strength() const;
    uint8_t                             intelligence() const;
    uint8_t                             dexteritry() const;

    uint32_t                            base_hp() const;
    uint32_t                            base_mp() const;

    float                               hp_percentage() const;
    float                               mp_percentage() const;

    uint8_t                             healing_cycle() const;


protected:
    virtual std::string                 mid_message() const;

public:
    virtual std::string                 tip_message() const;

public:
    static const std::string            column(equipment::slot slot);
};


//
// class weapon
// 무기
//
class weapon : public equipment
{
#pragma region structure
public:
    typedef struct _damage_range
    {
    public:
        range32_t                       small, large;

    public:
        _damage_range(const range32_t& small, const range32_t& large) : small(small), large(large) {}
    } damage_range;
#pragma endregion

#pragma region enum
public:
    enum types : uint8_t { NORMAL, SPEAR, BOW, FAN, UNKNOWN };
#pragma endregion

#pragma region master
public:
    class master : public equipment::master
    {
    private:
        damage_range                    _damage_range;
        uint16_t                        _sound;
        std::string                     _spell;

        friend class fb::game::weapon;

    public:
        master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color);
        master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color, const range32_t& small, const range32_t& large);
        ~master();

    public:
        virtual fb::game::item::attrs   attr() const;
        virtual fb::game::item*         make(fb::game::item::listener* listener) const;
        fb::game::weapon::types         weapon_type() const;

        const range32_t&                damage_small() const;
        void                            damage_small(uint32_t min, uint32_t max);
        const range32_t&                damage_large() const;
        void                            damage_large(uint32_t min, uint32_t max);

        uint16_t                        sound() const;
        void                            sound(uint16_t value);

        const std::string&              spell() const;
        void                            spell(const std::string& value);

    };
#pragma endregion master


public:
    weapon(const fb::game::weapon::master* master, listener* listener);
    weapon(const fb::game::weapon& right);
    ~weapon();

public:
    fb::game::weapon::types             weapon_type() const;
    const range32_t&                    damage_small() const;
    const range32_t&                    damage_large() const;
    uint16_t                            sound() const;
    const std::string&                  spell() const;

protected:
    std::string                         mid_message() const;
};


//
// class armor
// 갑옷
//
class armor : public equipment
{
#pragma region master
public:
    class master : public equipment::master
    {
    public:
        master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color = 0);
        ~master();

    public:
        virtual fb::game::item::attrs   attr() const;
        virtual fb::game::item*         make(fb::game::item::listener* listener) const;
    };

public:
    armor(const fb::game::armor::master* master, listener* listener);
    armor(const fb::game::armor& right);
    ~armor();
#pragma endregion
};


//
// helmet
// 투구
//
class helmet : public equipment
{
#pragma region
public:
    class master : public equipment::master
    {
    public:
        master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color = 0);
        ~master();

    public:
        virtual fb::game::item::attrs   attr() const;
        virtual fb::game::item*         make(fb::game::item::listener* listener) const;
    };

public:
    helmet(const master* master, listener* listener);
    helmet(const helmet& right);
    ~helmet();
#pragma endregion
};


//
// class shield
// 방패
//
class shield : public equipment
{
#pragma region master
public:
    class master : public equipment::master
    {
    public:
        master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color = 0);
        ~master();

    public:
        virtual fb::game::item::attrs   attr() const;
        virtual fb::game::item*         make(fb::game::item::listener* listener) const;
    };
#pragma endregion

public:
    shield(const master* master, listener* listener);
    shield(const shield& right);
    ~shield();
};


//
// class ring
// 반지
//
class ring : public equipment
{
#pragma region master
public:
    class master : public equipment::master
    {
    public:
        master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color = 0);
        ~master();

    public:
        virtual fb::game::item::attrs   attr() const;
        virtual fb::game::item*         make(fb::game::item::listener* listener) const;
    };
#pragma endregion

public:
    ring(const master* master, listener* listener);
    ring(const ring& right);
    ~ring();
};


//
// class auxilliary
// 보조
//
class auxiliary : public equipment
{
#pragma region master
public:
    class master : public equipment::master
    {
    public:
        master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color = 0);
        ~master();

    public:
        virtual fb::game::item::attrs   attr() const;
        virtual fb::game::item*         make(fb::game::item::listener* listener) const;
    };
#pragma endregion

public:
    auxiliary(const master* master, listener* listener);
    auxiliary(const auxiliary& right);
    ~auxiliary();
};


//
// class arrow
// 화살
//
class arrow : public equipment
{
#pragma region master
public:
    class master : public equipment::master
    {
    public:
        master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color = 0);
        ~master();

    public:
        virtual fb::game::item::attrs   attr() const;
        virtual fb::game::item*         make(fb::game::item::listener* listener) const;
    };
#pragma endregion

public:
    arrow(const master* master, listener* listener);
    arrow(const arrow& right);
    ~arrow();
};




class itemmix
{
#pragma region builder
public:
    class builder : private std::vector<item*>
    {
    private:
        session&                        _owner;

    public:
        builder(session& owner);
        ~builder();

    public:
        builder&                        push(uint8_t index);
        bool                            mix();
    };

#pragma endregion

public:
    DECLARE_EXCEPTION(no_match_exception, "조합할 수 없습니다.")

private:
    typedef struct _element
    {
    public:
        fb::game::item::master*         item;       // 재료 아이템
        uint32_t                        count;      // 갯수

    public:
        _element(fb::game::item::master* item, uint32_t count) : item(item), count(count) {}
        _element(const _element& right) : item(right.item), count(right.count) {}
    } element;

public:
    std::vector<element>                require;   // 재료 아이템
    std::vector<element>                success;   // 성공시 얻는 아이템
    std::vector<element>                failed;    // 실패시 얻는 아이템
    float percentage;

public:
    itemmix(float percentage = 100.0f) : percentage(percentage) {}
    itemmix(const class itemmix& right) : 
        require(right.require.begin(), right.require.end()),
        success(right.success.begin(), right.success.end()),
        failed(right.failed.begin(), right.failed.end()),
        percentage(right.percentage)
    {}

private:
    bool contains(const item* item) const;

public:
    void                                require_add(fb::game::item::master* item, uint32_t count);
    void                                success_add(fb::game::item::master* item, uint32_t count);
    void                                failed_add(fb::game::item::master* item, uint32_t count);
    bool                                matched(const std::vector<item*>& items) const;
};



class items : public fb::game::base_container<fb::game::item>
{
private:
    fb::game::session&                  _owner;

private:
    fb::game::weapon*                   _weapon;
    fb::game::armor*                    _armor;
    fb::game::helmet*                   _helmet;
    fb::game::shield*                   _shield;
    fb::game::ring*                     _rings[2];
    fb::game::auxiliary*                _auxiliaries[2];

public:
    items(session& owner);
    ~items();

private:
    uint8_t                             equipment_off(fb::game::equipment::slot slot);

public:
    uint8_t                             add(fb::game::item& item);
    uint8_t                             add(fb::game::item* item);
    std::vector<uint8_t>                add(const std::vector<fb::game::item*>& items);
    uint8_t                             add(fb::game::item& item, uint8_t index);
    bool                                reduce(uint8_t index, uint16_t count);
    fb::game::item*                     active(uint8_t index);
    uint8_t                             inactive(equipment::slot slot);
    uint8_t                             index(const fb::game::item::master* item) const;
    uint8_t                             index(const fb::game::item& item) const;

    fb::game::equipment*                wear(fb::game::equipment::slot slot, fb::game::equipment* item);

    fb::game::weapon*                   weapon() const;
    fb::game::weapon*                   weapon(fb::game::weapon* weapon);

    fb::game::armor*                    armor() const;
    fb::game::armor*                    armor(fb::game::armor* armor);

    fb::game::shield*                   shield() const;
    fb::game::shield*                   shield(fb::game::shield* shield);

    fb::game::helmet*                   helmet() const;
    fb::game::helmet*                   helmet(fb::game::helmet* helmet);

    fb::game::ring*                     ring(equipment::EQUIPMENT_POSITION position) const;
    fb::game::ring*                     ring(fb::game::ring* ring);
    fb::game::ring*                     ring(fb::game::ring* ring, equipment::EQUIPMENT_POSITION position);

    fb::game::auxiliary*                auxiliary(equipment::EQUIPMENT_POSITION position) const;
    fb::game::auxiliary*                auxiliary(fb::game::auxiliary* auxiliary);
    fb::game::auxiliary*                auxiliary(fb::game::auxiliary* auxiliary, equipment::EQUIPMENT_POSITION position);

    fb::game::item*                     find(const std::string& name) const;
    fb::game::item*                     find(const fb::game::item::master& base) const;
    fb::game::item*                     drop(uint8_t index, uint8_t count);
    void                                pickup(bool boost);
    bool                                throws(uint8_t index);
    
    // override
    fb::game::item*                     remove(uint8_t index, uint16_t copunt = 1, item::delete_attr attr = item::delete_attr::DELETE_NONE);
    fb::game::item*                     remove(fb::game::item& item, uint16_t count = 1, item::delete_attr attr = item::delete_attr::DELETE_NONE);
    bool                                swap(uint8_t src, uint8_t dst);

    std::map<equipment::slot, item*>    equipments() const;
};


} }

#endif // !__ITEM_H__
