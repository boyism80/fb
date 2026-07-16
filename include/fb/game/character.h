#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include <fb/game/life.h>
#include <algorithm>
#include <fb/game/dialog.h>
#include <fb/game/character.h>
#include <fb/synchronized.h>
#include <fb/socket.h>
#include <iostream>
#include <fb/game/mob.h>
#include <sstream>
#include <fb/game/trade.h>
#include <fb/game/achievement.h>
#include <fb/game/bulletin.h>
#include <fb/game/mail_box.h>
#include <fb/game/stat.h>
#include <fb/game/quest.h>
#include <fb/game/storage.h>
#include <fb/game/marketplace.h>
#include <fb/game/marriage.h>
#include <fb/game/matchmaker.h>
#include <fb/game/appearance.h>
#include <set>
#include <shared_mutex>
#include <string_view>
#include <unordered_map>
#include <optional>
#include <tuple>
#include <vector>

namespace fb {
class stream;
}

namespace fb::protocol::internal::response {
class Ban;
class Unban;
class Broadcast;
class KickOut;
class StartMaintenance;
} // namespace fb::protocol::internal::response

namespace fb::game {

class map;

class character : public life
{
    friend class group;
    friend class character_stat;
    friend class server;
    friend class mob;

public:
    using object::map;

public:
    class container;

private:
    using mob_vector_t      = std::vector<std::shared_ptr<fb::game::mob>>;
    using achievement_map_t = std::map<uint32_t, std::unique_ptr<achievement>>;

public:
    LUA_PROTOTYPE

public:
    struct listener_t;
    struct initial_params;

    /**
     * Ping state for keepalive: last ping time, token, and whether pong was received.
     * Initially pong_received is true so the first ping is sent after the interval.
     */
    struct ping_state_t
    {
        fb::model::datetime last_ping_time;
        uint32_t            token         = 0;
        bool                pong_received = true;
    };

private:
    const std::string                          _pw;
    const fb::model::datetime                  _created_date;
    const fb::model::datetime                  _updated_date;
    const std::string                          _name;
    ROLE                                       _role;
    std::optional<uint32_t>                    _birthday;
    uint16_t                                   _look             = 0;
    uint8_t                                    _color            = 0;
    std::optional<uint8_t>                     _armor_color      = 0;
    std::optional<uint8_t>                     _weapon_color     = std::nullopt;
    std::optional<uint8_t>                     _shield_color     = std::nullopt;
    uint32_t                                   _experience       = 0;
    NATION                                     _nation           = NATION::GOGURYEO;
    CREATURE                                   _creature         = CREATURE::DRAGON;
    GENDER                                     _gender           = GENDER::MALE;
    STATE                                      _state            = STATE::NORMAL;
    uint8_t                                    _level            = 1;
    CLASS                                      _class            = CLASS::NONE;
    uint8_t                                    _promotion        = 0;
    uint32_t                                   _money            = 0;
    std::optional<character_appearance>        _mimicry          = std::nullopt;
    std::string                                _title            = "";
    std::optional<uint32_t>                    _group_id         = std::nullopt;
    std::optional<uint32_t>                    _clan_id          = std::nullopt;
    uint16_t                                   _weapon_damage    = 0;
    bool                                       _detect           = false;
    mob_vector_t                               _spawned_mobs     = {};
    bool                                       _super_hide       = false;
    mutable std::optional<fb::model::datetime> _first_login_date = std::nullopt;
    fb::model::datetime                        _last_afk_time;
    fb::game::marriage                         _marriage;
    bool                                       _options[0x0B + 1] = {
        1,
    };
    std::weak_ptr<fb::socket<character>> _socket;
    ping_state_t                         _ping_state;

public:
    const uint32_t        id;
    fb::game::trade       trade;
    fb::game::items       items;
    fb::game::quests      quests;
    fb::game::bulletin    bulletin = fb::game::bulletin(*this);
    fb::game::mail_box    mail_box = fb::game::mail_box(*this);
    fb::game::storage_box storage_box;
    fb::game::marketplace marketplace;
    fb::game::matchmaker  matchmaker;
    fb::lua::context*     dialog = nullptr;
    achievement_map_t     achievements;
    listener_t&           listener;
    character_stat        stat;

private:
    using object::based;

public:
    struct initial_params : fb::game::life::initial_params
    {
    public:
        std::shared_ptr<fb::socket<character>> socket;
        uint32_t                               id = 0;
        std::string                            name;
        std::string                            pw;
        std::optional<uint32_t>                birthday = std::nullopt;
        fb::model::datetime                    created_date;
        fb::model::datetime                    updated_date;
        std::optional<fb::model::datetime>     first_login_date = std::nullopt;
        ROLE                                   role             = ROLE::USER;
        CLASS                                  class_type       = CLASS::NONE;
        uint8_t                                promotion        = 0;
        uint16_t                               color            = 0;
        DIRECTION                              direction        = DIRECTION::BOTTOM;
        uint16_t                               look             = 0;
        uint32_t                               money            = 0;
        GENDER                                 gender           = GENDER::MALE;
        uint8_t                                level            = 1;
        uint32_t                               exp              = 0;
        STATE                                  state            = STATE::NORMAL;
        std::string                            title;
        std::optional<uint8_t>                 armor_color  = std::nullopt;
        std::optional<uint8_t>                 weapon_color = std::nullopt;
        std::optional<uint8_t>                 shield_color = std::nullopt;
        std::optional<character_appearance>    mimicry      = std::nullopt;
        NATION                                 nation       = NATION::GOGURYEO;
        CREATURE                               creature     = CREATURE::DRAGON;
        bool                                   super_hide   = false;
    };

public:
    // clang-format off
    character(fb::game::server& server, const initial_params& params);
   ~character();
    // clang-format on

private:
    uint32_t limited_exp(uint32_t exp) const;

public:
    // clang-format off
    void                                     on_init() override final;
    size_t                                   send(const fb::stream& stream, bool encrypt = true, bool wrap = true) override final;
    size_t                                   send(const fb::protocol::header& response, bool encrypt = true, bool wrap = true) override final;
    [[nodiscard]] async::task<size_t>        send_immediate(const fb::stream& stream, bool encrypt = true, bool wrap = true);
    [[nodiscard]] async::task<size_t>        send_immediate(const fb::protocol::header& response, bool encrypt = true, bool wrap = true);
    OBJECT_TYPE                              what() const override final;
    async::task<void>                        attack(DURATION duration = DURATION::ATTACK) override final;
    uint32_t                                 normal_attack_damage(MOB_SIZE size) const override final;
    void                                     action(ACTION action, DURATION duration, uint8_t sound = 0x00) override final;
    const std::string&                       name() const override final;
    bool                                     is_first_login() const { return !_first_login_date.has_value(); }
    [[nodiscard]] const fb::model::datetime& created_date() const { return this->_created_date; }
    uint16_t                                 look() const override final;
    uint8_t                                  color() const override final;
    [[nodiscard]] async::task<bool>          map(std::shared_ptr<fb::game::map> map, std::optional<fb::model::point16_t> position = std::nullopt, map_options options = {}) override final;
    void                                     update_map(const fb::game::map& map) override final;
    void                                     update_bgm(uint16_t bgm, uint8_t volume) override final;
    void                                     update_position() override final;
    bool                                     condition(const std::vector<fb::model::dsl>& conditions) const override final;
    fb::thread*                              thread() const override final;
    void                                     assert_thread() const override final;
    void                                     update(UPDATE_STATE_LEVEL value = UPDATE_STATE_LEVEL::EXP_MONEY | UPDATE_STATE_LEVEL::CROWD_CONTROL) override final;
    void                                     kill(DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) override final;
    void                                     notify_death(std::shared_ptr<fb::game::object> killer = nullptr);
    async::task<void>                        damage_to(const damage_list& targets) override final;
    async::task<void>                        damage_to(const damage_list& targets, const damage_opts& opts) override final;
    async::task<void>                        settle_kills(mob_vector dead);
    void                                     award_exp(const fb::game::mob& mob);
    void                                     update_id() override final;
    bool                                     super_hide() const override final;
    bool                                     hidden(const fb::game::object& target) const override final;
    std::shared_ptr<fb::game::appearance>    appearance() const override;
    // clang-format on

public:
    // clang-format off
    bool                                                      inited() const;
    ROLE                                                      role() const;
    void                                                      role(ROLE value);
    const std::optional<uint32_t>&                            birthday() const;
    void                                                      birthday(const std::optional<uint32_t>& value);
    void                                                      look(uint16_t value);
    void                                                      color(uint8_t value);
    std::optional<uint8_t>                                    armor_color() const;
    void                                                      armor_color(std::optional<uint8_t> value);
    std::optional<uint8_t>                                    weapon_color() const;
    void                                                      weapon_color(std::optional<uint8_t> value);
    std::optional<uint8_t>                                    shield_color() const;
    void                                                      shield_color(std::optional<uint8_t> value);
    const std::optional<character_appearance>&                mimicry() const;
    void                                                      mimicry(std::optional<character_appearance> value);
    NATION                                                    nation() const;
    bool                                                      nation(NATION value);
    CREATURE                                                  creature() const;
    bool                                                      creature(CREATURE value);
    uint8_t                                                   level() const;
    void                                                      level(uint8_t value);
    bool                                                      level_up();
    bool                                                      max_level() const;
    GENDER                                                    gender() const;
    void                                                      gender(GENDER value);
    STATE                                                     state() const;
    STATE                                                     state_to(const fb::game::object& to, STATE state) const;
    void                                                      state(STATE value);
    CLASS                                                     cls() const;
    void                                                      cls(CLASS value);
    uint8_t                                                   promotion() const;
    void                                                      promotion(uint8_t value);
    uint32_t                                                  exp() const;
    void                                                      exp(uint32_t value);
    uint32_t                                                  add_exp(uint32_t value, bool limit = false, bool notify = false);
    uint32_t                                                  reduce_exp(uint32_t value);
    uint32_t                                                  experience_remained() const;
    float                                                     experience_percent() const;
    uint32_t                                                  money() const;
    void                                                      money(uint32_t value);
    uint32_t                                                  money_add(uint32_t value);
    void                                                      money_reduce(uint32_t value);
    [[nodiscard]] async::task<fb::game::cash*>                money_drop(uint32_t value);
    bool                                                      option(OPTION key) const;
    void                                                      option(OPTION key, bool value, bool notify = true);
    bool                                                      option_toggle(OPTION key, bool notify = true);
    void                                                      update_option();
    void                                                      update_map();
    void                                                      update_map(const fb::game::map& map, const fb::model::point16_t& begin, const fb::model::size8_t& size, uint16_t crc = 0);
    void                                                      update_buff();
    void                                                      update_internal();
    void                                                      update_time(uint16_t hours);
    void                                                      init();
    void                                                      screen_refresh();
    const std::string&                                        title() const;
    void                                                      title(std::string_view value);
    const std::optional<uint32_t>&                            group_id() const;
    std::optional<uint32_t>&                                  group_id();
    void                                                      group_id(uint32_t gid);
    void                                                      group_reset();
    const std::optional<uint32_t>&                            clan_id() const;
    void                                                      clan_id(std::optional<uint32_t> value);
    void                                                      clan_reset();
    void                                                      assert_state(STATE value) const;
    void                                                      assert_state(const std::vector<STATE>& values) const;
    bool                                                      move(const fb::model::point16_t& before);
    bool                                                      move(DIRECTION direction, const fb::model::point16_t& before);
    [[nodiscard]] async::task<void>                           ride(mob& horse);
    [[nodiscard]] async::task<void>                           ride();
    [[nodiscard]] async::task<void>                           unride();
    bool                                                      alive() const;
    void                                                      message(std::string_view message, MESSAGE_TYPE type = MESSAGE_TYPE::STATE);
    async::task<void>                                         whisper(std::string receiver_name, std::string message);
    void                                                      thread(fb::thread* value);
    void                                                      browse_ch(const character& ch);
    void                                                      item_tooltip(const item& iteem, uint16_t position);
    async::task<void>                                         show_user_list();
    void                                                      show_world_map(uint32_t id, uint16_t index);
    void                                                      timer(uint32_t time, TIMER_TYPE type);
    void                                                      weather(WEATHER_TYPE weather);
    void                                                      bright(uint8_t value);
    void                                                      ping(uint32_t token);
    void                                                      save_ack();
    void                                                      bulk_objects_update(const std::vector<object*>& objects);
    void                                                      ad(uint32_t width, uint32_t height, std::string_view url, uint8_t time);
    void                                                      web(uint8_t type, std::string_view url, std::string_view message);
    void                                                      ui(uint8_t screen);
    void                                                      item_throw_confirm(uint8_t slot);
    void                                                      freeze(bool value);
    void                                                      friends_sync(uint8_t enabled);
    void                                                      holyday_screen(uint8_t screen, uint8_t hair, fb::model::enum_value::DIRECTION direction, const fb::model::point<uint8_t>& position);
    void                                                      weapon_damage(uint16_t value);
    uint16_t                                                  weapon_damage() const;
    void                                                      detect(bool value);
    bool                                                      detect() const;
    [[nodiscard]] async::task<std::shared_ptr<fb::game::mob>> spawn_mob(const fb::model::mob& model, const fb::model::point16_t& position, bool owned = true, bool notify = true);
    const std::vector<std::shared_ptr<fb::game::mob>>&        spawned_mobs() const;
    bool                                                      detach_spawned_mob(fb::game::mob& mob);
    async::task<void>                                         death_penalty();
    [[nodiscard]] async::task<bool>                           reward(const std::vector<fb::model::dsl>& reward);
    fb::protocol::internal::Character                         to_protocol() const;
    const fb::game::marriage&                                 marriage() const;
    void                                                      marriage(const fb::game::marriage& value);
    void                                                      super_hide(bool enabled);
    bool                                                      hidden(ROLE role) const;
    void                                                      update_last_afk_time();
    fb::model::datetime&                                      last_afk_time();
    std::shared_ptr<fb::socket<character>>                    socket_ptr() const;
    ping_state_t&                                             ping_state();
    // clang-format on
};

class character::container
{
public:
    using character_ptr_t = std::shared_ptr<fb::game::character>;

private:
    using character_function_t       = std::function<void(character_ptr_t&)>;
    using character_predicate_t      = std::function<bool(const character_ptr_t&)>;
    using character_async_function_t = std::function<async::task<void>(character_ptr_t&)>;
    using character_function_t_miss  = std::function<void(std::string_view name)>;

private:
    mutable std::shared_mutex                        _mutex;
    std::unordered_map<uint32_t, character_ptr_t>    _from_uid;
    std::unordered_map<std::string, character_ptr_t> _from_name;

public:
    fb::game::server& _server;

public:
    container(fb::game::server& server);
    ~container() = default;

public:
    // clang-format off
    size_t                size() const;
    bool                  insert(character_ptr_t ch);
    void                  remove(character_ptr_t ch);
    character_ptr_t       find(uint32_t uid) const;
    character_ptr_t       find(std::string_view name) const;
    bool                  contains(std::string_view name) const;
    bool                  contains(uint32_t uid) const;
    std::vector<uint32_t> collect_ids(character_predicate_t predict = nullptr) const;
    async::task<void>     foreach(character_function_t fn, character_predicate_t predicate = nullptr);
    async::task<void>     foreach(character_function_t fn, const std::vector<character_ptr_t>& characters);
    async::task<void>     foreach_async(character_async_function_t fn, character_predicate_t predict = nullptr) const;
    async::task<void>     foreach_async(character_async_function_t fn, const std::vector<character_ptr_t>& characters) const;
    async::task<void>     foreach(const std::vector<std::string>& names, character_function_t fn, character_function_t_miss miss = nullptr);
    async::task<void>     foreach_async(const std::vector<std::string>& names, character_async_function_t fn, character_function_t_miss miss = nullptr) const;
    void                  foreach_enqueue(character_async_function_t&& fn, character_predicate_t predict = nullptr) const;
    void                  foreach_enqueue(character_async_function_t&& fn, const std::vector<character_ptr_t>& characters) const;
    void                  foreach_enqueue(const std::vector<std::string>& names, character_async_function_t&& fn, character_function_t_miss miss = nullptr) const;
    async::task<void>     invoke(std::string_view name, character_function_t fn, character_function_t_miss miss = nullptr) const;
    async::task<void>     invoke_async(std::string_view name, character_async_function_t fn, character_function_t_miss miss = nullptr) const;
    void                  broadcast(std::string_view message, MESSAGE_TYPE type);
    async::task<void>     broadcast(std::string_view message, MESSAGE_TYPE type, BROADCAST_TYPE broadcast_type);
    async::task<void>     on_broadcast(const fb::protocol::internal::response::Broadcast& resp);
    void                  update_time(uint8_t hours);
    void                  send(const fb::stream& stream, bool encrypt);
    static void           assert_whisper(uint32_t error, std::string_view to);
    void                  on_kick_out(const fb::protocol::internal::response::KickOut& message);
    void                  on_start_maintenance(const fb::protocol::internal::response::StartMaintenance& message);
    static std::string    build_ban_message(std::string_view reason, const std::optional<std::string>& expire_date);
    // clang-format on

public:
    character_ptr_t operator[] (uint32_t uid);
    character_ptr_t operator[] (std::string_view name);
    character_ptr_t operator[] (uint32_t uid) const;
    character_ptr_t operator[] (std::string_view name) const;

private:
    std::vector<character_ptr_t> snapshot(character_predicate_t predict = nullptr) const;
};

struct character::listener_t : public virtual life::listener_t,
                               public virtual dialog::listener_t,
                               public virtual trade::listener_t,
                               public virtual equipment::listener_t
{
public:
    // clang-format off
    virtual void                            on_message(character& me, std::string_view message, MESSAGE_TYPE type = MESSAGE_TYPE::STATE) = 0;
    virtual void                            on_option_changed(character& me, OPTION option, bool enabled) = 0;
    virtual void                            on_update_option(character& me) = 0;
    virtual void                            on_update_map(character& ch, const fb::game::map& map) = 0;
    virtual void                            on_update_map(character& ch, const fb::game::map& map, const fb::model::point16_t& begin, const fb::model::size8_t& size, uint16_t crc) = 0;
    virtual void                            on_update_bgm(character& ch, uint16_t bgm, uint8_t volume) = 0;
    virtual void                            on_update_buff(character& ch, const fb::game::buffs& buffs) = 0;
    virtual void                            on_update_internal(character& ch) = 0;
    virtual void                            on_update_time(character& ch, uint16_t hours) = 0;
    virtual void                            on_browse_character(character& ch, const character& target) = 0;
    virtual void                            on_item_tooltip(character& ch, const item& item, uint16_t position) = 0;
    virtual async::task<void>               on_show_user_list(character& ch) = 0;
    virtual void                            on_show_bulletin(character& ch) = 0;
    virtual void                            on_show_bulletin(character& ch, const fb::model::bulletin& section, const std::list<bulletin::article>& articles, BULLETIN_BUTTON_ENABLE flag) = 0;
    virtual void                            on_show_bulletin(character& ch, const bulletin::article& value, BULLETIN_BUTTON_ENABLE flag) = 0;
    virtual void                            on_show_mail_box(character& ch, const std::vector<mail_box::summary>& mails, MAIL_BUTTON_ENABLE flag) = 0;
    virtual void                            on_show_mail_box(character& ch, const mail_box::mail& mail, MAIL_BUTTON_ENABLE flag) = 0;
    virtual void                            on_show_bulletin_message(character& ch, std::string_view message, bool success, BULLETIN_MESSAGE_TYPE action) = 0;
    virtual void                            on_show_world_map(character& ch, uint32_t id, uint16_t index) = 0;
    virtual void                            on_timer(character& ch, uint32_t time, TIMER_TYPE type) = 0;
    virtual void                            on_weather(character& ch, WEATHER_TYPE weather) = 0;
    virtual void                            on_bright(character& ch, uint8_t value) = 0;
    virtual void                            on_update_id(character& ch) = 0;
    virtual void                            on_character_init(character& ch) = 0;
    virtual void                            on_update_position(character& ch) = 0;
    virtual void                            on_screen_refresh(character& ch) = 0;
    virtual void                            on_level_up(character& me) = 0;
    virtual void                            on_update(character& me, UPDATE_STATE_LEVEL level = UPDATE_STATE_LEVEL::EXP_MONEY | UPDATE_STATE_LEVEL::CROWD_CONTROL) = 0;
    [[nodiscard]] virtual async::task<void> on_transfer(character& me, fb::game::map& map, const fb::model::point16_t& position, std::string_view ip, uint16_t port) = 0;
    virtual void                            on_ping(character& ch, uint32_t token) = 0;
    virtual void                            on_save(character& ch) = 0;
    virtual void                            on_bulk_update(character& ch, const std::vector<object*>& objects) = 0;
    virtual void                            on_ad(character& ch, uint32_t width, uint32_t height, std::string_view url, uint8_t time) = 0;
    virtual void                            on_web(character& ch, uint8_t type, std::string_view url, std::string_view message) = 0;
    virtual void                            on_ui(character& ch, uint8_t screen) = 0;
    virtual void                            on_item_throw_confirm(character& ch, uint8_t slot) = 0;
    virtual void                            on_freeze(character& ch, bool value) = 0;
    virtual void                            on_friends_sync(character& ch, uint8_t enabled) = 0;
    virtual void                            on_holyday_screen(character& ch, uint8_t screen, uint8_t hair, fb::model::enum_value::DIRECTION direction, const fb::model::point<uint8_t>& position) = 0;
    // clang-format on
};

} // namespace fb::game

#endif // !__CHARACTER_H__
