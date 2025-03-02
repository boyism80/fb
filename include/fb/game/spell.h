#ifndef __SPELL_H__
#define __SPELL_H__

#include <chrono>
#include <fb/game/inventory.h>
#include <fb/model/model.h>

namespace fb::game {

/**
 * @brief      This class describes an object.
 */
class object;
/**
 * @brief      This class describes a life.
 */
class life;

/**
 * @brief      This class describes a spell.
 */
class spell : public lua::luable
{
public:
    LUA_PROTOTYPE

private:
    fb::model::datetime _next;
    fb::model::datetime _internal_next;
    uint8_t             _internal_cast_count = 0;

public:
    const fb::game::context& context;
    const fb::game::life&    owner;
    const fb::model::spell&  model;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  context  The context
     * @param[in]  model    The model
     */
    spell(const fb::game::context& context,
          const fb::game::life&    owner,
          const fb::model::spell&  model,
          uint16_t                 delay = 0);
    /**
     * @brief      Destroys the object.
     */
    ~spell();

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void delay(uint16_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint16_t delay() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const fb::model::datetime& next() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool update_lock();

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_model(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_delay(lua_State* L);

    static int builtin_delay2(lua_State* L);
};

/**
 * @brief      This class describes spells.
 */
class spells : public fb::game::inventory<fb::game::spell>
{
public:
    /**
     * @brief      { struct_description }
     */
    struct listener;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      owner  The owner
     */
    spells(life& owner);
    /**
     * @brief      Destroys the object.
     */
    ~spells();

    /**
     * @brief      Searches for the first match.
     *
     * @param[in]  name  The name
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::spell* find(const std::string& name) const;

    /**
     * @brief      Searches for the first match.
     *
     * @param[in]  model  The model
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::spell* find(const fb::model::spell& model) const;

public:
    /**
     * @brief      Adds the specified element.
     *
     * @param[in]  element  The element
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t add(fb::game::spell& element) override;

    /**
     * @brief      { function_description }
     *
     * @param[in]  element  The element
     * @param[in]  index    The index
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t add(fb::game::spell& element, uint8_t index) override;

    /**
     * @brief      Adds the specified model.
     *
     * @param[in]  model  The model
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t add(const fb::model::spell& model, uint8_t slot, uint16_t delay);

    /**
     * @brief      Adds the specified model.
     *
     * @param[in]  model  The model
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t add(const fb::model::spell& model);

    /**
     * @brief      Removes the specified index.
     *
     * @param[in]  index  The index
     *
     * @return     { description_of_the_return_value }
     */
    bool remove(uint8_t index) override;
    /**
     * @brief      { function_description }
     *
     * @param[in]  src   The source
     * @param[in]  dst   The destination
     *
     * @return     { description_of_the_return_value }
     */
    bool swap(uint8_t src, uint8_t dst) override;
};

/**
 * @brief      { struct_description }
 */
struct spells::listener
{
    /**
     * @brief      Called on spell update.
     *
     * @param      me     { parameter_description }
     * @param[in]  index  The index
     */
    virtual void on_spell_update(life& me, uint8_t index) = 0;
    /**
     * @brief      Called on spell remove.
     *
     * @param      me     { parameter_description }
     * @param[in]  index  The index
     */
    virtual void on_spell_remove(life& me, uint8_t index) = 0;
};

/**
 * @brief      This class describes a buffer.
 */
class buff : public lua::luable
{
public:
    LUA_PROTOTYPE

private:
    std::chrono::milliseconds _time;

public:
    const fb::game::context& context;
    const fb::model::spell&  model;
    const fb::game::object*  caster;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  context  The context
     * @param[in]  model    The model
     * @param[in]  caster   The caster
     * @param[in]  seconds  The seconds
     */
    buff(const fb::game::context& context,
         const fb::model::spell&  model,
         const fb::game::object*  caster,
         uint32_t                 seconds);
    /**
     * @brief      Destroys the object.
     */
    ~buff();

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    std::chrono::milliseconds time() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  value    The value
     *
     * @tparam     _Rep     { description }
     * @tparam     _Period  { description }
     */
    template <class _Rep, class _Period>
    void time(const std::chrono::duration<_Rep, _Period>& value)
    {
        this->_time = std::chrono::duration_cast<std::chrono::milliseconds>(value);
    }
    /**
     * @brief      { function_description }
     *
     * @param[in]  inc   The increment
     */
    void time_inc(const std::chrono::steady_clock::duration& inc);
    /**
     * @brief      { function_description }
     *
     * @param[in]  dec   The decrement
     */
    void time_dec(const std::chrono::steady_clock::duration& dec);

public:
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_model(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_time(lua_State* L);
};

/**
 * @brief      This class describes buffs.
 */
class buffs : private std::unordered_map<uint32_t, buff*>
{
private:
    fb::game::object& _owner;

public:
    using std::unordered_map<uint32_t, buff*>::size;
    using std::unordered_map<uint32_t, buff*>::begin;
    using std::unordered_map<uint32_t, buff*>::end;
    using std::unordered_map<uint32_t, buff*>::cbegin;
    using std::unordered_map<uint32_t, buff*>::cend;
    using std::unordered_map<uint32_t, buff*>::contains;
    using std::unordered_map<uint32_t, buff*>::insert;
    using std::unordered_map<uint32_t, buff*>::erase;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      owner  The owner
     */
    buffs(fb::game::object& owner);
    /**
     * @brief      Destroys the object.
     */
    ~buffs();

private:
    /**
     * @brief      Pushes a back.
     *
     * @param      buff  The buffer
     *
     * @return     { description_of_the_return_value }
     */
    bool push_back(buff& buff);

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  model  The model
     *
     * @return     { description_of_the_return_value }
     */
    bool contains(const fb::model::spell& model) const;
    /**
     * @brief      Pushes a back.
     *
     * @param[in]  spell    The spell
     * @param[in]  seconds  The seconds
     * @param[in]  caster   The caster
     *
     * @return     { description_of_the_return_value }
     */
    buff* push_back(const fb::model::spell& spell, uint32_t seconds, const fb::game::object* caster = nullptr);
    /**
     * @brief      Removes the specified identifier.
     *
     * @param[in]  id    The identifier
     *
     * @return     { description_of_the_return_value }
     */
    bool remove(uint32_t id);
    /**
     * @brief      Removes the specified spell.
     *
     * @param[in]  spell  The spell
     *
     * @return     { description_of_the_return_value }
     */
    bool remove(const fb::model::spell& spell);

public:
    /**
     * @brief      Array indexer operator.
     *
     * @param[in]  id    The identifier
     *
     * @return     The result of the array indexer
     */
    buff* operator[] (uint32_t id) const;
};

} // namespace fb::game

#endif // !__SPELL_H__
