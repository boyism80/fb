#ifndef __TRADE_H__
#define __TRADE_H__

#include <vector>
#include <fb/stream.h>

namespace fb { namespace game {

/**
 * @brief      This class describes a character.
 */
class character;
/**
 * @brief      This class describes an item.
 */
class item;

/**
 * @brief      This class describes a trade.
 */
class trade
{
public:
    /**
     * @brief      This class describes a state.
     */
    enum class state : uint8_t;

public:
    /**
     * @brief      { struct_description }
     */
    struct listener;

private:
    character&                   _owner;
    character*                   _you = nullptr;
    std::vector<fb::game::item*> _items;
    fb::game::item*              _selected = nullptr;
    uint32_t                     _money    = 0;
    bool                         _locked   = false;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      owner  The owner
     */
    trade(character& owner);
    /**
     * @brief      Destroys the object.
     */
    ~trade();

private:
    /**
     * @brief      Searches for the first match.
     *
     * @param      item  The item
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t find(fb::game::item& item) const;
    /**
     * @brief      Adds the specified item.
     *
     * @param      item  The item
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t add(fb::game::item& item);
    /**
     * @brief      { function_description }
     */
    void restore();
    /**
     * @brief      Flushes the object.
     */
    void flush();
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool flushable() const;
    /**
     * @brief      { function_description }
     */
    void end();

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    character* you() const;
    /**
     * @brief      { function_description }
     *
     * @param      you   You
     *
     * @return     { description_of_the_return_value }
     */
    bool begin(character& you);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool trading() const;
    /**
     * @brief      { function_description }
     *
     * @param      item  The item
     *
     * @return     { description_of_the_return_value }
     */
    bool up(fb::game::item& item);
    /**
     * @brief      { function_description }
     *
     * @param[in]  money  The money
     *
     * @return     { description_of_the_return_value }
     */
    bool up(uint8_t money);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t money() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  count  The count
     *
     * @return     { description_of_the_return_value }
     */
    bool count(uint16_t count);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool cancel();
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool lock();
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const std::vector<fb::game::item*>& items() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  index  The index
     *
     * @return     { description_of_the_return_value }
     */
    const fb::game::item* item(uint8_t index) const;
};

/**
 * @brief      This class describes a state.
 */
enum class trade::state : uint8_t
{
    REQUEST    = 0x00,
    UP_ITEM    = 0x01,
    ITEM_COUNT = 0x02,
    UP_MONEY   = 0x03,
    CANCEL     = 0x04,
    LOCK       = 0x05,
};

/**
 * @brief      { struct_description }
 */
struct trade::listener
{
    /**
     * @brief      Called on trade begin.
     *
     * @param      me    { parameter_description }
     * @param      you   You
     */
    virtual void on_trade_begin(character& me, character& you) = 0;
    /**
     * @brief      Called on trade bundle.
     *
     * @param      me    { parameter_description }
     */
    virtual void on_trade_bundle(character& me) = 0;
    /**
     * @brief      Called on trade item.
     *
     * @param      me     { parameter_description }
     * @param      from   The from
     * @param[in]  index  The index
     */
    virtual void on_trade_item(character& me, character& from, uint8_t index) = 0;
    /**
     * @brief      Called on trade money.
     *
     * @param      me    { parameter_description }
     * @param      from  The from
     */
    virtual void on_trade_money(character& me, character& from) = 0;
    /**
     * @brief      Called on trade cancel.
     *
     * @param      me    { parameter_description }
     * @param      from  The from
     */
    virtual void on_trade_cancel(character& me, character& from) = 0;
    /**
     * @brief      Called on trade lock.
     *
     * @param      me    { parameter_description }
     * @param[in]  mine  The mine
     */
    virtual void on_trade_lock(character& me, bool mine) = 0;
    /**
     * @brief      Called when trade failed.
     *
     * @param      me    { parameter_description }
     */
    virtual void on_trade_failed(character& me) = 0;
    /**
     * @brief      Called on trade success.
     *
     * @param      me    { parameter_description }
     */
    virtual void on_trade_success(character& me) = 0;
};

}} // namespace fb::game

#endif // !__TRADE_H__