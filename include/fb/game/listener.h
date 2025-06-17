#ifndef __LISTENER_H__
#define __LISTENER_H__

#include <fb/game/character.h>

namespace fb::game {

class context;

/**
 * @brief      Base listener interface combining character and mob event handling.
 *
 *             This structure serves as the primary listener interface for the game context,
 *             inheriting from both character and mob listener interfaces to provide unified
 *             event handling for all game entities.
 */
struct listener_t : public virtual character::listener_t, public virtual mob::listener_t
{ };

/**
 * @brief      Concrete implementation of the game event listener system.
 *
 *             This class provides the complete implementation of all game event handlers,
 *             managing the flow of events between game objects and the network layer.
 *             It handles events from objects, life entities, characters, mobs, items,
 *             spells, trades, dialogs, and other game systems.
 *
 *             Key responsibilities:
 *             - Object lifecycle events (create, destroy, move, hide)
 *             - Character-specific events (level up, equipment, inventory)
 *             - Combat events (attack, damage, death, buffs)
 *             - Social events (chat, groups, clans, trade)
 *             - UI events (dialogs, menus, notifications)
 *             - Network protocol generation and transmission
 *             - Event broadcasting to relevant clients
 */
struct listener_impl : public listener_t
{
public:
    fb::game::context& context;

public:
    listener_impl(fb::game::context& ctx);
    listener_impl(const listener_impl&) = delete;
    listener_impl(listener_impl&&)      = delete;
    ~listener_impl()                    = default;

public:
    listener_impl& operator= (const listener_impl&) = delete;
    listener_impl& operator= (listener_impl&&)      = delete;

public:
    /**
     * @brief      Called when an object is created in the game world.
     *
     *             Handles the creation event for game objects, typically sending
     *             creation notifications to nearby players and initializing
     *             object visibility and state.
     *
     * @param      me    The object that was created.
     */
    void on_create(fb::game::object& me) override final;

    /**
     * @brief      Called when an object is destroyed in the game world.
     *
     *             Handles the destruction event for game objects, sending
     *             destruction notifications to nearby players and cleaning
     *             up object references and state.
     *
     * @param      me    The object that was destroyed.
     */
    void on_destroy(fb::game::object& me) override final;

    /**
     * @brief      Called when an object sends a chat message.
     *
     *             Processes chat messages from game objects, handling message
     *             broadcasting, filtering, and delivery to appropriate recipients
     *             based on chat type and proximity.
     *
     * @param      me         The object sending the chat message.
     * @param[in]  message    The chat message content.
     * @param[in]  chat_type  The type of chat (normal, whisper, shout, etc.).
     */
    void on_chat(fb::game::object&  me,
                 const std::string& message,
                 CHAT_TYPE          chat_type = CHAT_TYPE::NORMAL) override final;

    /**
     * @brief      Called when an object changes direction.
     *
     *             Handles direction change events for game objects, updating
     *             visual representation and notifying nearby players of the
     *             direction change.
     *
     * @param      me    The object that changed direction.
     */
    void on_direction(fb::game::object& me) override final;

    /**
     * @brief      Called when an object's external appearance needs updating.
     *
     *             Handles updates to an object's visual representation that are
     *             visible to other players, such as equipment changes, status
     *             effects, or other visual modifications.
     *
     * @param      me     The object whose appearance is being updated.
     * @param[in]  light  Whether to use light update (minimal data) or full update.
     */
    void on_update_external(fb::game::object& me, bool light) override final;

    /**
     * @brief      Called when an object's external appearance needs updating for a specific observer.
     *
     *             Handles targeted updates to an object's visual representation for
     *             a specific observer, allowing for personalized or conditional
     *             appearance updates.
     *
     * @param      me     The object whose appearance is being updated.
     * @param      you    The observer who will receive the update.
     * @param[in]  light  Whether to use light update (minimal data) or full update.
     */
    void on_update_external(fb::game::object& me, fb::game::object& you, bool light) override final;

    /**
     * @brief      Called when an object is hidden from view.
     *
     * @param      me            The object being hidden
     * @param[in]  destroy_type  The type of destruction/hiding to apply
     */
    void on_hide(fb::game::object& me, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) override final;

    /**
     * @brief      Called when an object is hidden from a specific observer.
     *
     * @param      me            The object being hidden
     * @param      you           The observer who should no longer see the object
     * @param[in]  destroy_type  The type of destruction/hiding to apply
     */
    void on_hide(fb::game::object& me,
                 fb::game::object& you,
                 DESTROY_TYPE      destroy_type = DESTROY_TYPE::DEFAULT) override final;

    /**
     * @brief      Called when an object moves to a new position.
     *
     * @param      me      The object that moved
     * @param[in]  before  The previous position before the move
     */
    void on_move(fb::game::object& me, const fb::model::point16_t& before) override final;

    /**
     * @brief      Called when a buff effect is applied to an object.
     *
     * @param      me    The object receiving the buff
     * @param      buff  The buff effect being applied
     */
    void on_buff(fb::game::object& me, fb::game::buff& buff) override final;

    /**
     * @brief      Called when a buff effect is removed from an object.
     *
     * @param      me    The object losing the buff
     * @param      buff  The buff effect being removed
     */
    void on_unbuff(fb::game::object& me, fb::game::buff& buff) override final;

    /**
     * @brief      Called when a character's map view needs updating.
     *
     * @param      ch    The character whose map view is being updated
     * @param[in]  map   The map data to send to the character
     */
    void on_update_map(character& ch, const fb::game::map& map) override final;

    /**
     * @brief      Called when a character's background music changes.
     *
     * @param      ch      The character whose background music is changing
     * @param[in]  bgm     The new background music ID to play
     * @param[in]  volume  The volume level for the background music
     */
    void on_update_bgm(character& ch, uint16_t bgm, uint8_t volume) override final;

    /**
     * @brief      Called when a sound effect should be played for an object.
     *
     * @param      ch     The object that should play the sound
     * @param[in]  sound  The sound effect ID to play
     */
    void on_sound(fb::game::object& ch, SOUND sound) override final;

    /**
     * @brief      Called when a visual effect should be displayed for an object.
     *
     * @param      ch     The object that should display the effect
     * @param[in]  value  The visual effect ID to display
     */
    void on_effect(fb::game::object& ch, uint8_t value) override final;

    /**
     * @brief      Called when a life entity performs an attack action.
     *
     * @param      me    The life entity performing the attack
     */
    async::task<void> on_attack(life& me, DURATION duration = DURATION::ATTACK) override final;

    /**
     * @brief      Called when a life entity dies.
     *
     * @param      me    The life entity that died
     * @param      you   The object that caused the death (optional)
     */
    void on_dead(life& me, object* you) override final;

    /**
     * @brief      Called when a life entity's HP changes and needs visual update.
     *
     * @param      me        The life entity whose HP changed
     * @param[in]  diff      The amount of HP change (damage or healing)
     * @param[in]  critical  Whether this was a critical hit
     */
    void on_update_hp(life& me, uint32_t diff, bool critical) override final;

    /**
     * @brief      Called when a life entity performs an action with visual effects.
     *
     * @param      me        The life entity performing the action
     * @param[in]  action    The type of action being performed
     * @param[in]  duration  The duration of the action animation
     * @param[in]  sound     The sound effect ID to play with the action
     */
    void on_action(life& me, ACTION action, DURATION duration, uint8_t sound) override final;

    /**
     * @brief      Called when a character's state needs synchronization with the client.
     *
     * @param      me     The character whose state is being updated
     * @param[in]  level  The update level indicating what data to synchronize
     */
    void on_update(character& me, STATE_LEVEL level) override final;

    /**
     * @brief      Called when a message should be sent to a character.
     *
     * @param      me       The character receiving the message
     * @param[in]  message  The message text to send
     * @param[in]  type     The type of message (system, chat, etc.)
     */
    void on_message(character& me, const std::string& message, MESSAGE_TYPE type) override final;

    /**
     * @brief      Called when a character option setting is changed.
     *
     * @param      me       The character whose option was changed
     * @param[in]  option   The option that was changed
     * @param[in]  enabled  Whether the option is now enabled or disabled
     */
    void on_option_changed(character& me, OPTION option, bool enabled) override final;

    /**
     * @brief      Called when character options need synchronization with the client.
     *
     * @param      me    The character whose options are being updated
     */
    void on_update_option(character& me) override final;

    /**
     * @brief      Called when a region of a character's map view needs updating.
     *
     * @param      ch     The character whose map view is being updated
     * @param[in]  map    The map containing the updated region
     * @param[in]  begin  The starting position of the region to update
     * @param[in]  size   The size of the region to update
     */
    void on_update_map(character&                  ch,
                       const fb::game::map&        map,
                       const fb::model::point16_t& begin,
                       const fb::model::size8_t&   size) override final;

    /**
     * @brief      Called when a character's buff effects need synchronization.
     *
     * @param      ch     The character whose buffs are being updated
     * @param[in]  buffs  The current buff effects applied to the character
     */
    void on_update_buff(character& ch, const fb::game::buffs& buffs) override final;

    /**
     * @brief      Called when a character's internal game state needs synchronization.
     *
     * @param      ch    The character whose internal state is being updated
     */
    void on_update_internal(character& ch) override final;

    /**
     * @brief      Called when game time changes and needs to be sent to a character.
     *
     * @param      ch     The character receiving the time update
     * @param[in]  hours  The current game time in hours
     */
    void on_update_time(character& ch, uint16_t hours) override final;

    /**
     * @brief      Called when a character has been fully initialized.
     *
     * @param      ch    The character that has been initialized
     */
    void on_character_init(character& ch) override final;

    /**
     * @brief      Called when a character's position has changed.
     *
     * @param      ch    The character whose position was updated
     */
    void on_update_position(character& ch) override final;

    /**
     * @brief      Called when a character inspects another character.
     *
     * @param      ch      The character performing the inspection
     * @param[in]  target  The character being inspected
     */
    void on_browse_character(character& ch, const character& target) override final;

    /**
     * @brief      Called when an item tooltip should be displayed.
     *
     * @param      ch        The character who should see the tooltip
     * @param[in]  item      The item to display information for
     * @param[in]  position  The slot position of the item
     */
    void on_item_tooltip(character& ch, const fb::game::item& item, uint16_t position) override final;

    /**
     * @brief      Called when the user list should be shown to a character.
     *
     * @param      ch    The character who should see the user list
     */
    void on_show_user_list(character& ch) override final;

    /**
     * @brief      Called when the main bulletin interface should be shown.
     *
     * @param      ch    The character who should see the bulletin
     */
    void on_show_bulletin(character& ch) override final;

    /**
     * @brief      Called when a bulletin is displayed to a character.
     *
     * @param      ch        The character viewing the bulletin.
     * @param[in]  section   The bulletin section being displayed.
     * @param[in]  articles  The list of articles in the bulletin section.
     * @param[in]  flag      The bulletin button enable flags for UI control.
     */
    void on_show_bulletin(character&                                    ch,
                          const fb::model::bulletin&                    section,
                          const std::list<fb::game::bulletin::article>& articles,
                          BULLETIN_BUTTON_ENABLE                        flag) override final;

    /**
     * @brief      Called when displaying a specific bulletin article to a character.
     *
     * @param      ch     The character viewing the bulletin article.
     * @param[in]  value  The bulletin article being displayed.
     * @param[in]  flag   The button enable flags for bulletin interaction.
     */
    void on_show_bulletin(character&                         ch,
                          const fb::game::bulletin::article& value,
                          BULLETIN_BUTTON_ENABLE             flag) override final;

    /**
     * @brief      Called when displaying the mail box list to a character.
     *
     * @param      ch     The character viewing the mail box.
     * @param[in]  mails  The list of mail summaries to display.
     * @param[in]  flag   The button enable flags for mail interaction.
     */
    void on_show_mail_box(character&                                              ch,
                          const std::vector<fb::protocol::internal::MailSummary>& mails,
                          MAIL_BUTTON_ENABLE                                      flag) override final;

    /**
     * @brief      Called when displaying a specific mail to a character.
     *
     * @param      ch    The character viewing the mail.
     * @param[in]  mail  The mail content being displayed.
     * @param[in]  flag  The button enable flags for mail interaction.
     */
    void on_show_mail_box(character&                          ch,
                          const fb::protocol::internal::Mail& mail,
                          MAIL_BUTTON_ENABLE                  flag) override final;

    /**
     * @brief      Called when displaying a bulletin operation result message.
     *
     * @param      ch       The character receiving the message.
     * @param[in]  message  The result message to display.
     * @param[in]  success  Whether the operation was successful.
     * @param[in]  mail     Whether this is related to mail functionality.
     */
    void on_show_bulletin_message(character& ch, const std::string& message, bool success, bool mail) override final;

    /**
     * @brief      Called when displaying the world map to a character.
     *
     * @param      ch     The character viewing the world map.
     * @param[in]  id     The map identifier.
     * @param[in]  index  The map index or position.
     */
    void on_show_world_map(character& ch, uint32_t id, uint16_t index) override final;

    /**
     * @brief      Called when a timer event occurs for a character.
     *
     * @param      ch    The character affected by the timer.
     * @param[in]  time  The timer duration or remaining time.
     * @param[in]  type  The type of timer event.
     */
    void on_timer(character& ch, uint32_t time, TIMER_TYPE type) override final;

    /**
     * @brief      Called when weather changes for a character.
     *
     * @param      ch       The character experiencing the weather change.
     * @param[in]  weather  The new weather type.
     */
    void on_weather(character& ch, WEATHER_TYPE weather) override final;

    /**
     * @brief      Called when brightness/lighting changes for a character.
     *
     * @param      ch     The character experiencing the brightness change.
     * @param[in]  value  The new brightness value.
     */
    void on_bright(character& ch, uint8_t value) override final;

    /**
     * @brief      Called when a character's unique identifier needs updating.
     *
     * @param      ch    The character whose ID is being updated
     */
    void on_update_id(character& ch) override final;

    /**
     * @brief      Called when a character gains a level.
     *
     * @param      me    The character who leveled up
     */
    void on_level_up(character& me) override final;

    /**
     * @brief      Called when a character is being transferred to a different map.
     *
     * @param      me        The character being transferred
     * @param      map       The destination map
     * @param[in]  position  The target position on the destination map
     *
     * @return     Async task that completes with transfer success status
     */
    [[nodiscard]] async::task<bool> on_transfer(character&                  me,
                                                fb::game::map&              map,
                                                const fb::model::point16_t& position) override final;

    /**
     * @brief      Called when an item is removed from a character's inventory.
     *
     * @param      me     The character whose item was removed
     * @param[in]  index  The inventory slot index of the removed item
     * @param[in]  attr   The type of item deletion that occurred
     */
    void on_item_remove(character& me, uint8_t index, ITEM_DELETE_TYPE attr) override final;

    /**
     * @brief      Called when an item in a character's inventory needs updating.
     *
     * @param      me     The character whose item was updated
     * @param[in]  index  The inventory slot index of the updated item
     */
    void on_item_update(character& me, uint8_t index) override final;

    /**
     * @brief      Called when two items are swapped in a character's inventory.
     *
     * @param      me    The character whose items were swapped
     * @param[in]  src   The source slot index
     * @param[in]  dst   The destination slot index
     */
    void on_item_swap(character& me, uint8_t src, uint8_t dst) override final;

    /**
     * @brief      Called when a character equips an item.
     *
     * @param      me     The character equipping the item
     * @param      item   The item being equipped
     * @param[in]  parts  The equipment slot where the item is equipped
     */
    void on_equipment_on(character& me, item& item, EQUIPMENT_PARTS parts) override final;

    /**
     * @brief      Called when a character unequips an item.
     *
     * @param      me     The character unequipping the item
     * @param[in]  parts  The equipment slot being cleared
     * @param[in]  index  The equipment being removed
     */
    void on_equipment_off(character& me, EQUIPMENT_PARTS parts, fb::game::equipment& equipment) override final;

    /**
     * @brief      Called when equipment durability decreases.
     *
     * @param      me         The character whose equipment lost durability
     * @param      equipment  The equipment that lost durability
     * @param[in]  before     The durability value before the decrease
     * @param[in]  after      The durability value after the decrease
     */
    void
    on_durability_down(character& me, fb::game::equipment& equipment, uint32_t before, uint32_t after) override final;

    /**
     * @brief      Called when a character activates or uses an item.
     *
     * @param      me    The character using the item
     * @param      item  The item being activated
     */
    void on_item_active(character& me, item& item) override final;

    /**
     * @brief      Called when a character throws an item to a location.
     *
     * @param      me    The character throwing the item
     * @param      item  The item being thrown
     * @param[in]  to    The target position where the item is thrown
     */
    void on_item_throws(character& me, item& item, const fb::model::point16_t& to) override final;

    /**
     * @brief      Called when a spell in a life entity's collection needs updating.
     *
     * @param      me     The life entity whose spell was updated
     * @param[in]  index  The spell slot index that was updated
     */
    void on_spell_update(life& me, uint8_t index) override final;

    /**
     * @brief      Called when a spell is removed from a life entity's collection.
     *
     * @param      me     The life entity whose spell was removed
     * @param[in]  index  The spell slot index that was cleared
     */
    void on_spell_remove(life& me, uint8_t index) override final;

    /**
     * @brief      Called when a trade session begins between two characters.
     *
     * @param      me    The character whose trade is being observed
     * @param      you   The other character participating in the trade
     */
    void on_trade_begin(character& me, character& you) override final;

    /**
     * @brief      Called when trade items need to be bundled and synchronized.
     *
     * @param      me    The character whose trade bundle is being updated
     */
    void on_trade_bundle(character& me) override final;

    /**
     * @brief      Called when an item is added to or modified in a trade.
     *
     * @param      me     The character whose trade is being observed
     * @param      from   The character who added the item to the trade
     * @param[in]  index  The trade slot index where the item was placed
     * @param[in]  item   The item that was added to the trade
     */
    void on_trade_item(character& me, character& from, uint8_t index, const fb::game::item& item) override final;

    /**
     * @brief      Called when money is offered or changed in a trade.
     *
     * @param      me     The character whose trade is being observed
     * @param      you    The other character participating in the trade
     * @param[in]  money  The amount of money being offered
     */
    void on_trade_money(character& me, character& you, uint32_t money) override final;

    /**
     * @brief      Called when a trade session is cancelled.
     *
     * @param      me    The character whose trade is being observed
     * @param      you   The other character who was participating in the trade
     */
    void on_trade_cancel(character& me, character& you) override final;

    /**
     * @brief      Called when a trade is locked by one of the participants.
     *
     * @param      me    The character whose trade is being observed
     * @param      you   The other character participating in the trade
     */
    void on_trade_lock(character& me, character& you) override final;

    /**
     * @brief      Called when a trade fails due to validation or other errors.
     *
     * @param      me    The character whose trade is being observed
     * @param      you   The other character who was participating in the trade
     */
    void on_trade_failed(character& me, character& you) override final;

    /**
     * @brief      Called when a trade is successfully completed.
     *
     * @param      me    The character whose trade is being observed
     * @param      you   The other character who participated in the trade
     */
    void on_trade_success(character& me, character& you) override final;

    /**
     * @brief      Called when displaying a dialog with navigation buttons.
     *
     * @param      me           The character who should see the dialog
     * @param[in]  object       The object model associated with the dialog
     * @param[in]  message      The dialog message text
     * @param[in]  button_prev  Whether to show a previous button
     * @param[in]  button_next  Whether to show a next button
     * @param[in]  sequence     The dialog sequence number for tracking
     */
    void on_dialog(character&               me,
                   const fb::model::object& object,
                   const std::string&       message,
                   bool                     button_prev,
                   bool                     button_next,
                   uint32_t                 sequence = 0xFFFFFFFD) override final;

    /**
     * @brief      Called when displaying a dialog with menu options.
     *
     * @param      me       The character who should see the dialog
     * @param[in]  obj      The object model associated with the dialog
     * @param[in]  message  The dialog message text
     * @param[in]  menus    The list of menu option strings
     * @param[in]  sequence The dialog sequence number for tracking
     */
    void on_dialog(character&                      me,
                   const fb::model::object&        obj,
                   const std::string&              message,
                   const std::vector<std::string>& menus,
                   uint32_t                        sequence = 0xFFFFFFFD) override final;

    /**
     * @brief      Called when displaying a dialog with menu options and navigation.
     *
     * @param      me           The character who should see the dialog
     * @param[in]  obj          The object model associated with the dialog
     * @param[in]  message      The dialog message text
     * @param[in]  menus        The list of menu option strings
     * @param[in]  button_prev  Whether to show a previous button
     * @param[in]  sequence     The dialog sequence number for tracking
     */
    void on_dialog(character&                      me,
                   const fb::model::object&        obj,
                   const std::string&              message,
                   const std::vector<std::string>& menus,
                   bool                            button_prev,
                   uint32_t                        sequence = 0xFFFFFFFD) override final;

    /**
     * @brief      Called when displaying a dialog with menu options, navigation, and preset configuration.
     *
     * @param      me           The character who should see the dialog
     * @param[in]  obj          The object model associated with the dialog
     * @param[in]  message      The dialog message text
     * @param[in]  menus        The list of menu option strings
     * @param[in]  button_prev  Whether to show a previous button
     * @param[in]  preset       The dialog preset configuration
     * @param[in]  sequence     The dialog sequence number for tracking
     */
    void on_dialog(character&                      me,
                   const fb::model::object&        obj,
                   const std::string&              message,
                   const std::vector<std::string>& menus,
                   bool                            button_prev,
                   const dialog::preset&           preset,
                   uint32_t                        sequence = 0xFFFFFFFD) override final;

    /**
     * @brief      Called when displaying a dialog with item slot selection.
     *
     * @param      me           The character who should see the dialog
     * @param[in]  obj          The object model associated with the dialog
     * @param[in]  message      The dialog message text
     * @param[in]  item_slots   The list of item slot indices to display
     * @param[in]  sequence     The dialog sequence number for tracking
     */
    void on_dialog(character&                  me,
                   const fb::model::object&    obj,
                   const std::string&          message,
                   const std::vector<uint8_t>& item_slots,
                   uint32_t                    sequence = 0xFFFFFFFD) override final;

    /**
     * @brief      Called when displaying a dialog with item pairs.
     *
     * @param      me       The character who should see the dialog
     * @param[in]  obj      The object model associated with the dialog
     * @param[in]  message  The dialog message text
     * @param[in]  pairs    The item pairs to display in the dialog
     * @param[in]  sequence The dialog sequence number for tracking
     * @param[in]  pursuit  The pursuit identifier for quest tracking
     */
    void on_dialog(character&                          me,
                   const fb::model::object&            obj,
                   const std::string&                  message,
                   const fb::game::dialog::item_pairs& pairs,
                   uint32_t                            sequence = 0xFFFFFFFD,
                   uint16_t                            pursuit  = 0xFFFF) override final;

    /**
     * @brief      Called when displaying a simple dialog.
     *
     * @param      me       The character who should see the dialog
     * @param[in]  obj      The object model associated with the dialog
     * @param[in]  message  The dialog message text
     * @param[in]  sequence The dialog sequence number for tracking
     */
    void on_dialog(character&               me,
                   const fb::model::object& obj,
                   const std::string&       message,
                   uint32_t                 sequence = 0xFFFFFFFD) override final;

    /**
     * @brief      Called when displaying a dialog with text input fields.
     *
     * @param      me       The character who should see the dialog
     * @param[in]  obj      The object model associated with the dialog
     * @param[in]  message  The dialog message text
     * @param[in]  top      The top text input field label
     * @param[in]  bottom   The bottom text input field label
     * @param[in]  maxlen   The maximum length for text input
     * @param[in]  prev     Whether to show a previous button
     * @param[in]  sequence The dialog sequence number for tracking
     */
    void on_dialog(character&               me,
                   const fb::model::object& obj,
                   const std::string&       message,
                   const std::string&       top,
                   const std::string&       bottom,
                   int                      maxlen   = 0xFF,
                   bool                     prev     = false,
                   uint32_t                 sequence = 0xFFFFFFFD) override final;
};

} // namespace fb::game

#endif // !__LISTENER_H__
