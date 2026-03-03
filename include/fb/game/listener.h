#ifndef __LISTENER_H__
#define __LISTENER_H__

#include <fb/game/character.h>

namespace fb::game {

class server;

struct listener_t : public virtual character::listener_t, public virtual mob::listener_t
{ };

struct listener_impl : public listener_t
{
public:
    fb::game::server& server;

public:
    listener_impl(fb::game::server& ctx);
    listener_impl(const listener_impl&) = delete;
    listener_impl(listener_impl&&)      = delete;
    ~listener_impl()                    = default;

public:
    listener_impl& operator= (const listener_impl&) = delete;
    listener_impl& operator= (listener_impl&&)      = delete;

private:
    void send_update_appearance(fb::game::object& obj, const fb::model::appearance& appearance);

public:
    // clang-format off
    void                          on_create(fb::game::object& me) override final;
    void                          on_destroy(fb::game::object& me) override final;
    void                          on_chat(fb::game::object& me, std::string_view message, CHAT_TYPE chat_type = CHAT_TYPE::NORMAL) override final;
    void                          on_direction(fb::game::object& me) override final;
    void                          on_update_external(fb::game::object& me, bool detailed) override final;
    void                          on_update_external(fb::game::object& me, fb::game::object& you, bool detailed) override final;
    void                          on_hide(fb::game::object& me, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) override final;
    void                          on_hide(fb::game::object& me, fb::game::object& you, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) override final;
    void                          on_move(fb::game::object& me, const fb::model::point16_t& before) override final;
    void                          on_buff(fb::game::object& me, fb::game::buff& buff) override final;
    void                          on_unbuff(fb::game::object& me, fb::game::buff& buff) override final;
    void                          on_update_map(character& ch, const fb::game::map& map) override final;
    void                          on_update_bgm(character& ch, uint16_t bgm, uint8_t volume) override final;
    void                          on_sound(fb::game::object& ch, SOUND sound) override final;
    void                          on_effect(fb::game::object& ch, uint8_t value) override final;
    void                          on_map_leave(fb::game::object& me, const fb::game::map& map) override final;
    void                          on_map_enter(fb::game::object& me, const fb::game::map& map) override final;
    void                          on_attack(life& me, DURATION duration = DURATION::ATTACK) override final;
    void                          on_dead(life& me, std::shared_ptr<object> you) override final;
    void                          on_update_hp(life& me, uint32_t diff, bool critical) override final;
    void                          on_action(life& me, ACTION action, DURATION duration, uint8_t sound) override final;
    void                          on_update(character& me, UPDATE_STATE_LEVEL level) override final;
    void                          on_message(character& me, std::string_view message, MESSAGE_TYPE type) override final;
    void                          on_option_changed(character& me, OPTION option, bool enabled) override final;
    void                          on_update_option(character& me) override final;
    void                          on_update_map(character& ch, const fb::game::map& map, const fb::model::point16_t& position, const fb::model::size8_t& size, uint16_t crc) override final;
    void                          on_update_buff(character& ch, const fb::game::buffs& buffs) override final;
    void                          on_update_internal(character& ch) override final;
    void                          on_update_time(character& ch, uint16_t hours) override final;
    void                          on_character_init(character& ch) override final;
    void                          on_update_position(character& ch) override final;
    void                          on_browse_character(character& ch, const character& target) override final;
    void                          on_item_tooltip(character& ch, const fb::game::item& item, uint16_t position) override final;
    void                          on_show_user_list(character& ch) override final;
    void                          on_show_bulletin(character& ch) override final;
    void                          on_show_bulletin(character& ch, const fb::model::bulletin& section, const std::list<fb::game::bulletin::article>& articles, BULLETIN_BUTTON_ENABLE flag) override final;
    void                          on_show_bulletin(character& ch, const fb::game::bulletin::article& value, BULLETIN_BUTTON_ENABLE flag) override final;
    void                          on_show_mail_box(character& ch, const std::vector<mail_box::summary>& mails, MAIL_BUTTON_ENABLE flag) override final;
    void                          on_show_mail_box(character& ch, const mail_box::mail& mail, MAIL_BUTTON_ENABLE flag) override final;
    void                          on_show_bulletin_message(character& ch, std::string_view message, bool success, bool mail) override final;
    void                          on_show_world_map(character& ch, uint32_t id, uint16_t index) override final;
    void                          on_timer(character& ch, uint32_t time, TIMER_TYPE type) override final;
    void                          on_weather(character& ch, WEATHER_TYPE weather) override final;
    void                          on_bright(character& ch, uint8_t value) override final;
    void                          on_update_id(character& ch) override final;
    void                          on_level_up(character& me) override final;
    void                          on_item_remove(character& me, uint8_t index, ITEM_DELETE_TYPE attr) override final;
    void                          on_item_update(character& me, uint8_t index) override final;
    void                          on_item_swap(character& me, uint8_t src, uint8_t dst) override final;
    void                          on_equipment_on(character& me, item& item, EQUIPMENT_PARTS parts) override final;
    void                          on_equipment_off(character& me, EQUIPMENT_PARTS parts, fb::game::equipment& equipment) override final;
    void                          on_durability_down(character& me, fb::game::equipment& equipment, uint32_t before, uint32_t after) override final;
    void                          on_item_active(character& me, item& item) override final;
    void                          on_item_throws(character& me, item& item, const fb::model::point16_t& to) override final;
    void                          on_spell_update(life& me, uint8_t index) override final;
    void                          on_spell_remove(life& me, uint8_t index) override final;
    void                          on_trade_begin(character& me, character& you) override final;
    void                          on_trade_bundle(character& me) override final;
    void                          on_trade_item(character& me, character& from, uint8_t index, const fb::game::item& item) override final;
    void                          on_trade_money(character& me, character& you, uint32_t money) override final;
    void                          on_trade_cancel(character& me, character& you) override final;
    void                          on_trade_lock(character& me, character& you) override final;
    void                          on_trade_failed(character& me, character& you) override final;
    void                          on_trade_success(character& me, character& you) override final;
    void                          on_dialog(character& me, std::string_view message, bool button_prev, bool button_next, uint32_t seq = 0xFFFFFFFD) override final;
    void                          on_dialog(character& me, const fb::model::object& object, std::string_view message, bool button_prev, bool button_next, uint32_t seq = 0xFFFFFFFD) override final;
    void                          on_dialog(character& me, const fb::game::object& obj, std::string_view message, bool button_prev, bool button_next, uint32_t seq = 0xFFFFFFFD) override final;
    void                          on_dialog(character& me, const fb::model::object& obj, std::string_view message, const std::vector<std::string>& menus, uint32_t seq = 0xFFFFFFFD) override final;
    void                          on_dialog(character& me, const fb::game::object& obj, std::string_view message, const std::vector<std::string>& menus, uint32_t seq = 0xFFFFFFFD) override final;
    void                          on_dialog(character& me, const fb::model::object& obj, std::string_view message, const std::vector<std::string>& menus, bool button_prev, uint32_t seq = 0xFFFFFFFD) override final;
    void                          on_dialog(character& me, const fb::game::object& obj, std::string_view message, const std::vector<std::string>& menus, bool button_prev, uint32_t seq = 0xFFFFFFFD) override final;
    void                          on_dialog(character& me, std::unique_ptr<fb::game::appearance>&& appearance_ptr, std::string_view message, const std::vector<std::string>& menus, bool button_prev, uint32_t seq = 0xFFFFFFFD) override final;
    void                          on_dialog(character& me, const fb::model::object& obj, std::string_view message, const std::vector<uint8_t>& item_slots, uint32_t seq = 0xFFFFFFFD) override final;
    void                          on_dialog(character& me, const fb::game::object& obj, std::string_view message, const std::vector<uint8_t>& item_slots, uint32_t seq = 0xFFFFFFFD) override final;
    void                          on_dialog(character& me, const fb::model::object& obj, std::string_view message, const fb::game::dialog::item_pairs& pairs, uint32_t seq = 0xFFFFFFFD, uint16_t pursuit = 0xFFFF) override final;
    void                          on_dialog(character& me, const fb::game::object& obj, std::string_view message, const fb::game::dialog::item_pairs& pairs, uint32_t seq = 0xFFFFFFFD, uint16_t pursuit = 0xFFFF) override final;
    void                          on_dialog(character& me, const fb::model::object& obj, std::string_view message, uint32_t seq = 0xFFFFFFFD) override final;
    void                          on_dialog(character& me, const fb::game::object& obj, std::string_view message, uint32_t seq = 0xFFFFFFFD) override final;
    void                          on_dialog(character& me, const fb::model::object& obj, std::string_view message, std::string_view top, std::string_view bottom, int maxlen = 0xFF, bool prev = false, uint32_t seq = 0xFFFFFFFD) override final;
    void                          on_dialog(character& me, const fb::game::object& obj, std::string_view message, std::string_view top, std::string_view bottom, int maxlen = 0xFF, bool prev = false, uint32_t seq = 0xFFFFFFFD) override final;
    async::task<bool>             on_transfer(character& me, fb::game::map& map, const fb::model::point16_t& position) override final;
    // clang-format on
};

} // namespace fb::game

#endif // !__LISTENER_H__
