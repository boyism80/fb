#ifndef __ITEM_H__
#define __ITEM_H__

#include <string>
#include <sstream>
#include <iomanip>
#include <optional>
#include <fb/game/object.h>

#ifdef small
#undef small
#endif


namespace fb { namespace game {

class session;
class items;

class item : public object
{
    friend class items;

public:
    interface listener;

public:
    class model;

public:
    struct trade;
    struct storage;
    struct conditions;

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

public:
    LUA_PROTOTYPE

public:
    struct config : fb::game::object::config
    {
    public:
        uint16_t            count       = 1;
    };

public:
    DECLARE_EXCEPTION(full_inven_exception, "소지품이 꽉 찼습니다.")

    
    static const conditions                 DEFAULT_CONDITION;


protected:
    uint16_t                                _count = 0;
    session*                                _owner = nullptr;

public:
    item(fb::game::context& context, const fb::game::item::model* model, const fb::game::item::config& config = fb::game::item::config { .count = 1 });
    item(const item& right);
    virtual ~item();


public:
    virtual std::string                     tip_message() const;

public:
    virtual const std::string               name_styled() const;
    virtual const std::string               name_trade() const;
    virtual std::optional<uint16_t>         durability() const;
    virtual void                            durability(std::optional<uint16_t> value);



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


public:
    virtual bool                            active();
    virtual item*                           split(uint16_t count = 1);
    virtual void                            merge(fb::game::item& item);

public:
    static int                              builtin_count(lua_State* lua);
};


interface item::listener : public virtual fb::game::object::listener
{
    virtual void on_item_remove(session& me, uint8_t index, item::delete_attr attr = item::delete_attr::NONE) = 0;
    virtual void on_item_update(session& me, uint8_t index) = 0;
    virtual void on_item_swap(session& me, uint8_t src, uint8_t dst) = 0;
    virtual void on_item_active(session& me, item& item) = 0;
    virtual void on_item_throws(session& me, item& item, const point16_t& to) = 0;
};

struct item::trade
{
public:
    friend class item;

public:
    const bool                              enabled = false;

public:
    trade() = default;
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
    storage() = default;
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
};

class item::model : public fb::game::object::model
{
public:
    struct config : public fb::game::object::model::config
    {
    public:
        uint32_t                            id          = 0;
        uint32_t                            price       = 0;
        fb::game::item::conditions          condition;
        fb::game::item::penalties           penalty     = fb::game::item::penalties::NONE;
        uint16_t                            capacity    = 0;
        fb::game::item::trade               trade;
        fb::game::item::storage             storage;
        std::string                         desc;
        std::string                         active_script;
    };

    friend class fb::game::item;

public:
    LUA_PROTOTYPE

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

public:
    model(const fb::game::item::model::config& config);
    virtual ~model();

public:
    fb::game::object::types                  type() const override { return object::types::ITEM; }

public:
    virtual fb::game::item::attrs            attr() const;
    bool                                     attr(fb::game::item::attrs flag) const;

public:
    virtual fb::game::item* make(fb::game::context& context, uint16_t count = 1) const
    {
        return new fb::game::item(context, this, fb::game::item::config { .count = count });
    }

public:
    static int                               builtin_make(lua_State* lua);
    static int                               builtin_attr(lua_State* lua);
    static int                               builtin_capacity(lua_State* lua);
};


class cash : public item
{
public:
    class model;

public:
    static const model                 BRONZE, BRONZE_BUNDLE, SILVER, SILVER_BUNDLE, GOLD, GOLD_BUNDLE;

private:
    uint32_t                            _chunk = 0;

public:
    cash(fb::game::context& context, uint32_t chunk);
    ~cash();

public:
    virtual const std::string           name_styled() const;

public:
    uint32_t                            chunk() const;
    fb::game::cash*                     chunk(uint32_t value);
    uint32_t                            chunk_reduce(uint32_t value);

    bool                                empty() const;
};


class cash::model : public fb::game::item::model
{
public:
    model(const fb::game::item::model::config& config);
    ~model();

public:
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final
    {
        return new fb::game::cash(context, count);
    }

    virtual fb::game::item::attrs       attr() const;
};


class consume : public item
{
public:
    class model;

public:
    consume(fb::game::context& context, const model* model, uint16_t count = 1);
    consume(const consume& right);
    ~consume();

public:
    bool                                active();
};


class consume::model : public fb::game::item::model
{
public:
    friend class consume;

public:
    model(const fb::game::item::model::config& config);
    ~model();

public:
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final
    {
        return new fb::game::consume(context, this, count);
    }
    virtual fb::game::item::attrs       attr() const;
};


class pack : public item
{
public:
    class model;

private:
    uint16_t                            _durability = 0;

public:
    pack(fb::game::context& context, const model* model);
    pack(const pack& right);
    ~pack();

public:
    std::optional<uint16_t>             durability() const;
    void                                durability(std::optional<uint16_t> value);

public:
    const std::string                   name_styled() const final;
    bool                                active() final;
};


class pack::model : public fb::game::item::model
{
public:
    friend class pack;

public:
    uint16_t                                durability = 0;

public:
    model(const fb::game::item::model::config& config);
    ~model();

public:
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final
    {
        return new fb::game::pack(context, this);
    }
    virtual fb::game::item::attrs       attr() const;
};


class equipment : public item
{
public:
    enum class slot : uint8_t
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

    enum class position : uint8_t { LEFT = 0, RIGHT = 1, };

public:
    interface listener;

public:
    class model;

public:
    struct repair;
    struct rename;

public:
    DECLARE_EXCEPTION(not_equipment_exception, "입을 수 없는 물건입니다.")
    
protected:
    uint16_t                            _durability = 0;

protected:
    equipment(fb::game::context& context, const fb::game::equipment::model* model);
    equipment(const fb::game::equipment& right);
public:
    virtual ~equipment();

public:
    const std::string                   name_trade() const;
    bool                                active();

public:
    std::optional<uint16_t>             durability() const;
    void                                durability(std::optional<uint16_t> value);


protected:
    virtual std::string                 mid_message() const;

public:
    virtual std::string                 tip_message() const;

public:
    static const std::string            column(equipment::slot slot);
};


interface equipment::listener : public virtual fb::game::item::listener
{
    virtual void on_equipment_on(session& me, item& item, equipment::slot slot) = 0;
    virtual void on_equipment_off(session& me, equipment::slot slot, uint8_t index) = 0;
};

struct equipment::repair
{
public:
    friend class equipment;

private:
    bool                                _enabled = false;
    float                               _price   = 0.0f;

public:
    repair(bool enabled = true, float price = 0) : _enabled(enabled), _price(price) { }

public:
    bool                                enabled() const { return this->_enabled; }
    void                                enabled(bool value) { this->_enabled = value; }

    float                               price() const { return this->_price; }
    void                                price(float value) { this->_price = value; }
};

struct equipment::rename
{
public:
    friend class equipment;

private:
    bool                                _enabled = false;
    uint32_t                            _price   = 0;

public:
    rename(bool enabled = true, uint32_t price = 0) : _enabled(enabled), _price(price) { }

public:
    bool                                enabled() const { return this->_enabled; }
    void                                enabled(bool value) { this->_enabled = value; }

    uint32_t                            price() const { return this->_price; }
    void                                price(uint32_t value) { this->_price = value; }
};

class equipment::model : public fb::game::item::model
{
public:
    struct config : public fb::game::item::model::config
    {
    public:
        uint16_t                            dress = 0;
        uint16_t                            durability = 0;
        fb::game::equipment::repair         repair;
        fb::game::equipment::rename         rename;
        std::string                         dress_script;
        std::string                         undress_script;
        uint8_t                             hit = 0;
        uint8_t                             damage = 0;
        uint8_t                             strength = 0;
        uint8_t                             intelligence = 0;
        uint8_t                             dexteritry = 0;
        uint32_t                            base_hp = 0;
        uint32_t                            base_mp = 0;
        float                               hp_percentage = 0.0f;
        float                               mp_percentage = 0.0f;
        uint8_t                             healing_cycle = 0;
        fb::game::defensive                 defensive;
    };


public:
    friend class equipment;

public:
    const uint16_t                      dress;
    const uint16_t                      durability;
    const fb::game::equipment::repair   repair;
    const fb::game::equipment::rename   rename;
    const std::string                   dress_script, undress_script;

    const uint8_t                       hit, damage;
    const uint8_t                       strength, intelligence, dexteritry;
    const uint32_t                      base_hp, base_mp;
    const float                         hp_percentage, mp_percentage;
    const uint8_t                       healing_cycle;
    const fb::game::defensive           defensive;

public:
    model(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color = 0, uint16_t durability = 100);
    model(const fb::game::equipment::model::config& config);
    ~model();

public:
    virtual fb::game::item::attrs       attr() const;
};


class weapon : public equipment
{
public:
    struct damage_range;

public:
    class model;

public:
    enum class types : uint8_t { NORMAL, SPEAR, BOW, FAN, UNKNOWN };

public:
    weapon(fb::game::context& context, const fb::game::weapon::model* model);
    weapon(const fb::game::weapon& right);
    ~weapon();

protected:
    std::string                         mid_message() const final;
};


struct weapon::damage_range
{
public:
    range32_t                           small, large;

public:
    damage_range(const range32_t& small, const range32_t& large) : small(small), large(large) { }
};

class weapon::model : public equipment::model
{
public:
    struct config : public fb::game::equipment::model::config
    {
    public:
        range32_t                       small; 
        range32_t                       large;
        uint16_t                        sound;
        std::string                     spell;
    };

public:
    friend class weapon;

public:
    const weapon::damage_range          damage_range;
    const uint16_t                      sound;
    const std::string                   spell;

public:
    model(const fb::game::weapon::model::config& config);
    ~model();

public:
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final
    {
        return new fb::game::weapon(context, this);
    }
    virtual fb::game::item::attrs       attr() const;
    fb::game::weapon::types             weapon_type() const;

};


class armor : public equipment
{
public:
    class model;

public:
    armor(fb::game::context& context, const fb::game::armor::model* model);
    armor(const fb::game::armor& right);
    ~armor();
};


class armor::model : public equipment::model
{
public:
    model(const fb::game::equipment::model::config& config);
    ~model();

public:
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final
    {
        return new fb::game::armor(context, this);
    }
    virtual fb::game::item::attrs       attr() const;
};


class helmet : public equipment
{
public:
    class model;

public:
    helmet(fb::game::context& context, const model* model);
    helmet(const helmet& right);
    ~helmet();
};


class helmet::model : public equipment::model
{
public:
    model(const fb::game::equipment::model::config& config);
    ~model();

public:
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final
    {
        return new fb::game::helmet(context, this);
    }
    virtual fb::game::item::attrs       attr() const;
};


class shield : public equipment
{
public:
    class model;

public:
    shield(fb::game::context& context, const model* model);
    shield(const shield& right);
    ~shield();
};


class shield::model : public equipment::model
{
public:
    model(const fb::game::equipment::model::config& config);
    ~model();
    
public:
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final
    {
        return new fb::game::shield(context, this);
    }
    virtual fb::game::item::attrs       attr() const;
};


class ring : public equipment
{
public:
    class model;

public:
    ring(fb::game::context& context, const model* model);
    ring(const ring& right);
    ~ring();
};


class ring::model : public equipment::model
{
public:
    model(const fb::game::equipment::model::config& config);
    ~model();
    
public:
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final
    {
        return new fb::game::ring(context, this);
    }
    virtual fb::game::item::attrs       attr() const;
};


class auxiliary : public equipment
{
public:
    class model;

public:
    auxiliary(fb::game::context& context, const model* model);
    auxiliary(const auxiliary& right);
    ~auxiliary();
};


class auxiliary::model : public equipment::model
{
public:
    model(const fb::game::equipment::model::config& config);
    ~model();

public:
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final
    {
        return new fb::game::auxiliary(context, this);
    }
    virtual fb::game::item::attrs       attr() const;
};


class bow : public equipment
{
public:
    class model;

public:
    bow(fb::game::context& context, const model* model);
    bow(const bow& right);
    ~bow();
};


class bow::model : public equipment::model
{
public:
    model(const fb::game::equipment::model::config& config);
    ~model();

public:
    fb::game::item* make(fb::game::context& context, uint16_t count = 1) const final
    {
        return new fb::game::bow(context, this);
    }
    virtual fb::game::item::attrs       attr() const;
};


class items : public fb::game::base_container<fb::game::item>
{
private:
    fb::game::session&                  _owner;

private:
    fb::game::weapon*                   _weapon          = nullptr;
    fb::game::armor*                    _armor           = nullptr;
    fb::game::helmet*                   _helmet          = nullptr;
    fb::game::shield*                   _shield          = nullptr;
    fb::game::ring*                     _rings[2]        = { nullptr, nullptr };
    fb::game::auxiliary*                _auxiliaries[2]  = { nullptr, nullptr };

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
    uint8_t                             index(const fb::game::item::model* item) const;
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

    fb::game::ring*                     ring(equipment::position position) const;
    fb::game::ring*                     ring(fb::game::ring* ring);
    fb::game::ring*                     ring(fb::game::ring* ring, equipment::position position);

    fb::game::auxiliary*                auxiliary(equipment::position position) const;
    fb::game::auxiliary*                auxiliary(fb::game::auxiliary* auxiliary);
    fb::game::auxiliary*                auxiliary(fb::game::auxiliary* auxiliary, equipment::position position);

    fb::game::item*                     find(const std::string& name) const;
    fb::game::item*                     find(const fb::game::item::model& base) const;
    fb::game::item*                     drop(uint8_t index, uint8_t count);
    void                                pickup(bool boost);
    bool                                throws(uint8_t index);

    fb::game::item*                     remove(uint8_t index, uint16_t count = 1, item::delete_attr attr = item::delete_attr::NONE);
    fb::game::item*                     remove(fb::game::item& item, uint16_t count = 1, item::delete_attr attr = item::delete_attr::NONE);

    std::map<equipment::slot, item*>    equipments() const;
    
    bool                                swap(uint8_t src, uint8_t dst) override;
};


class itemmix
{
private:
    struct element;

public:
    class builder;

public:
    DECLARE_EXCEPTION(no_match_exception, "조합할 수 없습니다.")

public:
    std::vector<element>                require;   // 재료 아이템
    std::vector<element>                success;   // 성공시 얻는 아이템
    std::vector<element>                failed;    // 실패시 얻는 아이템
    float                               percentage = 0.0f;

public:
    itemmix(float percentage = 100.0f) : percentage(percentage) { }
    itemmix(const class itemmix& right) : 
        require(right.require.begin(), right.require.end()),
        success(right.success.begin(), right.success.end()),
        failed(right.failed.begin(), right.failed.end()),
        percentage(right.percentage)
    { }

private:
    bool                                contains(const item* item) const;

public:
    void                                require_add(fb::game::item::model* item, uint32_t count);
    void                                success_add(fb::game::item::model* item, uint32_t count);
    void                                failed_add(fb::game::item::model* item, uint32_t count);
    bool                                matched(const std::vector<item*>& items) const;
};


class itemmix::builder : private std::vector<fb::game::item*>
{
private:
    session&                            _owner;

public:
    builder(session& owner);
    ~builder();

public:
    builder&                            push(uint8_t index);
    bool                                mix();
};

struct itemmix::element
{
public:
    fb::game::item::model*             item;       // 재료 아이템
    uint32_t                            count;      // 갯수

public:
    element(fb::game::item::model* item, uint32_t count) : item(item), count(count) { }
    element(const element& right) : item(right.item), count(right.count) { }
};

} }

#endif // !__ITEM_H__
