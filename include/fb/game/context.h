#ifndef __FB_GAME_H__
#define __FB_GAME_H__

#include <boost/algorithm/string/join.hpp>
#include <json/json.h>
#include <fstream>
#include <fb/acceptor.h>
#include <fb/game/protocol.h>
#include <fb/amqp.h>
#include <fb/game/listener.h>
#include <fb/game/thread_params.h>
#include <fb/game/map/container.h>
#include <fb/game/shard.h>

using namespace fb::protocol::internal;
namespace fb_reqs       = fb::protocol::game::request;
namespace fb_resp       = fb::protocol::game::response;
namespace internal      = fb::protocol::internal;
namespace internal_reqs = fb::protocol::internal::request;
namespace internal_resp = fb::protocol::internal::response;

namespace fb::game {

class context : public fb::acceptor<fb::game::character>, public fb::game::listener
{
public:
    LUA_PROTOTYPE

public:
    enum class scope
    {
        PIVOT,
        GROUP,
        MAP,
        WORLD
    };

public:
    struct command_config
    {
        using func_type = std::function<async::task<bool>(fb::game::character&, Json::Value&)>;
        func_type fn;
        bool      admin;
    };

public:
    using object_set         = std::unordered_map<const fb::game::object*, std::unique_ptr<fb::game::object>>;
    using transfer_param     = fb_reqs::login::transfer_param;
    using protocol_generator = std::function<std::unique_ptr<fb::protocol::header>(const fb::game::object&)>;
    using command_container  = std::unordered_map<std::string, command_config>;
    using npc_interaction_func =
        std::function<bool(character&, const std::string&, const std::vector<fb::game::npc*>&)>;

private:
    command_container                 _commands;
    fb::model::datetime               _time;
    std::unique_ptr<fb::amqp::socket> _amqp;
    std::unique_ptr<std::thread>      _amqp_thread;
    fb::game::shard                   _shard;
    std::vector<npc_interaction_func> _npc_interaction_funcs;

public:
    fb::model::model        model;
    fb::game::map_container maps;

public:
    context(boost::asio::io_context& context, uint16_t port);
    context(const context&) = delete;
    ~context();

private:
    std::string                     elapsed_message(const std::string& dt);
    void                            upsert_group_then(uint32_t gid, const std::function<void(shared_group_lock&)>& fn);
    void                            upsert_group_then(uint32_t                                       gid,
                                                      const std::string&                             master,
                                                      const std::vector<std::string>&                members,
                                                      const std::function<void(shared_group_lock&)>& fn);
    void                            update_clan(clan&                                                  clan,
                                                fb::protocol::internal::Clan&                          resp1,
                                                const std::vector<fb::protocol::internal::ClanMember>& resp2) const;
    void                            upsert_clan_then(uint32_t id, const std::function<void(shared_clan_lock&)>& fn);
    [[nodiscard]] async::task<bool> init_ch(const fb::protocol::internal::Character& response,
                                            fb::game::character&                     ch,
                                            std::optional<uint32_t>                  group,
                                            std::optional<uint32_t>                  clan,
                                            const std::optional<transfer_param>&     transfer);
    void init_option(const fb::protocol::internal::Option& response, fb::game::character& ch);
    void init_items(const std::vector<fb::protocol::internal::Item>& response, fb::game::character& ch);
    void init_spells(const std::vector<fb::protocol::internal::Spell>& response, fb::game::character& ch);
    void init_traces(const std::vector<fb::protocol::internal::Trace>& response, fb::game::character& ch);

private:
    template <typename Func>
    void command(const std::string& cmd, Func&& func, bool admin)
    {
        this->_commands.insert({
            cmd,
            command_config{.fn = std::bind(func, this, std::placeholders::_1, std::placeholders::_2), .admin = admin}
        });
    }

    template <typename Func>
    void bind_npc_interaction(Func&& func)
    {
        auto c_fn = std::bind(func, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        this->_npc_interaction_funcs.push_back(c_fn);
    }

private:
    void assert_whisper(const internal::response::Whisper& response) const;
    void assert_group(uint32_t error, const std::string& actor) const;
    void assert_clan(uint32_t) const;
    void assert_mail(uint32_t) const;
    void on_enter_group(internal_resp::EnterGroup resp);
    void on_leave_group(const internal_resp::LeaveGroup& resp);
    void on_group_broadcast(const internal_resp::BroadcastGroup& resp);
    void on_clan_broadcast(const internal_resp::BroadcastClan& resp);
    void on_clan_title_changed(const internal_resp::SetClanTitle& resp);
    void on_clan_join_member(const internal_resp::JoinClan& resp);
    void on_clan_leave_member(const internal_resp::LeaveClan& resp);
    void on_write_mail(const internal_resp::WriteMail& resp);

public:
    template <typename T, typename... Args>
    T* make(Args&&... args)
    {
        return new T(*this, std::forward<Args>(args)...);
    }

public:
    template <typename T>
    async::task<void> destroy(T& obj, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT)
    {
        if constexpr (std::is_same_v<T, fb::game::object>)
        {
            std::ignore = co_await obj.map(nullptr, fb::model::point16_t{0, 0}, destroy_type);
        }
        delete &obj;
        co_return;
    }

public:
    void              send(fb::game::object&           object,
                           const fb::protocol::header& header,
                           context::scope              scope,
                           bool                        exclude_self = false,
                           bool                        encrypt      = true);
    async::task<void> save(fb::game::character& ch);
    void              foreach_ch(const std::string&                                  name,
                                 const std::function<void(fb::game::character&)>&    fn,
                                 const std::function<void(const std::string& name)>& miss);
    void              foreach_ch(const std::string& name, const std::function<void(fb::game::character&)>& fn);
    void              foreach_ch(const std::vector<std::string>&                     names,
                                 const std::function<void(fb::game::character&)>&    fn,
                                 const std::function<void(const std::string& name)>& miss);
    void foreach_ch(const std::vector<std::string>& names, const std::function<void(fb::game::character&)>& fn);
    void foreach_ch(const clan& clan, const std::function<void(fb::game::character&)>& fn);
    void foreach_ch(const group& group, const std::function<void(fb::game::character&)>& fn);
    void foreach_ch(const std::function<void(fb::game::character&)>& fn);

public:
    virtual uint32_t  thread_id(const fb::socket<fb::game::character>& socket) const;
    fb::thread*       thread(const fb::game::map& map);
    void              amqp_thread();
    async::task<bool> create_group(character& me, const std::string& target);
    async::task<void> broadcast(const group& group, const std::string& message, MESSAGE_TYPE type);
    async::task<void> create_clan(character& me, const std::string& name);
    async::task<void> destroy_clan(character& me);
    async::task<void> set_clan_title(const clan& clan, std::string title);
    async::task<void> join_clan_member(const clan& clan, character& ch);
    async::task<void> leave_clan_member(const clan& clan, const std::string& name, bool kick);
    async::task<void> broadcast(const clan& clan, const std::string& message, MESSAGE_TYPE type);
    async::task<internal_resp::WriteMail>
    write_mail(const character& ch, const std::string& to, const std::string& title, const std::string& contents);
    async::task<internal_resp::GetMailList> mail_list(const character& ch, uint16_t offset, uint16_t count);
    async::task<internal_resp::GetMail>     read_mail(character& ch, uint16_t id);
    async::task<internal_resp::DeleteMail>  delete_mail(character& ch, uint16_t id);

protected:
    bool                            decrypt_policy(uint8_t cmd) const final;
    async::task<void>               handle_start() override final;
    [[nodiscard]] async::task<bool> handle_connected(fb::socket<fb::game::character>& ch) override final;
    [[nodiscard]] async::task<bool> handle_disconnected(fb::socket<fb::game::character>& ch) override final;
    fb::game::character*            handle_accepted(fb::socket<fb::game::character>& socket) override final;

protected:
    uint8_t id() const
    {
        return fb::config<uint8_t>("id");
    }
    Service service() const
    {
        return Service::Game;
    }

public:
    void handle_click_mob(fb::game::character& ch, fb::game::mob& mob);
    void handle_click_npc(fb::game::character& ch, fb::game::npc& npc);

public:
    [[nodiscard]] async::task<bool> handle_login(fb::socket<fb::game::character>&, const fb_reqs::login&);
    [[nodiscard]] async::task<bool> handle_direction(fb::socket<fb::game::character>&, const fb_reqs::direction&);
    [[nodiscard]] async::task<bool> handle_logout(fb::socket<fb::game::character>&, const fb_reqs::exit&);
    [[nodiscard]] async::task<bool> handle_move(fb::socket<fb::game::character>&, const fb_reqs::move&);
    [[nodiscard]] async::task<bool> handle_update_move(fb::socket<fb::game::character>&, const fb_reqs::update_move&);
    [[nodiscard]] async::task<bool> handle_attack(fb::socket<fb::game::character>&, const fb_reqs::attack&);
    [[nodiscard]] async::task<bool> handle_pickup(fb::socket<fb::game::character>&, const fb_reqs::pick_up&);
    [[nodiscard]] async::task<bool> handle_emotion(fb::socket<fb::game::character>&, const fb_reqs::emotion&);
    [[nodiscard]] async::task<bool> handle_update_map(fb::socket<fb::game::character>&, const fb_reqs::map_update&);
    [[nodiscard]] async::task<bool> handle_update_screen(fb::socket<fb::game::character>&,
                                                         const fb_reqs::update_screen&);
    [[nodiscard]] async::task<bool> handle_active_item(fb::socket<fb::game::character>&, const fb_reqs::item_active&);
    [[nodiscard]] async::task<bool> handle_inactive_item(fb::socket<fb::game::character>&,
                                                         const fb_reqs::item_inactive&);
    [[nodiscard]] async::task<bool> handle_drop_item(fb::socket<fb::game::character>&, const fb_reqs::item_drop&);
    [[nodiscard]] async::task<bool> handle_drop_cash(fb::socket<fb::game::character>&, const fb_reqs::item_drop_cash&);
    [[nodiscard]] async::task<bool> handle_front_info(fb::socket<fb::game::character>&, const fb_reqs::front_info&);
    [[nodiscard]] async::task<bool> handle_self_info(fb::socket<fb::game::character>&, const fb_reqs::self_info&);
    [[nodiscard]] async::task<bool> handle_option_changed(fb::socket<fb::game::character>&,
                                                          const fb_reqs::update_option&);
    [[nodiscard]] async::task<bool> handle_click_object(fb::socket<fb::game::character>&, const fb_reqs::click&);
    [[nodiscard]] async::task<bool> handle_item_info(fb::socket<fb::game::character>&, const fb_reqs::item_info&);
    [[nodiscard]] async::task<bool> handle_itemmix(fb::socket<fb::game::character>&, const fb_reqs::item_mix&);
    [[nodiscard]] async::task<bool> handle_trade(fb::socket<fb::game::character>&, const fb_reqs::trade&);
    [[nodiscard]] async::task<bool> handle_group(fb::socket<fb::game::character>&, const fb_reqs::group&);
    [[nodiscard]] async::task<bool> handle_user_list(fb::socket<fb::game::character>&, const fb_reqs::user_list&);
    [[nodiscard]] async::task<bool> handle_chat(fb::socket<fb::game::character>&, const fb_reqs::chat&);
    [[nodiscard]] async::task<bool> handle_board(fb::socket<fb::game::character>&, const fb_reqs::board&);
    [[nodiscard]] async::task<bool> handle_swap(fb::socket<fb::game::character>&, const fb_reqs::swap&);
    [[nodiscard]] async::task<bool> handle_dialog(fb::socket<fb::game::character>&, const fb_reqs::dialog&);
    [[nodiscard]] async::task<bool> handle_throw_item(fb::socket<fb::game::character>&, const fb_reqs::item_throws&);
    [[nodiscard]] async::task<bool> handle_spell(fb::socket<fb::game::character>&, const fb_reqs::spell_cast&);
    [[nodiscard]] async::task<bool> handle_door(fb::socket<fb::game::character>&, const fb_reqs::door&);
    [[nodiscard]] async::task<bool> handle_whisper(fb::socket<fb::game::character>&, const fb_reqs::whisper&);
    [[nodiscard]] async::task<bool> handle_world(fb::socket<fb::game::character>&, const fb_reqs::map_world&);

public:
    [[nodiscard]] async::task<void> handle_mob_action(const fb::model::datetime& now, std::thread::id id);
    [[nodiscard]] async::task<void> handle_mob_respawn(const fb::model::datetime& now, std::thread::id id);
    [[nodiscard]] async::task<void> handle_buff_timer(const fb::model::datetime& now, std::thread::id id);
    [[nodiscard]] async::task<void> handle_save_timer(const fb::model::datetime& now, std::thread::id id);
    [[nodiscard]] async::task<void> handle_time();
    [[nodiscard]] async::task<void> handle_heart_beat();
    [[nodiscard]] async::task<bool> handle_command(fb::game::character& ch, const std::string& message);

public:
    [[nodiscard]] async::task<bool> handle_command_map(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_sound(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_action(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_weather(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_bright(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_timer(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_effect(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_disguise(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_undisguise(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_mob(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_class(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_level(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_hp(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_mp(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_spell(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_item(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_world(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_script(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_hair(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_hair_color(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_armor_color(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_exit(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_tile(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_save(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_mapobj(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_randmap(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_npc(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_durability(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_concurrency(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_sleep(fb::game::character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_map_tile(character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_ad(character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_web(character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_write_mail(character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_read_mail(character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_command_delete_mail(character& ch, Json::Value& parameters);

public:
    [[nodiscard]] async::task<void> handle_amqp_Pong(const internal_resp::Pong& response);
    [[nodiscard]] async::task<void> handle_amqp_KickOut(const internal_resp::KickOut& response);
    [[nodiscard]] async::task<void> handle_amqp_Whisper(const internal_resp::Whisper& response);
    [[nodiscard]] async::task<void> handle_amqp_EnterGroup(const internal_resp::EnterGroup& response);
    [[nodiscard]] async::task<void> handle_amqp_LeaveGroup(const internal_resp::LeaveGroup& response);
    [[nodiscard]] async::task<void> handle_amqp_SetClanTitle(const internal_resp::SetClanTitle& response);
    [[nodiscard]] async::task<void> handle_amqp_JoinClan(const internal_resp::JoinClan& response);
    [[nodiscard]] async::task<void> handle_amqp_LeaveClan(const internal_resp::LeaveClan& response);
    [[nodiscard]] async::task<void> handle_amqp_BroadcastClan(const internal_resp::BroadcastClan& response);
    [[nodiscard]] async::task<void> handle_amqp_WriteMail(const internal_resp::WriteMail& response);

public:
    void on_create(fb::game::object& me) override final;
    void on_destroy(fb::game::object& me) override final;
    void on_chat(fb::game::object&  me,
                 const std::string& message,
                 CHAT_TYPE          chat_type = CHAT_TYPE::NORMAL) override final;
    void on_direction(fb::game::object& me) override final;
    void on_update_external(fb::game::object& me, bool light) override final;
    void on_update_external(fb::game::object& me, fb::game::object& you, bool light) override final;
    void on_hide(fb::game::object& me, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) override final;
    void on_hide(fb::game::object& me,
                 fb::game::object& you,
                 DESTROY_TYPE      destroy_type = DESTROY_TYPE::DEFAULT) override final;
    void on_move(fb::game::object& me, const fb::model::point16_t& before) override final;
    void on_unbuff(fb::game::object& me, fb::game::buff& buff) override final;
    void on_update_map(character& ch, const fb::game::map& map) override final;
    void on_map_changed(fb::game::object& me, fb::game::map* before, fb::game::map* after) override final;
    void on_update_bgm(character& ch, uint16_t bgm, uint8_t volume) override final;
    void on_sound(fb::game::object& ch, SOUND sound) override final;
    void on_effect(fb::game::object& ch, uint8_t value) override final;

    void on_attack(life& me) override final;
    void on_dead(life& me, object* you) override final;
    void on_update_hp(life& me, uint32_t diff, bool critical) override final;
    void on_action(life& me, ACTION action, DURATION duration, uint8_t sound) override final;
    void on_update(character& me, STATE_LEVEL level) override final;
    void on_message(character& me, const std::string& message, MESSAGE_TYPE type) override final;
    void on_option_changed(character& me, OPTION option, bool enabled) override final;
    void on_update_option(character& me) override final;
    void on_update_map(character&                  ch,
                       const fb::game::map&        map,
                       const fb::model::point16_t& begin,
                       const fb::model::size8_t&   size) override final;
    void on_update_buff(character& ch, const fb::game::buffs& buffs) override final;
    void on_update_internal(character& ch) override final;
    void on_update_time(character& ch, uint16_t hours) override final;
    void on_character_init(character& ch) override final;
    void on_update_position(character& ch) override final;

    void on_browse_character(character& ch, const character& target) override final;
    void on_item_tooltip(character& ch, const fb::game::item& item, uint16_t position) override final;
    void on_show_user_list(character& ch) override final;

    void on_show_board(character& ch) override final;
    void on_show_board(character&                                 ch,
                       const fb::model::board&                    section,
                       const std::list<fb::game::board::article>& articles,
                       BOARD_BUTTON_ENABLE                        flag) override final;
    void on_show_board(character& ch, const fb::game::board::article& value, BOARD_BUTTON_ENABLE flag) override final;
    void on_show_mail_box(character&                                              ch,
                          const std::vector<fb::protocol::internal::MailSummary>& mails,
                          MAIL_BUTTON_ENABLE                                      flag) override final;
    void on_show_mail_box(character&                          ch,
                          const fb::protocol::internal::Mail& mail,
                          MAIL_BUTTON_ENABLE                  flag) override final;
    void on_show_board_message(character& ch, const std::string& message, bool success, bool mail) override final;
    void on_show_world_map(character& ch, uint32_t id, uint16_t index) override final;

    void on_timer(character& ch, uint32_t time, TIMER_TYPE type) override final;
    void on_weather(character& ch, WEATHER_TYPE weather) override final;
    void on_bright(character& ch, uint8_t value) override final;
    void on_update_id(character& ch) override final;

    void                            on_level_up(character& me) override final;
    [[nodiscard]] async::task<bool> on_transfer(character&                  me,
                                                fb::game::map&              map,
                                                const fb::model::point16_t& position) override final;
    void                            on_item_remove(character& me, uint8_t index, ITEM_DELETE_TYPE attr) override final;
    void                            on_item_update(character& me, uint8_t index) override final;
    void                            on_item_swap(character& me, uint8_t src, uint8_t dst) override final;
    void                            on_equipment_on(character& me, item& item, EQUIPMENT_PARTS parts) override final;
    void on_equipment_off(character& me, EQUIPMENT_PARTS parts, uint8_t index) override final;
    void on_item_active(character& me, item& item) override final;
    void on_item_throws(character& me, item& item, const fb::model::point16_t& to) override final;
    void on_spell_update(life& me, uint8_t index) override final;
    void on_spell_remove(life& me, uint8_t index) override final;
    void on_trade_begin(character& me, character& you) override final;
    void on_trade_bundle(character& me) override final;
    void on_trade_item(character& me, character& from, uint8_t index, const fb::game::item& item) override final;
    void on_trade_money(character& me, character& you, uint32_t money) override final;
    void on_trade_cancel(character& me, character& you) override final;
    void on_trade_lock(character& me, character& you) override final;
    void on_trade_failed(character& me, character& you) override final;
    void on_trade_success(character& me, character& you) override final;
    void on_dialog(character&                    me,
                   const fb::model::object&      object,
                   const std::string&            message,
                   bool                          button_prev,
                   bool                          button_next,
                   fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) override final;
    void on_dialog(character&                      me,
                   const fb::model::npc&           npc,
                   const std::string&              message,
                   const std::vector<std::string>& menus,
                   fb::game::dialog::interaction   interaction = fb::game::dialog::interaction::NORMAL) override final;
    void on_dialog(character&                    me,
                   const fb::model::npc&         npc,
                   const std::string&            message,
                   const std::vector<uint8_t>&   item_slots,
                   fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) override final;
    void on_dialog(character&                          me,
                   const fb::model::npc&               npc,
                   const std::string&                  message,
                   const fb::game::dialog::item_pairs& pairs,
                   uint16_t                            pursuit = 0xFFFF,
                   fb::game::dialog::interaction interaction   = fb::game::dialog::interaction::NORMAL) override final;
    void on_dialog(character&                    me,
                   const fb::model::npc&         npc,
                   const std::string&            message,
                   fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) override final;
    void on_dialog(character&                    me,
                   const fb::model::npc&         npc,
                   const std::string&            message,
                   const std::string&            top,
                   const std::string&            bottom,
                   int                           maxlen      = 0xFF,
                   bool                          prev        = false,
                   fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) override final;

public:
    bool npc_interaction_sell(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool npc_interaction_buy(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool npc_interaction_repair(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool npc_interaction_deposit_money(character&                         ch,
                                       const std::string&                 message,
                                       const std::vector<fb::game::npc*>& npcs);
    bool npc_interaction_withdraw_money(character&                         ch,
                                        const std::string&                 message,
                                        const std::vector<fb::game::npc*>& npcs);
    bool npc_interaction_deposit_item(character&                         ch,
                                      const std::string&                 message,
                                      const std::vector<fb::game::npc*>& npcs);
    bool npc_interaction_withdraw_item(character&                         ch,
                                       const std::string&                 message,
                                       const std::vector<fb::game::npc*>& npcs);
    bool npc_interaction_sell_list(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool npc_interaction_buy_list(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool npc_interaction_sell_price(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool npc_interaction_buy_price(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool npc_interaction_show_deposited_money(character&                         ch,
                                              const std::string&                 message,
                                              const std::vector<fb::game::npc*>& npcs);
    bool npc_interaction_rename_weapon(character&                         ch,
                                       const std::string&                 message,
                                       const std::vector<fb::game::npc*>& npcs);
    bool npc_interaction_hold_item_list(character&                         ch,
                                        const std::string&                 message,
                                        const std::vector<fb::game::npc*>& npcs);
    bool npc_interaction_hold_item_count(character&                         ch,
                                         const std::string&                 message,
                                         const std::vector<fb::game::npc*>& npcs);
    bool npc_interaction(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs);

public:
    static int builtin_seed(lua_State* lua);
    static int builtin_sleep(lua_State* lua);
    static int builtin_name2mob(lua_State* lua);
    static int builtin_name2npc(lua_State* lua);
    static int builtin_name2map(lua_State* lua);
    static int builtin_name2item(lua_State* lua);
    static int builtin_pursuit_sell(lua_State* lua);
    static int builtin_pursuit_buy(lua_State* lua);
    static int builtin_sell_price(lua_State* lua);
    static int builtin_buy_price(lua_State* lua);
    static int builtin_timer(lua_State* lua);
    static int builtin_weather(lua_State* lua);
    static int builtin_name_with(lua_State* lua);
    static int builtin_assert_korean(lua_State* lua);
    static int builtin_cp949(lua_State* lua);
    static int builtin_broadcast(lua_State* lua);
};

} // namespace fb::game

template <typename T, typename... Args>
T* fb::model::object::make(fb::game::context& context, Args&&... args) const
{
    auto& model = static_cast<const typename T::model_type&>(*this);
    return context.template make<T>(model, std::forward<Args>(args)...);
}

#endif // !__FB_GAME_H__
