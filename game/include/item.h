#ifndef __ITEM_H__
#define __ITEM_H__

#include <iomanip>
#include <object.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/model/datetime.h>

#ifdef small
#undef small
#endif

namespace fb { namespace game {

/**
 * @brief      This class describes a character.
 */
class character;
/**
 * @brief      This class describes items.
 */
class items;

/**
 * @brief      This class describes an item.
 */
class item : public object
{
public:
    using model_type = fb::model::item;

public:
    friend class items;

public:
    /**
     * @brief      { struct_description }
     */
    struct listener;
    /**
     * @brief      { struct_description }
     */
    struct conditions;

public:
    LUA_PROTOTYPE

public:
    /**
     * @brief      { struct_description }
     */
    struct config : fb::game::object::config
    {
    public:
        uint16_t count = 1;
    };

public:
    DECLARE_EXCEPTION(full_inven_exception, "소지품이 꽉 찼습니다.")

    static const conditions DEFAULT_CONDITION;

public:
    using nullable_time = std::optional<datetime>;

protected:
    uint16_t      _count        = 0;
    character*    _owner        = nullptr;
    nullable_time _dropped_time = std::nullopt;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  model    The model
     * @param[in]  config   The configuration
     */
    item(fb::game::context&            context,
         const fb::model::item&        model,
         const fb::game::item::config& config = fb::game::item::config{.count = 1});
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right
     */
    item(const item& right);
    /**
     * @brief      Destroys the object.
     */
    virtual ~item();

protected:
    /**
     * @brief      Called when map changed.
     *
     * @param      map   The map
     */
    virtual void on_map_changed(fb::game::map* map);

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual std::string tip_message() const;

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual std::optional<uint32_t> durability() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    virtual void durability(uint32_t value);

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual std::string inven_name() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual std::string trade_name() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  count  The count
     *
     * @return     { description_of_the_return_value }
     */
    uint16_t fill(uint16_t count);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint16_t free_space() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint16_t count() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void count(uint16_t value);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual bool empty() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const nullable_time& dropped_time() const;

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::character* owner() const;
    /**
     * @brief      { function_description }
     *
     * @param      owner  The owner
     */
    void owner(fb::game::character* owner);

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual bool active();
    /**
     * @brief      { function_description }
     *
     * @param[in]  count  The count
     *
     * @return     { description_of_the_return_value }
     */
    virtual item* split(uint16_t count = 1);
    /**
     * @brief      { function_description }
     *
     * @param      item  The item
     */
    virtual void merge(fb::game::item& item);

public:
    /**
     * @brief      Returns a protocol representation of the object.
     *
     * @return     Protocol representation of the object.
     */
    virtual fb::protocol::internal::Item to_protocol() const;

public:
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_model(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_count(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_durability(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_rename(lua_State* lua);
};

/**
 * @brief      { struct_description }
 */
struct item::listener : public virtual fb::game::object::listener
{
    /**
     * @brief      Called on item remove.
     *
     * @param      me     { parameter_description }
     * @param[in]  index  The index
     * @param[in]  attr   The attribute
     */
    virtual void on_item_remove(character& me, uint8_t index, ITEM_DELETE_TYPE attr = ITEM_DELETE_TYPE::NONE) = 0;
    /**
     * @brief      Called on item update.
     *
     * @param      me     { parameter_description }
     * @param[in]  index  The index
     */
    virtual void on_item_update(character& me, uint8_t index) = 0;
    /**
     * @brief      Called on item swap.
     *
     * @param      me    { parameter_description }
     * @param[in]  src   The source
     * @param[in]  dst   The destination
     */
    virtual void on_item_swap(character& me, uint8_t src, uint8_t dst) = 0;
    /**
     * @brief      Called on item active.
     *
     * @param      me    { parameter_description }
     * @param      item  The item
     */
    virtual void on_item_active(character& me, item& item) = 0;
    /**
     * @brief      Called on item throws.
     *
     * @param      me    { parameter_description }
     * @param      item  The item
     * @param[in]  to    { parameter_description }
     */
    virtual void on_item_throws(character& me, item& item, const point16_t& to) = 0;
};

/**
 * @brief      This class describes a cash.
 */
class cash : public item
{
public:
    const uint32_t value = 0;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  chunk    The chunk
     */
    cash(fb::game::context& context, uint32_t chunk);
    /**
     * @brief      Destroys the object.
     */
    ~cash();

private:
    /**
     * @brief      { function_description }
     *
     * @param      context  The context
     * @param[in]  value    The value
     *
     * @return     { description_of_the_return_value }
     */
    static const fb::model::cash& match_model(fb::game::context& context, uint32_t value);

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    async::task<fb::game::cash*> replace(uint32_t value);
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t reduce(uint32_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool empty() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    std::string inven_name() const override;
};

/**
 * @brief      This class describes a consume.
 */
class consume : public item
{
public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  model    The model
     * @param[in]  count    The count
     */
    consume(fb::game::context& context, const fb::model::consume& model, uint16_t count = 1);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right
     */
    consume(const consume& right);
    /**
     * @brief      Destroys the object.
     */
    ~consume();

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool active();
};

/**
 * @brief      This class describes a pack.
 */
class pack : public item
{
private:
    uint32_t _durability = 0;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  model    The model
     */
    pack(fb::game::context& context, const fb::model::pack& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right
     */
    pack(const pack& right);
    /**
     * @brief      Destroys the object.
     */
    ~pack();

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    std::optional<uint32_t> durability() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void durability(uint32_t value);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    std::string inven_name() const override;

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool active() final;
};

/**
 * @brief      This class describes an equipment.
 */
class equipment : public item
{
public:
    /**
     * @brief      { struct_description }
     */
    struct listener;

public:
    DECLARE_EXCEPTION(not_equipment_exception, "입을 수 없는 물건입니다.")

protected:
    uint32_t _durability = 0;

protected:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  model    The model
     */
    equipment(fb::game::context& context, const fb::model::equipment& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right
     */
    equipment(const fb::game::equipment& right);

public:
    /**
     * @brief      Destroys the object.
     */
    virtual ~equipment();

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool active();

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    std::optional<uint32_t> durability() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void durability(uint32_t value);

protected:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual std::string mid_message() const;

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual std::string tip_message() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    virtual std::string trade_name() const override;

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  parts  The parts
     *
     * @return     { description_of_the_return_value }
     */
    static const std::string column(EQUIPMENT_PARTS parts);
};

/**
 * @brief      { struct_description }
 */
struct equipment::listener : public virtual fb::game::item::listener
{
    /**
     * @brief      Called on equipment on.
     *
     * @param      me     { parameter_description }
     * @param      item   The item
     * @param[in]  parts  The parts
     */
    virtual void on_equipment_on(character& me, item& item, EQUIPMENT_PARTS parts) = 0;
    /**
     * @brief      Called on equipment off.
     *
     * @param      me     { parameter_description }
     * @param[in]  parts  The parts
     * @param[in]  index  The index
     */
    virtual void on_equipment_off(character& me, EQUIPMENT_PARTS parts, uint8_t index) = 0;
};

/**
 * @brief      This class describes a weapon.
 */
class weapon : public equipment
{
public:
    /**
     * @brief      This class describes types.
     */
    enum class types : uint8_t
    {
        NORMAL,
        SPEAR,
        BOW,
        FAN,
        UNKNOWN
    };

private:
    std::optional<std::string> _custom_name;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  model    The model
     */
    weapon(fb::game::context& context, const fb::model::weapon& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right
     */
    weapon(const fb::game::weapon& right);
    /**
     * @brief      Destroys the object.
     */
    ~weapon();

protected:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    std::string mid_message() const final;

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    std::string inven_name() const override;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    std::string trade_name() const override;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const std::optional<std::string>& custom_name() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  name  The name
     */
    void custom_name(const std::string& name);
    /**
     * @brief      { function_description }
     */
    void reset_custom_name();
    /**
     * @brief      Returns a protocol representation of the object.
     *
     * @return     Protocol representation of the object.
     */
    fb::protocol::internal::Item to_protocol() const override;
};

/**
 * @brief      This class describes an armor.
 */
class armor : public equipment
{
public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  model    The model
     */
    armor(fb::game::context& context, const fb::model::armor& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right
     */
    armor(const fb::game::armor& right);
    /**
     * @brief      Destroys the object.
     */
    ~armor();
};

/**
 * @brief      This class describes a helmet.
 */
class helmet : public equipment
{
public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  model    The model
     */
    helmet(fb::game::context& context, const fb::model::helmet& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right
     */
    helmet(const helmet& right);
    /**
     * @brief      Destroys the object.
     */
    ~helmet();
};

/**
 * @brief      This class describes a shield.
 */
class shield : public equipment
{
public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  model    The model
     */
    shield(fb::game::context& context, const fb::model::shield& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right
     */
    shield(const shield& right);
    /**
     * @brief      Destroys the object.
     */
    ~shield();
};

/**
 * @brief      This class describes a ring.
 */
class ring : public equipment
{
public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  model    The model
     */
    ring(fb::game::context& context, const fb::model::ring& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right
     */
    ring(const ring& right);
    /**
     * @brief      Destroys the object.
     */
    ~ring();
};

/**
 * @brief      This class describes an auxiliary.
 */
class auxiliary : public equipment
{
public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  model    The model
     */
    auxiliary(fb::game::context& context, const fb::model::auxiliary& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right
     */
    auxiliary(const auxiliary& right);
    /**
     * @brief      Destroys the object.
     */
    ~auxiliary();
};

/**
 * @brief      This class describes a bow.
 */
class bow : public equipment
{
public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  model    The model
     */
    bow(fb::game::context& context, const fb::model::bow& model);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right
     */
    bow(const bow& right);
    /**
     * @brief      Destroys the object.
     */
    ~bow();
};

/**
 * @brief      This class describes items.
 */
class items : public fb::game::inventory<fb::game::item>
{
private:
    fb::game::character& _owner;

private:
    fb::game::weapon*    _weapon         = nullptr;
    fb::game::armor*     _armor          = nullptr;
    fb::game::helmet*    _helmet         = nullptr;
    fb::game::shield*    _shield         = nullptr;
    fb::game::ring*      _rings[2]       = {nullptr, nullptr};
    fb::game::auxiliary* _auxiliaries[2] = {nullptr, nullptr};

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      owner  The owner
     */
    items(fb::game::character& owner);
    /**
     * @brief      Destroys the object.
     */
    ~items();

private:
    /**
     * @brief      { function_description }
     *
     * @param[in]  parts  The parts
     *
     * @return     { description_of_the_return_value }
     */
    async::task<uint8_t> equipment_off(EQUIPMENT_PARTS parts);

public:
    /**
     * @brief      Adds the specified item.
     *
     * @param      item  The item
     *
     * @return     { description_of_the_return_value }
     */
    async::task<uint8_t> add(fb::game::item& item);
    /**
     * @brief      Adds the specified item.
     *
     * @param      item  The item
     *
     * @return     { description_of_the_return_value }
     */
    async::task<uint8_t> add(fb::game::item* item);
    /**
     * @brief      { function_description }
     *
     * @param[in]  items             The items
     * @param[in]  stop_if_remained  Indicates if stop if remained
     *
     * @return     { description_of_the_return_value }
     */
    async::task<std::vector<uint8_t>> add(const std::vector<fb::game::item*>& items, bool stop_if_remained = false);
    /**
     * @brief      { function_description }
     *
     * @param      item   The item
     * @param[in]  index  The index
     *
     * @return     { description_of_the_return_value }
     */
    async::task<uint8_t> add(fb::game::item& item, uint8_t index);
    /**
     * @brief      { function_description }
     *
     * @param[in]  index  The index
     *
     * @return     { description_of_the_return_value }
     */
    async::task<fb::game::item*> active(uint8_t index);
    /**
     * @brief      { function_description }
     *
     * @param[in]  parts  The parts
     *
     * @return     { description_of_the_return_value }
     */
    async::task<uint8_t> inactive(EQUIPMENT_PARTS parts);
    /**
     * @brief      { function_description }
     *
     * @param[in]  item  The item
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t index(const fb::model::item& item) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  item  The item
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t index(const fb::game::item& item) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  item  The item
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<uint8_t> index_all(const fb::model::item& item) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  parts  The parts
     * @param      item   The item
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::equipment* wear(EQUIPMENT_PARTS parts, fb::game::equipment* item);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::weapon* weapon() const;
    /**
     * @brief      { function_description }
     *
     * @param      weapon  The weapon
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::weapon* weapon(fb::game::weapon* weapon);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::armor* armor() const;
    /**
     * @brief      { function_description }
     *
     * @param      armor  The armor
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::armor* armor(fb::game::armor* armor);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::shield* shield() const;
    /**
     * @brief      { function_description }
     *
     * @param      shield  The shield
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::shield* shield(fb::game::shield* shield);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::helmet* helmet() const;
    /**
     * @brief      { function_description }
     *
     * @param      helmet  The helmet
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::helmet* helmet(fb::game::helmet* helmet);
    /**
     * @brief      { function_description }
     *
     * @param[in]  position  The position
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::ring* ring(EQUIPMENT_POSITION position) const;
    /**
     * @brief      { function_description }
     *
     * @param      ring  The ring
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::ring* ring(fb::game::ring* ring);
    /**
     * @brief      { function_description }
     *
     * @param      ring      The ring
     * @param[in]  position  The position
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::ring* ring(fb::game::ring* ring, EQUIPMENT_POSITION position);
    /**
     * @brief      { function_description }
     *
     * @param[in]  position  The position
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::auxiliary* auxiliary(EQUIPMENT_POSITION position) const;
    /**
     * @brief      { function_description }
     *
     * @param      auxiliary  The auxiliary
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::auxiliary* auxiliary(fb::game::auxiliary* auxiliary);
    /**
     * @brief      { function_description }
     *
     * @param      auxiliary  The auxiliary
     * @param[in]  position   The position
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::auxiliary* auxiliary(fb::game::auxiliary* auxiliary, EQUIPMENT_POSITION position);
    /**
     * @brief      Searches for the first match.
     *
     * @param[in]  name  The name
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::item* find(const std::string& name) const;
    /**
     * @brief      Searches for the first match.
     *
     * @param[in]  model  The model
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::item* find(const fb::model::item& model) const;
    /**
     * @brief      Finds a bundle.
     *
     * @param[in]  model  The model
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::item* find_bundle(const fb::model::item& model) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  index  The index
     * @param[in]  count  The count
     *
     * @return     { description_of_the_return_value }
     */
    async::task<fb::game::item*> drop(uint8_t index, uint8_t count);
    /**
     * @brief      { function_description }
     *
     * @param[in]  boost  The boost
     *
     * @return     { description_of_the_return_value }
     */
    async::task<void> pickup(bool boost);
    /**
     * @brief      { function_description }
     *
     * @param[in]  index  The index
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> throws(uint8_t index);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    std::map<EQUIPMENT_PARTS, item*> equipments() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  src   The source
     * @param[in]  dst   The destination
     *
     * @return     { description_of_the_return_value }
     */
    bool swap(uint8_t src, uint8_t dst) override;

    /**
     * @brief      { function_description }
     *
     * @param[in]  index  The index
     * @param[in]  count  The count
     * @param[in]  attr   The attribute
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::item* remove(uint8_t index, uint16_t count = 1, ITEM_DELETE_TYPE attr = ITEM_DELETE_TYPE::NONE);
    /**
     * @brief      { function_description }
     *
     * @param      item   The item
     * @param[in]  count  The count
     * @param[in]  attr   The attribute
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::item* remove(fb::game::item& item, uint16_t count = 1, ITEM_DELETE_TYPE attr = ITEM_DELETE_TYPE::NONE);
};

}} // namespace fb::game

#endif // !__ITEM_H__
