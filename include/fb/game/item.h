#ifndef __ITEM_H__
#define __ITEM_H__

#include <iomanip>
#include <fb/game/object.h>

#ifdef small
#undef small
#endif


namespace fb { namespace game {

class session;
class items;

class item : public object
{
public:
    using model_type = fb::model::item;

public:
    friend class items;

public:
    interface listener;
    struct conditions;

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
    item(fb::game::context& context, const fb::model::item& model, const fb::game::item::config& config = fb::game::item::config { .count = 1 });
    item(const item& right);
    virtual ~item();


public:
    virtual std::string                     tip_message() const;

public:
    virtual std::optional<uint32_t>         durability() const;
    virtual void                            durability(uint32_t value);



public:
    virtual std::string                     detailed_name() const;
    uint16_t                                fill(uint16_t count);
    uint16_t                                free_space() const;
    uint16_t                                count() const;
    void                                    count(uint16_t value);
    virtual bool                            empty() const;

public:
    fb::game::session*                      owner() const;
    void                                    owner(fb::game::session* owner);


public:
    virtual bool                            active();
    virtual item*                           split(uint16_t count = 1);
    virtual void                            merge(fb::game::item& item);

public:
    static int                              builtin_model(lua_State* lua);
    static int                              builtin_count(lua_State* lua);
    static int                              builtin_durability(lua_State* lua);
    static int                              builtin_rename(lua_State* lua);
};


interface item::listener : public virtual fb::game::object::listener
{
    virtual void on_item_remove(session& me, uint8_t index, ITEM_DELETE_TYPE attr = ITEM_DELETE_TYPE::NONE) = 0;
    virtual void on_item_update(session& me, uint8_t index) = 0;
    virtual void on_item_swap(session& me, uint8_t src, uint8_t dst) = 0;
    virtual void on_item_active(session& me, item& item) = 0;
    virtual void on_item_throws(session& me, item& item, const point16_t& to) = 0;
};


class cash : public item
{
public:
    const uint32_t                      value = 0;

public:
    cash(fb::game::context& context, uint32_t chunk);
    ~cash();

private:
    static const fb::model::cash&       match_model(fb::game::context& context, uint32_t value);

public:
    fb::task<fb::game::cash*>           replace(uint32_t value);
    uint32_t                            reduce(uint32_t value);

    bool                                empty() const;
    std::string                         detailed_name() const override;
};


class consume : public item
{
public:
    consume(fb::game::context& context, const fb::model::consume& model, uint16_t count = 1);
    consume(const consume& right);
    ~consume();

public:
    bool                                active();
};


class pack : public item
{
private:
    uint32_t                            _durability = 0;

public:
    pack(fb::game::context& context, const fb::model::pack& model);
    pack(const pack& right);
    ~pack();

public:
    std::optional<uint32_t>             durability() const;
    void                                durability(uint32_t value);
    std::string                         detailed_name() const override;

public:
    bool                                active() final;
};


class equipment : public item
{
public:
    interface listener;

public:
    DECLARE_EXCEPTION(not_equipment_exception, "입을 수 없는 물건입니다.")
    
protected:
    uint32_t                            _durability = 0;

protected:
    equipment(fb::game::context& context, const fb::model::equipment& model);
    equipment(const fb::game::equipment& right);
public:
    virtual ~equipment();

public:
    bool                                active();

public:
    std::optional<uint32_t>             durability() const;
    void                                durability(uint32_t value);


protected:
    virtual std::string                 mid_message() const;

public:
    virtual std::string                 tip_message() const;
    virtual std::string                 detailed_name() const override;

public:
    static const std::string            column(EQUIPMENT_PARTS parts);
};


interface equipment::listener : public virtual fb::game::item::listener
{
    virtual void on_equipment_on(session& me, item& item, EQUIPMENT_PARTS parts) = 0;
    virtual void on_equipment_off(session& me, EQUIPMENT_PARTS parts, uint8_t index) = 0;
};


class weapon : public equipment
{
public:
    enum class types : uint8_t { NORMAL, SPEAR, BOW, FAN, UNKNOWN };

private:
    std::optional<std::string>          _custom_name;

public:
    weapon(fb::game::context& context, const fb::model::weapon& model);
    weapon(const fb::game::weapon& right);
    ~weapon();

protected:
    std::string                         mid_message() const final;

public:
    std::string                         detailed_name() const override;
    const std::optional<std::string>&   custom_name() const;
    void                                custom_name(const std::string& name);
    void                                reset_custom_name();
};

class armor : public equipment
{
public:
    armor(fb::game::context& context, const fb::model::armor& model);
    armor(const fb::game::armor& right);
    ~armor();
};


class helmet : public equipment
{
public:
    helmet(fb::game::context& context, const fb::model::helmet& model);
    helmet(const helmet& right);
    ~helmet();
};


class shield : public equipment
{
public:
    shield(fb::game::context& context, const fb::model::shield& model);
    shield(const shield& right);
    ~shield();
};


class ring : public equipment
{
public:
    ring(fb::game::context& context, const fb::model::ring& model);
    ring(const ring& right);
    ~ring();
};


class auxiliary : public equipment
{
public:
    auxiliary(fb::game::context& context, const fb::model::auxiliary& model);
    auxiliary(const auxiliary& right);
    ~auxiliary();
};


class bow : public equipment
{
public:
    bow(fb::game::context& context, const fb::model::bow& model);
    bow(const bow& right);
    ~bow();
};


class items : public fb::game::inventory<fb::game::item>
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
    fb::task< uint8_t>                  equipment_off(EQUIPMENT_PARTS parts);

public:
    fb::task<uint8_t>                   add(fb::game::item& item);
    fb::task<uint8_t>                   add(fb::game::item* item);
    fb::task<std::vector<uint8_t>>      add(const std::vector<fb::game::item*>& items);
    fb::task<uint8_t>                   add(fb::game::item& item, uint8_t index);
    fb::task<fb::game::item*>           active(uint8_t index);
    fb::task<uint8_t>                   inactive(EQUIPMENT_PARTS parts);
    uint8_t                             index(const fb::model::item& item) const;
    uint8_t                             index(const fb::game::item& item) const;
    std::vector<uint8_t>                index_all(const fb::model::item& item) const;

    fb::game::equipment*                wear(EQUIPMENT_PARTS parts, fb::game::equipment* item);

    fb::game::weapon*                   weapon() const;
    fb::game::weapon*                   weapon(fb::game::weapon* weapon);

    fb::game::armor*                    armor() const;
    fb::game::armor*                    armor(fb::game::armor* armor);

    fb::game::shield*                   shield() const;
    fb::game::shield*                   shield(fb::game::shield* shield);

    fb::game::helmet*                   helmet() const;
    fb::game::helmet*                   helmet(fb::game::helmet* helmet);

    fb::game::ring*                     ring(EQUIPMENT_POSITION position) const;
    fb::game::ring*                     ring(fb::game::ring* ring);
    fb::game::ring*                     ring(fb::game::ring* ring, EQUIPMENT_POSITION position);

    fb::game::auxiliary*                auxiliary(EQUIPMENT_POSITION position) const;
    fb::game::auxiliary*                auxiliary(fb::game::auxiliary* auxiliary);
    fb::game::auxiliary*                auxiliary(fb::game::auxiliary* auxiliary, EQUIPMENT_POSITION position);

    fb::game::item*                     find(const std::string& name) const;
    fb::game::item*                     find(const fb::model::item& model) const;
    fb::game::item*                     find_bundle(const fb::model::item& model) const;
    fb::task<fb::game::item*>           drop(uint8_t index, uint8_t count);
    fb::task<void>                      pickup(bool boost);
    fb::task<bool>                      throws(uint8_t index);

    fb::game::item*                     remove(uint8_t index, uint16_t count = 1, ITEM_DELETE_TYPE attr = ITEM_DELETE_TYPE::NONE);
    fb::game::item*                     remove(fb::game::item& item, uint16_t count = 1, ITEM_DELETE_TYPE attr = ITEM_DELETE_TYPE::NONE);

    std::map<EQUIPMENT_PARTS, item*>    equipments() const;
    
    bool                                swap(uint8_t src, uint8_t dst) override;
};

} }

#endif // !__ITEM_H__
