#ifndef __LISTENER_H__
#define __LISTENER_H__

#include <fb/game/character.h>

namespace fb::game {

class context;

/**
 * @brief      { struct_description }
 */
struct listener : public virtual character::listener_t, public virtual mob::listener_t
{ };

/**
 * @brief      { struct_description }
 */
struct listener_impl : public listener
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
     * @brief      Called on create.
     *
     * @param      me    { parameter_description }
     */
    void on_create(fb::game::object& me) override final;

    /**
     * @brief      Called on destroy.
     *
     * @param      me    { parameter_description }
     */
    void on_destroy(fb::game::object& me) override final;

    /**
     * @brief      Called on chat.
     *
     * @param      me         { parameter_description }
     * @param[in]  message    The message
     * @param[in]  chat_type  The chat type
     */
    void on_chat(fb::game::object&  me,
                 const std::string& message,
                 CHAT_TYPE          chat_type = CHAT_TYPE::NORMAL) override final;

    /**
     * @brief      Called on direction.
     *
     * @param      me    { parameter_description }
     */
    void on_direction(fb::game::object& me) override final;

    /**
     * @brief      Called on update external.
     *
     * @param      me     { parameter_description }
     * @param[in]  light  The light
     */
    void on_update_external(fb::game::object& me, bool light) override final;

    /**
     * @brief      Called on update external.
     *
     * @param      me     { parameter_description }
     * @param      you    You
     * @param[in]  light  The light
     */
    void on_update_external(fb::game::object& me, fb::game::object& you, bool light) override final;

    /**
     * @brief      Called on hide.
     *
     * @param      me            { parameter_description }
     * @param[in]  destroy_type  The destroy type
     */
    void on_hide(fb::game::object& me, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) override final;

    /**
     * @brief      Called on hide.
     *
     * @param      me            { parameter_description }
     * @param      you           You
     * @param[in]  destroy_type  The destroy type
     */
    void on_hide(fb::game::object& me,
                 fb::game::object& you,
                 DESTROY_TYPE      destroy_type = DESTROY_TYPE::DEFAULT) override final;

    /**
     * @brief      Called on move.
     *
     * @param      me      { parameter_description }
     * @param[in]  before  The before
     */
    void on_move(fb::game::object& me, const fb::model::point16_t& before) override final;

    /**
     * @brief      Called on buffer.
     *
     * @param      me    { parameter_description }
     * @param      buff  The buffer
     */
    void on_buff(fb::game::object& me, fb::game::buff& buff) override final;

    /**
     * @brief      Called on unbuff.
     *
     * @param      me    { parameter_description }
     * @param      buff  The buffer
     */
    void on_unbuff(fb::game::object& me, fb::game::buff& buff) override final;

    /**
     * @brief      Called on update map.
     *
     * @param      ch    { parameter_description }
     * @param[in]  map   The map
     */
    void on_update_map(character& ch, const fb::game::map& map) override final;

    /**
     * @brief      Called on update bgm.
     *
     * @param      ch      { parameter_description }
     * @param[in]  bgm     The bgm
     * @param[in]  volume  The volume
     */
    void on_update_bgm(character& ch, uint16_t bgm, uint8_t volume) override final;

    /**
     * @brief      Called on sound.
     *
     * @param      ch     { parameter_description }
     * @param[in]  sound  The sound
     */
    void on_sound(fb::game::object& ch, SOUND sound) override final;

    /**
     * @brief      Called on effect.
     *
     * @param      ch     { parameter_description }
     * @param[in]  value  The value
     */
    void on_effect(fb::game::object& ch, uint8_t value) override final;

    /**
     * @brief      Called on attack.
     *
     * @param      me    { parameter_description }
     */
    async::task<void> on_attack(life& me, DURATION duration = DURATION::ATTACK) override final;

    /**
     * @brief      Called on dead.
     *
     * @param      me    { parameter_description }
     * @param      you   You
     */
    void on_dead(life& me, object* you) override final;

    /**
     * @brief      Called on update hp.
     *
     * @param      me        { parameter_description }
     * @param[in]  diff      The difference
     * @param[in]  critical  The critical
     */
    void on_update_hp(life& me, uint32_t diff, bool critical) override final;

    /**
     * @brief      Called on action.
     *
     * @param      me        { parameter_description }
     * @param[in]  action    The action
     * @param[in]  duration  The duration
     * @param[in]  sound     The sound
     */
    void on_action(life& me, ACTION action, DURATION duration, uint8_t sound) override final;

    /**
     * @brief      Called on update.
     *
     * @param      me     { parameter_description }
     * @param[in]  level  The level
     */
    void on_update(character& me, STATE_LEVEL level) override final;

    /**
     * @brief      Called on message.
     *
     * @param      me       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  type     The type
     */
    void on_message(character& me, const std::string& message, MESSAGE_TYPE type) override final;

    /**
     * @brief      Called when option changed.
     *
     * @param      me       { parameter_description }
     * @param[in]  option   The option
     * @param[in]  enabled  Indicates if enabled
     */
    void on_option_changed(character& me, OPTION option, bool enabled) override final;

    /**
     * @brief      Called on update option.
     *
     * @param      me    { parameter_description }
     */
    void on_update_option(character& me) override final;

    /**
     * @brief      Called on update map.
     *
     * @param      ch     { parameter_description }
     * @param[in]  map    The map
     * @param[in]  begin  The begin
     * @param[in]  size   The size
     */
    void on_update_map(character&                  ch,
                       const fb::game::map&        map,
                       const fb::model::point16_t& begin,
                       const fb::model::size8_t&   size) override final;

    /**
     * @brief      Called on update buffer.
     *
     * @param      ch     { parameter_description }
     * @param[in]  buffs  The buffs
     */
    void on_update_buff(character& ch, const fb::game::buffs& buffs) override final;

    /**
     * @brief      Called on update internal.
     *
     * @param      ch    { parameter_description }
     */
    void on_update_internal(character& ch) override final;

    /**
     * @brief      Called on update time.
     *
     * @param      ch     { parameter_description }
     * @param[in]  hours  The hours
     */
    void on_update_time(character& ch, uint16_t hours) override final;

    /**
     * @brief      Called on character initialize.
     *
     * @param      ch    { parameter_description }
     */
    void on_character_init(character& ch) override final;

    /**
     * @brief      Called on update position.
     *
     * @param      ch    { parameter_description }
     */
    void on_update_position(character& ch) override final;

    /**
     * @brief      Called on browse character.
     *
     * @param      ch      { parameter_description }
     * @param[in]  target  The target
     */
    void on_browse_character(character& ch, const character& target) override final;

    /**
     * @brief      Called on item tooltip.
     *
     * @param      ch        { parameter_description }
     * @param[in]  item      The item
     * @param[in]  position  The position
     */
    void on_item_tooltip(character& ch, const fb::game::item& item, uint16_t position) override final;

    /**
     * @brief      Called on show user list.
     *
     * @param      ch    { parameter_description }
     */
    void on_show_user_list(character& ch) override final;

    /**
     * @brief      Called on show board.
     *
     * @param      ch    { parameter_description }
     */
    void on_show_board(character& ch) override final;

    /**
     * @brief      Called on show board.
     *
     * @param      ch        { parameter_description }
     * @param[in]  section   The section
     * @param[in]  articles  The articles
     * @param[in]  flag      The flag
     */
    void on_show_board(character&                                 ch,
                       const fb::model::board&                    section,
                       const std::list<fb::game::board::article>& articles,
                       BOARD_BUTTON_ENABLE                        flag) override final;

    /**
     * @brief      Called on show board.
     *
     * @param      ch     { parameter_description }
     * @param[in]  value  The value
     * @param[in]  flag   The flag
     */
    void on_show_board(character& ch, const fb::game::board::article& value, BOARD_BUTTON_ENABLE flag) override final;

    /**
     * @brief      Called on show mail box.
     *
     * @param      ch     { parameter_description }
     * @param[in]  mails  The mails
     * @param[in]  flag   The flag
     */
    void on_show_mail_box(character&                                              ch,
                          const std::vector<fb::protocol::internal::MailSummary>& mails,
                          MAIL_BUTTON_ENABLE                                      flag) override final;

    /**
     * @brief      Called on show mail box.
     *
     * @param      ch    { parameter_description }
     * @param[in]  mail  The mail
     * @param[in]  flag  The flag
     */
    void on_show_mail_box(character&                          ch,
                          const fb::protocol::internal::Mail& mail,
                          MAIL_BUTTON_ENABLE                  flag) override final;

    /**
     * @brief      Called on show board message.
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  success  The success
     * @param[in]  mail     The mail
     */
    void on_show_board_message(character& ch, const std::string& message, bool success, bool mail) override final;

    /**
     * @brief      Called on show world map.
     *
     * @param      ch     { parameter_description }
     * @param[in]  id     The identifier
     * @param[in]  index  The index
     */
    void on_show_world_map(character& ch, uint32_t id, uint16_t index) override final;

    /**
     * @brief      Called on timer.
     *
     * @param      ch    { parameter_description }
     * @param[in]  time  The time
     * @param[in]  type  The type
     */
    void on_timer(character& ch, uint32_t time, TIMER_TYPE type) override final;

    /**
     * @brief      Called on weather.
     *
     * @param      ch       { parameter_description }
     * @param[in]  weather  The weather
     */
    void on_weather(character& ch, WEATHER_TYPE weather) override final;

    /**
     * @brief      Called on bright.
     *
     * @param      ch     { parameter_description }
     * @param[in]  value  The value
     */
    void on_bright(character& ch, uint8_t value) override final;

    /**
     * @brief      Called on update identifier.
     *
     * @param      ch    { parameter_description }
     */
    void on_update_id(character& ch) override final;

    /**
     * @brief      Called on level up.
     *
     * @param      me    { parameter_description }
     */
    void on_level_up(character& me) override final;

    /**
     * @brief      Called on transfer.
     *
     * @param      me        { parameter_description }
     * @param      map       The map
     * @param[in]  position  The position
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> on_transfer(character&                  me,
                                                fb::game::map&              map,
                                                const fb::model::point16_t& position) override final;

    /**
     * @brief      Called on item remove.
     *
     * @param      me     { parameter_description }
     * @param[in]  index  The index
     * @param[in]  attr   The attribute
     */
    void on_item_remove(character& me, uint8_t index, ITEM_DELETE_TYPE attr) override final;

    /**
     * @brief      Called on item update.
     *
     * @param      me     { parameter_description }
     * @param[in]  index  The index
     */
    void on_item_update(character& me, uint8_t index) override final;

    /**
     * @brief      Called on item swap.
     *
     * @param      me    { parameter_description }
     * @param[in]  src   The source
     * @param[in]  dst   The destination
     */
    void on_item_swap(character& me, uint8_t src, uint8_t dst) override final;

    /**
     * @brief      Called on equipment on.
     *
     * @param      me     { parameter_description }
     * @param      item   The item
     * @param[in]  parts  The parts
     */
    void on_equipment_on(character& me, item& item, EQUIPMENT_PARTS parts) override final;

    /**
     * @brief      Called on equipment off.
     *
     * @param      me     { parameter_description }
     * @param[in]  parts  The parts
     * @param[in]  index  The index
     */
    void on_equipment_off(character& me, EQUIPMENT_PARTS parts, fb::game::equipment& equipment) override final;

    /**
     * @brief      Called on durability down.
     *
     * @param      me         { parameter_description }
     * @param      equipment  The equipment
     * @param[in]  before     The before
     * @param[in]  after      The after
     */
    void
    on_durability_down(character& me, fb::game::equipment& equipment, uint32_t before, uint32_t after) override final;

    /**
     * @brief      Called on item active.
     *
     * @param      me    { parameter_description }
     * @param      item  The item
     */
    void on_item_active(character& me, item& item) override final;

    /**
     * @brief      Called on item throws.
     *
     * @param      me    { parameter_description }
     * @param      item  The item
     * @param[in]  to    { parameter_description }
     */
    void on_item_throws(character& me, item& item, const fb::model::point16_t& to) override final;

    /**
     * @brief      Called on spell update.
     *
     * @param      me     { parameter_description }
     * @param[in]  index  The index
     */
    void on_spell_update(life& me, uint8_t index) override final;

    /**
     * @brief      Called on spell remove.
     *
     * @param      me     { parameter_description }
     * @param[in]  index  The index
     */
    void on_spell_remove(life& me, uint8_t index) override final;

    /**
     * @brief      Called on trade begin.
     *
     * @param      me    { parameter_description }
     * @param      you   You
     */
    void on_trade_begin(character& me, character& you) override final;

    /**
     * @brief      Called on trade bundle.
     *
     * @param      me    { parameter_description }
     */
    void on_trade_bundle(character& me) override final;

    /**
     * @brief      Called on trade item.
     *
     * @param      me     { parameter_description }
     * @param      from   The from
     * @param[in]  index  The index
     * @param[in]  item   The item
     */
    void on_trade_item(character& me, character& from, uint8_t index, const fb::game::item& item) override final;

    /**
     * @brief      Called on trade money.
     *
     * @param      me     { parameter_description }
     * @param      you    You
     * @param[in]  money  The money
     */
    void on_trade_money(character& me, character& you, uint32_t money) override final;

    /**
     * @brief      Called on trade cancel.
     *
     * @param      me    { parameter_description }
     * @param      you   You
     */
    void on_trade_cancel(character& me, character& you) override final;

    /**
     * @brief      Called on trade lock.
     *
     * @param      me    { parameter_description }
     * @param      you   You
     */
    void on_trade_lock(character& me, character& you) override final;

    /**
     * @brief      Called when trade failed.
     *
     * @param      me    { parameter_description }
     * @param      you   You
     */
    void on_trade_failed(character& me, character& you) override final;

    /**
     * @brief      Called on trade success.
     *
     * @param      me    { parameter_description }
     * @param      you   You
     */
    void on_trade_success(character& me, character& you) override final;

    /**
     * @brief      Called on dialog.
     *
     * @param      me           { parameter_description }
     * @param[in]  object       The object
     * @param[in]  message      The message
     * @param[in]  button_prev  The button previous
     * @param[in]  button_next  The button next
     * @param[in]  interaction  The interaction
     */
    void on_dialog(character&               me,
                   const fb::model::object& object,
                   const std::string&       message,
                   bool                     button_prev,
                   bool                     button_next,
                   uint32_t                 sequence = 0xFFFFFFFD) override final;

    /**
     * @brief      Called on dialog.
     *
     * @param      me           { parameter_description }
     * @param[in]  obj          The obj
     * @param[in]  message      The message
     * @param[in]  menus        The menus
     * @param[in]  interaction  The interaction
     */
    void on_dialog(character&                      me,
                   const fb::model::object&        obj,
                   const std::string&              message,
                   const std::vector<std::string>& menus,
                   uint32_t                        sequence = 0xFFFFFFFD) override final;

    /**
     * @brief      Called on dialog.
     *
     * @param      me           { parameter_description }
     * @param[in]  obj          The obj
     * @param[in]  message      The message
     * @param[in]  menus        The menus
     * @param[in]  button_prev  The button previous
     * @param[in]  interaction  The interaction
     */
    void on_dialog(character&                      me,
                   const fb::model::object&        obj,
                   const std::string&              message,
                   const std::vector<std::string>& menus,
                   bool                            button_prev,
                   uint32_t                        sequence = 0xFFFFFFFD) override final;

    /**
     * @brief      Called on dialog.
     *
     * @param      me           { parameter_description }
     * @param[in]  obj          The object
     * @param[in]  message      The message
     * @param[in]  menus        The menus
     * @param[in]  button_prev  The button previous
     * @param[in]  preset       The preset
     */
    void on_dialog(character&                      me,
                   const fb::model::object&        obj,
                   const std::string&              message,
                   const std::vector<std::string>& menus,
                   bool                            button_prev,
                   const dialog::preset&           preset,
                   uint32_t                        sequence = 0xFFFFFFFD) override final;

    /**
     * @brief      Called on dialog.
     *
     * @param      me           { parameter_description }
     * @param[in]  obj          The obj
     * @param[in]  message      The message
     * @param[in]  item_slots   The item slots
     * @param[in]  interaction  The interaction
     */
    void on_dialog(character&                  me,
                   const fb::model::object&    obj,
                   const std::string&          message,
                   const std::vector<uint8_t>& item_slots,
                   uint32_t                    sequence = 0xFFFFFFFD) override final;

    /**
     * @brief      Called on dialog.
     *
     * @param      me           { parameter_description }
     * @param[in]  obj          The obj
     * @param[in]  message      The message
     * @param[in]  pairs        The pairs
     * @param[in]  pursuit      The pursuit
     * @param[in]  interaction  The interaction
     */
    void on_dialog(character&                          me,
                   const fb::model::object&            obj,
                   const std::string&                  message,
                   const fb::game::dialog::item_pairs& pairs,
                   uint32_t                            sequence = 0xFFFFFFFD,
                   uint16_t                            pursuit  = 0xFFFF) override final;

    /**
     * @brief      Called on dialog.
     *
     * @param      me           { parameter_description }
     * @param[in]  obj          The obj
     * @param[in]  message      The message
     * @param[in]  interaction  The interaction
     */
    void on_dialog(character&               me,
                   const fb::model::object& obj,
                   const std::string&       message,
                   uint32_t                 sequence = 0xFFFFFFFD) override final;

    /**
     * @brief      Called on dialog.
     *
     * @param      me           { parameter_description }
     * @param[in]  obj          The obj
     * @param[in]  message      The message
     * @param[in]  top          The top
     * @param[in]  bottom       The bottom
     * @param[in]  maxlen       The maxlen
     * @param[in]  prev         The previous
     * @param[in]  interaction  The interaction
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
