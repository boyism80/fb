#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include <fb/game/life.h>
#include <algorithm>
#include <fb/game/dialog.h>
#include <fb/game/character.h>
#include <fb/locker.h>
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
#include <set>
#include <unordered_map>

namespace fb::game {

class map;

class character : public life
{
    friend class group;
    friend class character_stat;
    friend class server;

public:
    using object::map;

public:
    class container;

private:
    using socket_ptr_t      = std::weak_ptr<fb::socket<character>>;
    using mob_vector_t      = std::vector<std::shared_ptr<fb::game::mob>>;
    using achievement_map_t = std::map<uint32_t, std::unique_ptr<achievement>>;

public:
    LUA_PROTOTYPE

public:
    struct listener_t;
    struct initial_params;

private:
    uint32_t                _id;
    socket_ptr_t            _socket;
    std::string             _name;
    ROLE                    _role;
    std::string             _pw;
    std::optional<uint32_t> _birthday;
    fb::model::datetime     _created_date;
    fb::model::datetime     _updated_date;
    uint16_t                _look              = 0;
    uint8_t                 _color             = 0;
    std::optional<uint8_t>  _armor_color       = 0;
    uint32_t                _experience        = 0;
    NATION                  _nation            = NATION::GOGURYEO;
    CREATURE                _creature          = CREATURE::DRAGON;
    SEX                     _sex               = SEX::MAN;
    STATE                   _state             = STATE::NORMAL;
    uint8_t                 _level             = 1;
    CLASS                   _class             = CLASS::NONE;
    uint8_t                 _promotion         = 0;
    uint32_t                _money             = 0;
    std::optional<uint16_t> _disguise          = 0;
    std::string             _title             = "";
    std::optional<uint32_t> _group_id          = std::nullopt;
    std::optional<uint32_t> _clan_id           = std::nullopt;
    uint16_t                _weapon_damage     = 0;
    bool                    _detect            = false;
    mob_vector_t            _spawned_mobs      = {};
    bool                    _super_hide        = false;
    bool                    _options[0x0B + 1] = {
        1,
    };

public:
    fb::game::trade       trade;
    fb::game::items       items;
    fb::game::quests      quests;
    fb::game::bulletin    bulletin = fb::game::bulletin(*this);
    fb::game::mail_box    mail_box = fb::game::mail_box(*this);
    fb::game::storage_box storage_box;
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
        fb::game::server*      server = nullptr;
        fb::socket<character>* socket = nullptr;

        uint32_t                id = 0;
        std::string             name;
        std::string             pw;
        std::optional<uint32_t> birthday = std::nullopt;
        fb::model::datetime     created_date;
        fb::model::datetime     updated_date;
        ROLE                    role            = ROLE::USER;
        CLASS                   class_type      = CLASS::NONE;
        uint8_t                 promotion       = 0;
        uint16_t                color           = 0;
        DIRECTION               direction       = DIRECTION::BOTTOM;
        uint16_t                look            = 0;
        uint32_t                money           = 0;
        uint32_t                deposited_money = 0;
        SEX                     sex             = SEX::MAN;
        uint32_t                base_hp         = 0;
        uint32_t                hp              = 0;
        uint32_t                base_mp         = 0;
        uint32_t                mp              = 0;
        uint8_t                 level           = 1;
        uint32_t                exp             = 0;
        STATE                   state           = STATE::NORMAL;
        std::string             title;
        std::optional<uint8_t>  armor_color = std::nullopt;
        std::optional<uint16_t> disguise    = std::nullopt;
    };

public:
    character(const initial_params& params);
    ~character();

public:
    void on_init() override final;

private:
    uint32_t limited_exp(uint32_t exp) const;

public:
    async::task<size_t> send(const fb::stream& stream, bool encrypt = true, bool wrap = true) override final;
    async::task<size_t> send(const fb::protocol::header& response, bool encrypt = true, bool wrap = true) override final;
    OBJECT_TYPE         what() const override final;
    [[nodiscard]] async::task<bool>
    map(std::shared_ptr<fb::game::map> map, const fb::model::point16_t& position, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT, bool notify = true) override final;

public:
    bool                                               inited() const;
    fb::socket<character>*                             socket() const;
    uint32_t                                           id() const;
    ROLE                                               role() const;
    void                                               role(ROLE value);
    async::task<void>                                  attack(DURATION duration = DURATION::ATTACK) override final;
    uint32_t                                           auto_attack_damage(MOB_SIZE size) const override final;
    void                                               action(ACTION action, DURATION duration, uint8_t sound = 0x00) override final;
    const std::string&                                 name() const override final;
    const std::optional<uint32_t>&                     birthday() const;
    void                                               birthday(const std::optional<uint32_t>& value);
    const fb::model::datetime&                         created_date() const;
    const fb::model::datetime&                         updated_date() const;
    uint16_t                                           look() const override final;
    void                                               look(uint16_t value);
    uint8_t                                            color() const override final;
    void                                               color(uint8_t value);
    std::optional<uint8_t>                             armor_color() const;
    void                                               armor_color(std::optional<uint8_t> value);
    std::optional<uint16_t>                            disguise() const;
    void                                               disguise(uint16_t value);
    void                                               undisguise();
    NATION                                             nation() const;
    bool                                               nation(NATION value);
    CREATURE                                           creature() const;
    bool                                               creature(CREATURE value);
    uint8_t                                            level() const;
    void                                               level(uint8_t value);
    bool                                               level_up();
    bool                                               max_level() const;
    SEX                                                sex() const;
    void                                               sex(SEX value);
    STATE                                              state() const;
    STATE                                              state_to(const fb::game::object& to) const;
    void                                               state(STATE value);
    CLASS                                              cls() const;
    void                                               cls(CLASS value);
    uint8_t                                            promotion() const;
    void                                               promotion(uint8_t value);
    uint32_t                                           exp() const;
    void                                               exp(uint32_t value);
    uint32_t                                           add_exp(uint32_t value, bool limit = false, bool notify = false);
    uint32_t                                           reduce_exp(uint32_t value);
    uint32_t                                           experience_remained() const;
    float                                              experience_percent() const;
    uint32_t                                           money() const;
    void                                               money(uint32_t value);
    uint32_t                                           money_add(uint32_t value);
    void                                               money_reduce(uint32_t value);
    fb::game::cash*                                    money_drop(uint32_t value);
    bool                                               option(OPTION key) const;
    void                                               option(OPTION key, bool value, bool notify = true);
    bool                                               option_toggle(OPTION key, bool notify = true);
    void                                               update_option();
    void                                               update_map(const fb::game::map& map) override final;
    void                                               update_map();
    void                                               update_map(const fb::game::map& map, const fb::model::point16_t& begin, const fb::model::size8_t& size, uint16_t crc = 0);
    void                                               update_bgm(uint16_t bgm, uint8_t volume) override final;
    void                                               update_buff();
    void                                               update_internal();
    void                                               update_time(uint16_t hours);
    void                                               init();
    void                                               update_position() override final;
    const std::string&                                 title() const;
    void                                               title(const std::string& value);
    const std::optional<uint32_t>&                     group_id() const;
    std::optional<uint32_t>&                           group_id();
    void                                               group_id(uint32_t gid);
    void                                               group_reset();
    const std::optional<uint32_t>&                     clan_id() const;
    void                                               clan_id(std::optional<uint32_t> value);
    void                                               clan_reset();
    void                                               assert_state(STATE value) const;
    void                                               assert_state(const std::vector<STATE>& values) const;
    bool                                               move(const fb::model::point16_t& before);
    bool                                               move(DIRECTION direction, const fb::model::point16_t& before);
    void                                               ride(mob& horse);
    void                                               ride();
    void                                               unride();
    bool                                               alive() const;
    bool                                               condition(const std::vector<fb::model::dsl>& conditions) const override final;
    void                                               message(const std::string& message, MESSAGE_TYPE type = MESSAGE_TYPE::STATE);
    async::task<void>                                  process_system_mails();
    fb::thread*                                        thread() const override final;
    void                                               thread(fb::thread* value);
    void                                               assert_thread() const override final;
    void                                               update(UPDATE_STATE_LEVEL value = UPDATE_STATE_LEVEL::EXP_MONEY | UPDATE_STATE_LEVEL::CROWD_CONTROL) override final;
    void                                               browse_ch(const character& ch);
    void                                               item_tooltip(const item& iteem, uint16_t position);
    void                                               show_user_list();
    void                                               show_world_map(uint32_t id, uint16_t index);
    void                                               timer(uint32_t time, TIMER_TYPE type);
    void                                               weather(WEATHER_TYPE weather);
    void                                               bright(uint8_t value);
    void                                               update_id() override final;
    void                                               weapon_damage(uint16_t value);
    uint16_t                                           weapon_damage() const;
    void                                               detect(bool value);
    bool                                               detect() const;
    std::shared_ptr<fb::game::mob>                     spawn_mob(const fb::model::mob& model, const fb::model::point16_t& position, bool owned = true, bool notify = true);
    const std::vector<std::shared_ptr<fb::game::mob>>& spawned_mobs() const;
    bool                                               detach_spawned_mob(fb::game::mob& mob);
    async::task<void>                                  death_penalty();
    bool                                               reward(const std::vector<fb::model::dsl>& reward);
    fb::protocol::internal::Character                  to_protocol() const;
    bool                                               super_hide() const override final;
    void                                               super_hide(bool enabled);
    bool                                               hidden(const fb::game::object& target) const override final;
    bool                                               hidden(ROLE role) const;
};

class character::container
{
private:
    using character_ptr_t            = std::shared_ptr<fb::game::character>;
    using character_function_t       = std::function<void(character_ptr_t&)>;
    using character_predicate_t      = std::function<bool(const character_ptr_t&)>;
    using character_async_function_t = std::function<async::task<void>(character_ptr_t&)>;
    using character_function_t_miss  = std::function<void(const std::string& name)>;

private:
    std::unordered_map<uint32_t, character_ptr_t>    _from_uid;
    std::unordered_map<std::string, character_ptr_t> _from_name;

public:
    using iterator       = std::unordered_map<uint32_t, character_ptr_t>::iterator;
    using const_iterator = std::unordered_map<uint32_t, character_ptr_t>::const_iterator;

public:
    fb::game::server& _server;

public:
    container(fb::game::server& server);
    ~container() = default;

public:
    bool            insert(character_ptr_t ch);
    void            remove(character_ptr_t ch);
    character_ptr_t find(uint32_t uid) const;
    character_ptr_t find(const std::string& name) const;
    bool            contains(const std::string& name) const;
    bool            contains(uint32_t uid) const;
    async::task<void> foreach (character_function_t&& fn, character_predicate_t predicate = nullptr);
    async::task<void> foreach (character_function_t&& fn, const std::vector<character_ptr_t>& characters);
    async::task<void> foreach_async(character_async_function_t&& fn, character_predicate_t predict = nullptr);
    async::task<void> foreach_async(character_async_function_t&& fn, const std::vector<character_ptr_t>& characters);
    async::task<void> foreach (const std::vector<std::string>& names, character_function_t && fn, character_function_t_miss miss = nullptr);
    async::task<void> foreach_async(const std::vector<std::string>& names, character_async_function_t&& fn, character_function_t_miss miss = nullptr);
    async::task<void> invoke(const std::string& name, character_function_t fn, character_function_t_miss miss = nullptr);
    async::task<void> invoke_async(const std::string& name, character_async_function_t fn, character_function_t_miss miss = nullptr);

public:
    character_ptr_t operator[] (uint32_t uid);
    character_ptr_t operator[] (const std::string& name);

public:
    iterator       begin();
    iterator       end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;
};

struct character::listener_t : public virtual life::listener_t, public virtual dialog::listener_t, public virtual trade::listener_t, public virtual equipment::listener_t
{
public:
    virtual void              on_message(character& me, const std::string& message, MESSAGE_TYPE type = MESSAGE_TYPE::STATE)                                                 = 0;
    virtual void              on_option_changed(character& me, OPTION option, bool enabled)                                                                                  = 0;
    virtual void              on_update_option(character& me)                                                                                                                = 0;
    virtual void              on_update_map(character& ch, const fb::game::map& map)                                                                                         = 0;
    virtual void              on_update_map(character& ch, const fb::game::map& map, const fb::model::point16_t& begin, const fb::model::size8_t& size, uint16_t crc)        = 0;
    virtual void              on_update_bgm(character& ch, uint16_t bgm, uint8_t volume)                                                                                     = 0;
    virtual void              on_update_buff(character& ch, const fb::game::buffs& buffs)                                                                                    = 0;
    virtual void              on_update_internal(character& ch)                                                                                                              = 0;
    virtual void              on_update_time(character& ch, uint16_t hours)                                                                                                  = 0;
    virtual void              on_browse_character(character& ch, const character& target)                                                                                    = 0;
    virtual void              on_item_tooltip(character& ch, const item& item, uint16_t position)                                                                            = 0;
    virtual void              on_show_user_list(character& ch)                                                                                                               = 0;
    virtual void              on_show_bulletin(character& ch)                                                                                                                = 0;
    virtual void              on_show_bulletin(character& ch, const fb::model::bulletin& section, const std::list<bulletin::article>& articles, BULLETIN_BUTTON_ENABLE flag) = 0;
    virtual void              on_show_bulletin(character& ch, const bulletin::article& value, BULLETIN_BUTTON_ENABLE flag)                                                   = 0;
    virtual void              on_show_mail_box(character& ch, const std::vector<mail_box::summary>& mails, MAIL_BUTTON_ENABLE flag)                                          = 0;
    virtual void              on_show_mail_box(character& ch, const mail_box::mail& mail, MAIL_BUTTON_ENABLE flag)                                                           = 0;
    virtual void              on_show_bulletin_message(character& ch, const std::string& message, bool success, bool mail)                                                   = 0;
    virtual void              on_show_world_map(character& ch, uint32_t id, uint16_t index)                                                                                  = 0;
    virtual void              on_timer(character& ch, uint32_t time, TIMER_TYPE type)                                                                                        = 0;
    virtual void              on_weather(character& ch, WEATHER_TYPE weather)                                                                                                = 0;
    virtual void              on_bright(character& ch, uint8_t value)                                                                                                        = 0;
    virtual void              on_update_id(character& ch)                                                                                                                    = 0;
    virtual void              on_character_init(character& ch)                                                                                                               = 0;
    virtual void              on_update_position(character& ch)                                                                                                              = 0;
    virtual void              on_level_up(character& me)                                                                                                                     = 0;
    virtual void              on_update(character& me, UPDATE_STATE_LEVEL level = UPDATE_STATE_LEVEL::EXP_MONEY | UPDATE_STATE_LEVEL::CROWD_CONTROL)                         = 0;
    virtual async::task<bool> on_transfer(character& me, fb::game::map& map, const fb::model::point16_t& position)                                                           = 0;
};

} // namespace fb::game

#endif // !__CHARACTER_H__
