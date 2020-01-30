#ifndef __ITEM_H__
#define __ITEM_H__

#include <string>
#include <sstream>
#include <iomanip>
#include "object.h"

#ifdef small
#undef small
#endif

namespace fb { namespace game {

class session;

//
// class item
// 일반 아이템
//
class item : public object
{
public:
    enum penalties { NONE, DROP, DESTRUCTION };

    enum delete_attr : uint8_t
    {
        DELETE_REMOVED              = 0x00,
        DELETE_DROP                 = 0x01,
        DELETE_EAT                  = 0x02,
        DELETE_SMOKE                = 0x03,
        DELETE_THROW                = 0x04,
        DELETE_SHOOT                = 0x05,
        DELETE_REDUCE               = 0x06,
        DELETE_STICK                = 0x07,
        DELETE_DECAY                = 0x08,
        DELETE_GIVE                 = 0x09,
        DELETE_SELL                 = 0x0A,
        DELETE_NONE                 = 0x0C,
        DELETE_DESTROY              = 0x0D,
    };

    enum attrs : uint32_t
    {
        ITEM_ATTR_NONE              = 0x00000000,
        ITEM_ATTR_EQUIPMENT         = 0x00000001,
        ITEM_ATTR_WEAPON            = 0x00000002,
        ITEM_ATTR_ARMOR             = 0x00000004,
        ITEM_ATTR_SHIELD            = 0x00000008,
        ITEM_ATTR_HELMET            = 0x00000010,
        ITEM_ATTR_RING              = 0x00000020,
        ITEM_ATTR_AUXILIARY         = 0x00000040,
        ITEM_ATTR_CONSUME           = 0x00000080,
        ITEM_ATTR_PACK              = 0x00000100,
        ITEM_ATTR_BUNDLE            = 0x00000200,
        ITEM_ATTR_SCRIPT            = 0x00000400,
        ITEM_ATTR_CASH              = 0x00000800,
    };

    enum eq_slots : uint8_t
    {
        UNKNOWN_SLOT                = 0,
        WEAPON_SLOT                 = 1,
        ARMOR_SLOT                  = 2,
        SHIELD_SLOT                 = 3,
        HELMET_SLOT                 = 4,
        LEFT_HAND_SLOT              = 7,
        RIGHT_HAND_SLOT             = 8,
        LEFT_AUX_SLOT               = 20,
        RIGHT_AUX_SLOT              = 21,
    };

    typedef struct _limit
    {
        uint8_t                     level;
        uint8_t                     strength;
        uint8_t                     dexteritry;
        uint8_t                     intelligence;
        uint8_t                     cls, promotion;
        fb::game::sex               sex;

    public:
        _limit();
        _limit(uint16_t level, uint16_t strength, uint16_t dexteritry, uint16_t intelligence, uint16_t cls, uint16_t promotion, fb::game::sex sex);
    } item_limit;

    static const item_limit         DEFAULT_LIMIT;
    static const uint32_t           MAX_ITEM_SLOT = 52;

public:
    class core : public fb::game::object::core
    {
    protected:
        uint32_t                    _price;
        item_limit                  _limit;
        penalties                   _penalty;
        uint16_t                    _capacity;
        bool                        _trade;
        bool                        _bundle;
        uint32_t                    _entrust;
        std::string                 _tooltip, _desc;
        std::string                 _active_script;

        friend class fb::game::item;

    public:
        core(uint32_t id, const std::string& name, uint16_t look, uint8_t color = 0, uint16_t capacity = 1, const item_limit& limit = DEFAULT_LIMIT);
        core(const fb::game::object::core& core);
        virtual ~core();

    public:
        uint32_t                    price() const;
        void                        price(uint32_t value);

        uint16_t                    capacity() const;
        void                        capacity(uint16_t value);

        bool                        trade() const;
        void                        trade(bool value);

        uint32_t                    entrust() const;
        void                        entrust(uint32_t value);

        const item_limit&           limit() const;
        void                        limit(const item::item_limit& value);

        penalties                   penalty() const;
        void                        penalty(penalties value);

        const std::string&          desc() const;
        void                        desc(const std::string& value);

        const std::string&          active_script() const;
        void                        active_script(const std::string& value);

        virtual attrs               attr() const;
        virtual object*             make() const;
    };

protected:
    uint16_t                        _count;

public:
    item(const fb::game::item::core* core);
    item(const item& right);
    virtual ~item();

public:
    virtual const std::string       name_styled() const;
    virtual const std::string       name_trade() const;

protected:
    virtual std::string             tip_message() const;

public:
    uint16_t                        fill(uint16_t count);
    uint16_t                        reduce(uint16_t count);
    uint16_t                        free_space() const;
    uint16_t                        count() const;
    void                            count(uint16_t value);
    virtual bool                    empty() const;

public:
    uint32_t                        price() const;
    uint16_t                        capacity() const;
    bool                            trade() const;
    uint32_t                        entrust() const;
    const item_limit&               limit() const;
    penalties                       penalty() const;
    const std::string&              desc() const;
    const std::string&              active_script() const;
    object::types                   type() const { return object::ITEM; }
    attrs                           attr() const;

public:
    fb::ostream                     make_tip_stream(uint16_t position);

public:
    virtual bool                    handle_acive(session& session) { return false; }
    virtual item*                   handle_drop(object& owner, uint16_t count = 1);
};

//
// class cash
// 게임머니
//
class cash : public item
{
public:
    class core : public fb::game::item::core
    {
    public:
        core(uint32_t id, const std::string& name, uint16_t look, uint8_t color = 0);
        ~core();

    public:
        virtual attrs               attr() const;
        virtual object*             make() const;
    };

public:
    static const core               BRONZE, BRONZE_BUNDLE, SILVER, SILVER_BUNDLE, GOLD, GOLD_BUNDLE;

private:
    uint32_t                        _chunk;

public:
    cash(uint32_t chunk);
    ~cash();

public:
    virtual const std::string       name_styled() const;

public:
    uint32_t                        chunk() const;
    void                            chunk(uint32_t value);
    uint32_t                        chunk_reduce(uint32_t value);

    bool                            empty() const;
};


//
// class consume
// 소비아이템
//
class consume : public item
{
public:
    class core : public fb::game::item::core
    {
    public:
        friend class consume;

    public:
        core(uint32_t id, const std::string& name, uint16_t look, uint8_t color = 0, uint16_t capacity = 1);
        ~core();

    public:
        virtual attrs               attr() const;
        virtual object*             make() const;
    };

public:
    consume(const core* core);
    consume(const consume& right);
    ~consume();

public:
    bool                            handle_acive(session& session);
};


//
// class pack
// 묶음 아이템 (동동주 같은 아이템)
//
class pack : public item
{
public:
    class core : public fb::game::item::core
    {
    private:
        uint16_t                    _durability;

        friend class pack;

    public:
        core(uint32_t id, const std::string& name, uint16_t look, uint8_t color = 0, uint16_t durability = 200);
        ~core();

    public:
        virtual attrs               attr() const;
        virtual object*             make() const;
    };

private:
    uint16_t                        _durability;

public:
    pack(const core* core);
    pack(const pack& right);
    ~pack();

public:
    uint16_t                        durability() const;
    uint16_t                        base_durability() const;

public:
    const std::string               name_styled() const;
    bool                            handle_acive(session& session);
};


//
// class equipment
// 장비
//
class equipment : public item
{
public:
    enum EQUIPMENT_POSITION : uint8_t { EQUIPMENT_LEFT = 0, EQUIPMENT_RIGHT = 1, };

public:
    class core : public fb::game::item::core
    {
    protected:
        uint16_t                    _dress;
        uint16_t                    _durability;
        bool                        _repairable;
        uint32_t                    _repair_price;
        uint32_t                    _rename_price;
        std::string                 _inactive_script;

        uint8_t                     _hit, _damage;
        uint8_t                     _strength, _intelligence, _dexteritry;
        uint32_t                    _base_hp, _base_mp;
        float                       _hp_percentage, _mp_percentage;
        uint8_t                     _healing_cycle;
        defensive                   _defensive;

        friend class equipment;

    public:
        core(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color = 0, uint16_t durability = 100);
        ~core();

    public:
        virtual attrs               attr() const;
        virtual object*             make() const;

    public:
        uint16_t                    dress() const;

        uint16_t                    durability() const;
        void                        durability(uint16_t value);

        bool                        repairable() const;
        void                        repairable(bool value);

        uint32_t                    repair_price() const;
        void                        repair_price(uint32_t value);

        uint32_t                    rename_price() const;
        void                        rename_price(uint32_t value);

        int16_t                     defensive_physical() const;
        void                        defensive_physical(int16_t value);

        int16_t                     defensive_magical() const;
        void                        defensive_magical(int16_t value);


        uint8_t                     hit() const;
        void                        hit(uint8_t value);

        uint8_t                     damage() const;
        void                        damage(uint8_t value);

        uint8_t                     strength() const;
        void                        strength(uint8_t value);

        uint8_t                     intelligence() const;
        void                        intelligence(uint8_t value);

        uint8_t                     dexteritry() const;
        void                        dexteritry(uint8_t value);

        uint32_t                    base_hp() const;
        void                        base_hp(uint32_t value);

        uint32_t                    base_mp() const;
        void                        base_mp(uint32_t value);

        float                       hp_percentage() const;
        void                        hp_percentage(float value);

        float                       mp_percentage() const;
        void                        mp_percentage(float value);

        uint8_t                     healing_cycle() const;
        void                        healing_cycle(uint8_t value);

    };

protected:
    uint16_t                        _durability;

public:
    equipment(const equipment::core* core);
    equipment(const equipment& right);
    virtual ~equipment();

public:
    const std::string               name_trade() const;

public:
    uint16_t                        dress() const;

    uint16_t                        durability() const;
    uint16_t                        durability_base() const;

    bool                            repairable() const;
    uint32_t                        repair_price() const;
    uint32_t                        rename_price() const;

    int16_t                         defensive_physical() const;
    int16_t                         defensive_magical() const;


    uint8_t                         hit() const;
    uint8_t                         damage() const;
    
    uint8_t                         strength() const;
    uint8_t                         intelligence() const;
    uint8_t                         dexteritry() const;

    uint32_t                        base_hp() const;
    uint32_t                        base_mp() const;

    float                           hp_percentage() const;
    float                           mp_percentage() const;

    uint8_t                         healing_cycle() const;


protected:
    virtual std::string             mid_message() const;
    virtual std::string             tip_message() const;
};


//
// class weapon
// 무기
//
class weapon : public equipment
{
public:
    typedef struct _damage_range
    {
    public:
        range32_t                   small, large;

    public:
        _damage_range(const range32_t& small, const range32_t& large) : small(small), large(large) {}
    } damage_range;

    
    enum types : uint8_t { NORMAL, SPEAR, BOW, FAN, UNKNOWN };

public:
    class core : public equipment::core
    {
    private:
        damage_range                _damage_range;
        uint16_t                    _sound;
        std::string                 _spell;

        friend class fb::game::weapon;

    public:
        core(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color);
        core(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color, const range32_t& small, const range32_t& large);
        ~core();

    public:
        virtual attrs               attr() const;
        virtual object*             make() const;
        weapon::types               weapon_type() const;

        const range32_t&            damage_small() const;
        void                        damage_small(uint32_t min, uint32_t max);
        const range32_t&            damage_large() const;
        void                        damage_large(uint32_t min, uint32_t max);

        uint16_t                    sound() const;
        void                        sound(uint16_t value);

        const std::string&          spell() const;
        void                        spell(const std::string& value);

    };


public:
    weapon(const core* core);
    weapon(const weapon& right);
    ~weapon();

public:
    weapon::types                   weapon_type() const;
    const range32_t&                damage_small() const;
    const range32_t&                damage_large() const;
    uint16_t                        sound() const;
    const std::string&              spell() const;

protected:
    std::string                     mid_message() const;
};


//
// class armor
// 갑옷
//
class armor : public equipment
{
public:
    class core : public equipment::core
    {
    public:
        core(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color = 0);
        ~core();

    public:
        virtual attrs               attr() const;
        virtual object*             make() const;
    };

public:
    armor(const core* core);
    armor(const armor& right);
    ~armor();
};


//
// helmet
// 투구
//
class helmet : public equipment
{
public:
    class core : public equipment::core
    {
    public:
        core(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color = 0);
        ~core();

    public:
        virtual attrs               attr() const;
        virtual object*             make() const;
    };

public:
    helmet(const core* core);
    helmet(const helmet& right);
    ~helmet();
};


//
// class shield
// 방패
//
class shield : public equipment
{
public:
    class core : public equipment::core
    {
    public:
        core(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color = 0);
        ~core();

    public:
        virtual attrs               attr() const;
        virtual object*             make() const;
    };

public:
    shield(const core* core);
    shield(const shield& right);
    ~shield();
};


//
// class ring
// 반지
//
class ring : public equipment
{
public:
    class core : public equipment::core
    {
    public:
        core(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color = 0);
        ~core();

    public:
        virtual attrs               attr() const;
        virtual object*             make() const;
    };

public:
    ring(const core* core);
    ring(const ring& right);
    ~ring();
};


//
// class auxilliary
// 보조
//
class auxiliary : public equipment
{
public:
    class core : public equipment::core
    {
    public:
        core(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color = 0);
        ~core();

    public:
        virtual attrs               attr() const;
        virtual object*             make() const;
    };

public:
    auxiliary(const core* core);
    auxiliary(const auxiliary& right);
    ~auxiliary();
};


//
// class arrow
// 화살
//
class arrow : public equipment
{
public:
    class core : public equipment::core
    {
    public:
        core(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color = 0);
        ~core();

    public:
        virtual attrs               attr() const;
        virtual object*             make() const;
    };

public:
    arrow(const core* core);
    arrow(const arrow& right);
    ~arrow();
};




typedef struct _itemmix
{
private:
    typedef struct _element
    {
    public:
        item::core*                 item;       // 재료 아이템
        uint32_t                    count;      // 갯수

    public:
        _element(fb::game::item::core* item, uint32_t count) : item(item), count(count) {}
        _element(const _element& right) : item(right.item), count(right.count) {}
    } element;

public:
    std::vector<element>            require;   // 재료 아이템
    std::vector<element>            success;   // 성공시 얻는 아이템
    std::vector<element>            failed;    // 실패시 얻는 아이템
    float percentage;

public:
    _itemmix(float percentage = 100.0f) : percentage(percentage) {}
    _itemmix(const struct _itemmix& right) : 
        require(right.require.begin(), right.require.end()),
        success(right.success.begin(), right.success.end()),
        failed(right.failed.begin(), right.failed.end()),
        percentage(right.percentage)
    {}

private:
    bool contains(const item* item) const;

public:
    void                            require_add(fb::game::item::core* item, uint32_t count);
    void                            success_add(fb::game::item::core* item, uint32_t count);
    void                            failed_add(fb::game::item::core* item, uint32_t count);
    bool                            matched(const std::vector<item*>& items) const;
} itemmix;

} }

#endif // !__ITEM_H__
