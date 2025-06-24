#ifndef __DIALOG_H__
#define __DIALOG_H__

/**
 * @file    dialog.h
 * @brief   Interactive dialog system for NPC conversations and UI interactions
 * @author  FB Development Team
 *
 * @details This file implements the comprehensive dialog system that handles all
 *          interactive conversations between players and NPCs, as well as various
 *          UI interactions that require player input or selection. The system
 *          provides multiple dialog types with rich customization options for
 *          creating engaging interactive experiences.
 *
 *          Key features:
 *          - Multiple dialog interaction types (normal, menu, input, item selection)
 *          - Character appearance presets for visual dialog representation
 *          - Navigation support with previous/next buttons for multi-page dialogs
 *          - Menu-based selection system for NPC services and options
 *          - Text input dialogs with validation and length restrictions
 *          - Item list dialogs for trading, crafting, and inventory management
 *          - Inventory slot selection dialogs for item-specific operations
 *          - Sequence tracking for complex dialog chains and state management
 *          - Event-driven architecture with listener pattern for UI updates
 *          - Rich customization options for dialog appearance and behavior
 *          - Integration with NPC scripting system for dynamic content
 *
 * @note    The dialog system serves as the primary interface for player-NPC
 *          interactions and provides the foundation for quest systems, trading,
 *          crafting, and other interactive game mechanics.
 */

#include <stack>
#include <fb/game/npc.h>

namespace fb::game {
class character;
}

namespace fb::game::dialog {
using item_pair  = std::pair<fb::model::item&, uint32_t>;
using item_pairs = std::vector<item_pair>;

/**
 * @brief      Enumeration defining different types of dialog interactions.
 *
 *             This enum specifies the various interaction modes available in the dialog
 *             system, determining how the client should present the dialog to the player
 *             and what type of input or response is expected.
 */
enum class interaction : uint8_t
{
    NORMAL,   ///< Standard dialog with text and navigation buttons
    INPUT,    ///< Dialog requesting text input from the player
    INPUT_EX, ///< Extended input dialog with additional formatting
    MENU,     ///< Dialog presenting a menu of selectable options
    LIST,     ///< Dialog showing a list of items or choices
    SLOT,     ///< Dialog for item slot selection or management
    ITEM,     ///< Dialog for item-related interactions
};

/**
 * @brief      Character appearance preset for dialog display.
 *
 *             This structure defines the visual appearance of a character or NPC
 *             as displayed in dialog windows. It includes all visual elements
 *             such as gender, equipment, colors, and other appearance attributes
 *             that affect how the character is rendered in the dialog interface.
 */
struct preset
{
    SEX                     sex          = SEX::MAN;      ///< Character gender
    STATE                   state        = STATE::NORMAL; ///< Character state/pose
    uint16_t                face         = 0;             ///< Face/head appearance ID
    uint8_t                 hair_color   = 0;             ///< Hair color index
    std::optional<uint16_t> weapon       = std::nullopt;  ///< Weapon appearance ID
    std::optional<uint8_t>  weapon_color = std::nullopt;  ///< Weapon color index
    std::optional<uint8_t>  armor        = std::nullopt;  ///< Armor appearance ID
    std::optional<uint8_t>  armor_color  = std::nullopt;  ///< Armor color index
    std::optional<uint16_t> shield       = std::nullopt;  ///< Shield appearance ID
    std::optional<uint8_t>  shield_color = std::nullopt;  ///< Shield color index

    /**
     * @brief      Constructs a default preset with standard appearance.
     */
    preset() = default;

    /**
     * @brief      Constructs a preset based on a character's current appearance.
     *
     *             Creates a preset that matches the specified character's current
     *             visual appearance, copying all relevant appearance attributes.
     *
     * @param[in]  ch    The character to copy appearance from.
     */
    preset(const character& ch);
};

/**
 * @brief      Event listener interface for dialog system interactions.
 *
 *             This interface defines all the callback methods that handle different
 *             types of dialog events. Implementations of this interface receive
 *             notifications when dialogs are displayed, allowing for proper client
 *             communication and UI updates.
 */
struct listener_t
{
    /**
     * @brief      Called when a basic dialog with navigation buttons is displayed.
     *
     *             Handles standard dialog interactions with text content and optional
     *             previous/next navigation buttons for multi-page dialogs.
     *
     * @param      me           The character receiving the dialog.
     * @param[in]  object       The object (typically NPC) initiating the dialog.
     * @param[in]  message      The dialog message text to display.
     * @param[in]  button_prev  Whether to show a "previous" navigation button.
     * @param[in]  button_next  Whether to show a "next" navigation button.
     * @param[in]  sequence     The dialog sequence identifier for tracking.
     */
    virtual void on_dialog(character&               me,
                           const fb::model::object& object,
                           const std::string&       message,
                           bool                     button_prev,
                           bool                     button_next,
                           uint32_t                 sequence = 0xFFFFFFFD) = 0;
    /**
     * @brief      Called when a menu dialog is displayed.
     *
     *             Handles dialog interactions that present a list of selectable menu
     *             options to the player, typically for NPC services or choices.
     *
     * @param      me       The character receiving the dialog.
     * @param[in]  obj      The object (typically NPC) initiating the dialog.
     * @param[in]  message  The dialog message text to display above the menu.
     * @param[in]  menus    The list of menu option strings for player selection.
     * @param[in]  sequence The dialog sequence identifier for tracking.
     */
    virtual void on_dialog(character&                      me,
                           const fb::model::object&        obj,
                           const std::string&              message,
                           const std::vector<std::string>& menus,
                           uint32_t                        sequence = 0xFFFFFFFD) = 0;

    /**
     * @brief      Called when a menu dialog with navigation is displayed.
     *
     *             Handles dialog interactions that present a menu with an optional
     *             previous button for navigating back through dialog sequences.
     *
     * @param      me           The character receiving the dialog.
     * @param[in]  obj          The object (typically NPC) initiating the dialog.
     * @param[in]  message      The dialog message text to display above the menu.
     * @param[in]  menus        The list of menu option strings for player selection.
     * @param[in]  button_prev  Whether to show a "previous" navigation button.
     * @param[in]  sequence     The dialog sequence identifier for tracking.
     */
    virtual void on_dialog(character&                      me,
                           const fb::model::object&        obj,
                           const std::string&              message,
                           const std::vector<std::string>& menus,
                           bool                            button_prev,
                           uint32_t                        sequence = 0xFFFFFFFD) = 0;

    /**
     * @brief      Called when a menu dialog with character preset is displayed.
     *
     *             Handles dialog interactions that include a character appearance
     *             preset for visual display alongside the menu options.
     *
     * @param      me           The character receiving the dialog.
     * @param[in]  obj          The object (typically NPC) initiating the dialog.
     * @param[in]  message      The dialog message text to display above the menu.
     * @param[in]  menus        The list of menu option strings for player selection.
     * @param[in]  button_prev  Whether to show a "previous" navigation button.
     * @param[in]  preset       The character appearance preset for visual display.
     * @param[in]  sequence     The dialog sequence identifier for tracking.
     */
    virtual void on_dialog(character&                      me,
                           const fb::model::object&        obj,
                           const std::string&              message,
                           const std::vector<std::string>& menus,
                           bool                            button_prev,
                           const dialog::preset&           preset,
                           uint32_t                        sequence = 0xFFFFFFFD) = 0;
    /**
     * @brief      Called when an item slot selection dialog is displayed.
     *
     *             Handles dialog interactions that allow players to select from
     *             specific inventory slots, typically for item management operations.
     *
     * @param      me          The character receiving the dialog.
     * @param[in]  obj         The object (typically NPC) initiating the dialog.
     * @param[in]  message     The dialog message text to display.
     * @param[in]  item_slots  The list of available inventory slot indices.
     * @param[in]  sequence    The dialog sequence identifier for tracking.
     */
    virtual void on_dialog(character&                  me,
                           const fb::model::object&    obj,
                           const std::string&          message,
                           const std::vector<uint8_t>& item_slots,
                           uint32_t                    sequence = 0xFFFFFFFD) = 0;

    /**
     * @brief      Called when an item list dialog is displayed.
     *
     *             Handles dialog interactions that present a list of items with
     *             quantities, typically for trading, crafting, or inventory management.
     *
     * @param      me       The character receiving the dialog.
     * @param[in]  obj      The object (typically NPC) initiating the dialog.
     * @param[in]  message  The dialog message text to display above the item list.
     * @param[in]  pairs    The list of item-quantity pairs to display.
     * @param[in]  sequence The dialog sequence identifier for tracking.
     * @param[in]  pursuit  Additional pursuit/context parameter for the dialog.
     */
    virtual void on_dialog(character&               me,
                           const fb::model::object& obj,
                           const std::string&       message,
                           const item_pairs&        pairs,
                           uint32_t                 sequence = 0xFFFFFFFD,
                           uint16_t                 pursuit  = 0xFFFF) = 0;
    /**
     * @brief      Called when a simple text dialog is displayed.
     *
     *             Handles basic dialog interactions with only text content,
     *             typically for informational messages or simple confirmations.
     *
     * @param      me       The character receiving the dialog.
     * @param[in]  obj      The object (typically NPC) initiating the dialog.
     * @param[in]  message  The dialog message text to display.
     * @param[in]  sequence The dialog sequence identifier for tracking.
     */
    virtual void on_dialog(character&               me,
                           const fb::model::object& obj,
                           const std::string&       message,
                           uint32_t                 sequence = 0xFFFFFFFD) = 0;

    /**
     * @brief      Called when a text input dialog is displayed.
     *
     *             Handles dialog interactions that request text input from the player,
     *             with customizable input field properties and validation.
     *
     * @param      me       The character receiving the dialog.
     * @param[in]  obj      The object (typically NPC) initiating the dialog.
     * @param[in]  message  The dialog message text to display above the input field.
     * @param[in]  top      The text to display at the top of the input field.
     * @param[in]  bottom   The text to display at the bottom of the input field.
     * @param[in]  maxlen   The maximum length of input allowed (default 255).
     * @param[in]  prev     Whether to show a "previous" navigation button.
     * @param[in]  sequence The dialog sequence identifier for tracking.
     */
    virtual void on_dialog(character&               me,
                           const fb::model::object& obj,
                           const std::string&       message,
                           const std::string&       top,
                           const std::string&       bottom,
                           int                      maxlen   = 0xFF,
                           bool                     prev     = false,
                           uint32_t                 sequence = 0xFFFFFFFD) = 0;
};
} // namespace fb::game::dialog
#endif