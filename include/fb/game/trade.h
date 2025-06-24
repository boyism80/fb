#ifndef __TRADE_H__
#define __TRADE_H__

/**
 * @file    trade.h
 * @brief   Player-to-player trading system for secure item and money exchange
 * @author  FB Development Team
 *
 * @details This file implements the comprehensive trading system that allows players
 *          to securely exchange items and money in a controlled, synchronized manner.
 *          The system provides complete trade session management with state tracking,
 *          validation, and atomic transaction processing to ensure fair and secure
 *          exchanges between players.
 *
 *          Key features:
 *          - Secure two-player trading with comprehensive state management
 *          - Item and money exchange with thorough validation and anti-cheat measures
 *          - Trade locking mechanism for final confirmation before exchange
 *          - Atomic transaction processing to prevent item duplication or loss
 *          - Trade cancellation and automatic rollback functionality
 *          - Event-driven architecture with listener pattern for UI updates
 *          - Support for stackable item quantity selection and management
 *          - Inventory space validation before trade completion
 *          - Real-time trade state synchronization between participants
 *          - Comprehensive error handling and edge case management
 *
 * @note    The trading system ensures complete transaction integrity and prevents
 *          common trading exploits while providing a smooth user experience
 *          for legitimate item and money exchanges between players.
 */

#include <fb/stream.h>
#include <unordered_map>

namespace fb { namespace game {

/**
 * @brief      Forward declaration of the character class.
 */
class character;
/**
 * @brief      Forward declaration of the item class.
 */
class item;

/**
 * @brief      Manages player-to-player trading functionality.
 *
 *             This class handles the complete trading system between two players, managing
 *             the exchange of items and money in a secure, synchronized manner. It provides
 *             state management for trade sessions, item/money validation, and ensures
 *             atomic transactions to prevent duplication or loss.
 *
 *             Key features:
 *             - Secure two-player trading with state management
 *             - Item and money exchange with validation
 *             - Trade locking mechanism for confirmation
 *             - Atomic transaction processing
 *             - Trade cancellation and rollback
 *             - Event-driven architecture with listener pattern
 *             - Anti-cheat validation and security checks
 */
class trade
{
public:
    /**
     * @brief      Enumeration of possible trade states.
     *
     *             This enumeration defines the various states that a trade
     *             can be in during the trading process, from initiation
     *             to completion or cancellation.
     */
    enum class state : uint8_t;

public:
    /**
     * @brief      Event listener interface for trade-related events and notifications.
     */
    struct listener_t;

private:
    character&                           _owner;
    character*                           _you = nullptr;
    std::unordered_map<uint8_t, uint8_t> _items;
    uint8_t                              _selected = 0xFF;
    uint32_t                             _money    = 0;
    bool                                 _locked   = false;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      owner  The character that owns this trade instance
     */
    trade(character& owner);
    /**
     * @brief      Destroys the object.
     */
    ~trade();

private:
    /**
     * @brief      Adds an item to the trade by inventory slot index.
     *
     * @param[in]  index  The inventory slot index of the item to add
     *
     * @return     The trade slot index where the item was placed, or invalid index if failed
     */
    uint8_t add(uint8_t index);
    /**
     * @brief      Restores all items and money back to the owner's inventory.
     */
    void restore();

    /**
     * @brief      Searches for an item in the owner's inventory by model.
     *
     * @param[in]  item  The item model to search for
     *
     * @return     Pointer to the found item, or nullptr if not found
     */
    std::shared_ptr<fb::game::item> find(const fb::model::item& item) const;
    /**
     * @brief      Validates that both trades can be completed successfully.
     *
     * @param      trade  The other trade to validate exchange with
     */
    void assert_exchange(const fb::game::trade& trade) const;
    /**
     * @brief      Ends the current trade session and cleans up resources.
     */
    void end();

    /**
     * @brief      Executes the item and money exchange between two trades.
     *
     * @param      trade1  The first trade participant
     * @param      trade2  The second trade participant
     */
    static void exchange(trade& trade1, trade& trade2);

public:
    /**
     * @brief      Gets the other character participating in this trade.
     *
     * @return     Pointer to the other character, or nullptr if no active trade
     */
    character* you() const;
    /**
     * @brief      Initiates a trade session with another character.
     *
     * @param      you   The character to begin trading with
     *
     * @return     True if the trade was successfully initiated, false otherwise
     */
    bool begin(character& you);
    /**
     * @brief      Checks if this character is currently in an active trade.
     *
     * @return     True if currently trading, false otherwise
     */
    bool trading() const;
    /**
     * @brief      Adds an item to the trade from the owner's inventory.
     *
     * @param[in]  index  The inventory slot index of the item to add
     *
     * @return     True if the item was successfully added to trade, false otherwise
     */
    bool up_item(uint8_t index);
    /**
     * @brief      Sets the amount of money to offer in the trade.
     *
     * @param[in]  money  The amount of money to offer
     *
     * @return     True if the money was successfully set, false otherwise
     */
    bool up_money(uint32_t money);
    /**
     * @brief      Gets the amount of money currently offered in this trade.
     *
     * @return     The amount of money being offered
     */
    uint32_t money() const;
    /**
     * @brief      Sets the quantity count for stackable items in the trade.
     *
     * @param[in]  count  The quantity count to set for the selected item
     *
     * @return     True if the count was successfully set, false otherwise
     */
    bool count(uint16_t count);
    /**
     * @brief      Cancels the current trade session.
     *
     * @return     True if the trade was successfully cancelled, false otherwise
     */
    bool cancel();
    /**
     * @brief      Locks the trade to prevent further modifications.
     *
     * @return     True if the trade was successfully locked, false otherwise
     */
    bool lock();
    /**
     * @brief      Gets all items currently offered in this trade.
     *
     * @return     Vector of pointers to items being offered in the trade
     */
    const std::vector<std::shared_ptr<fb::game::item>> items() const;

    /**
     * @brief      Gets a specific item from the trade by slot index.
     *
     * @param[in]  index  The trade slot index of the item to retrieve
     *
     * @return     Pointer to the item at the specified slot, or nullptr if empty
     */
    const std::shared_ptr<fb::game::item> item(uint8_t index) const;
};

/**
 * @brief      Enumeration of trade states for the trading system.
 *
 *             This enumeration defines all possible states that a trade
 *             transaction can be in, controlling the flow and validation
 *             of the trading process between two players.
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
 * @brief      Event listener interface for trade-related events and notifications.
 */
struct trade::listener_t
{
    /**
     * @brief      Called when a trade session begins between two characters.
     *
     * @param      me    The character whose trade is being observed
     * @param      you   The other character participating in the trade
     */
    virtual void on_trade_begin(character& me, character& you) = 0;
    /**
     * @brief      Called when trade items need to be bundled and synchronized.
     *
     * @param      me    The character whose trade bundle is being updated
     */
    virtual void on_trade_bundle(character& me) = 0;
    /**
     * @brief      Called when an item is added to or modified in the trade.
     *
     * @param      me     The character whose trade is being observed
     * @param      you    The other character participating in the trade
     * @param[in]  index  The trade slot index where the item was placed
     */
    virtual void on_trade_item(character& me, character& you, uint8_t index, const fb::game::item& item) = 0;
    /**
     * @brief      Called when money is offered or changed in the trade.
     *
     * @param      me     The character whose trade is being observed
     * @param      you    The other character participating in the trade
     * @param[in]  money  The amount of money being offered
     */
    virtual void on_trade_money(character& me, character& you, uint32_t money) = 0;
    /**
     * @brief      Called when a trade session is cancelled by either participant.
     *
     * @param      me    The character whose trade is being observed
     * @param      you   The other character who was participating in the trade
     */
    virtual void on_trade_cancel(character& me, character& you) = 0;
    /**
     * @brief      Called when a trade is locked by one of the participants.
     *
     * @param      me    The character whose trade is being observed
     * @param      you   The other character participating in the trade
     */
    virtual void on_trade_lock(character& me, character& you) = 0;
    /**
     * @brief      Called when a trade fails due to validation or other errors.
     *
     * @param      me    The character whose trade is being observed
     * @param      you   The other character who was participating in the trade
     */
    virtual void on_trade_failed(character& me, character& you) = 0;
    /**
     * @brief      Called when a trade is successfully completed.
     *
     * @param      me    The character whose trade is being observed
     * @param      you   The other character who participated in the trade
     */
    virtual void on_trade_success(character& me, character& you) = 0;
};

}} // namespace fb::game

#endif // !__TRADE_H__