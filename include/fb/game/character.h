#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include <algorithm>
#include <fb/game/dialog.h>
#include <fb/locker.h>
#include <fb/socket.h>
#include <iostream>
#include <fb/game/mob.h>
#include <sstream>
#include <fb/game/trade.h>
#include <fb/game/trace.h>
#include <fb/game/clan.h>
#include <fb/game/board.h>

namespace fb::game {

class map;
class clan;
class group;

using group_lock        = fb::locker<fb::game::group>;
using shared_group_lock = std::shared_ptr<group_lock>;

using clan_lock        = fb::locker<fb::game::clan>;
using shared_clan_lock = std::shared_ptr<clan_lock>;

class character : public life
{
    friend class group;
    friend class clan;

public:
    using fb::game::object::map;

public:
    class container;

public:
    LUA_PROTOTYPE

public:
    struct listener;

private:
    bool                    _init = false;
    uint32_t                _id   = 0xFFFFFFFF;
    fb::socket<character>&  _socket;
    bool                    _admin = false;
    std::string             _name;
    std::string             _pw;
    fb::model::datetime     _updated_date;
    uint16_t                _look               = 0;
    uint8_t                 _color              = 0;
    std::optional<uint8_t>  _armor_color        = 0;
    int16_t                 _defensive_physical = 0;
    int16_t                 _defensive_magical  = 0;
    uint32_t                _base_hp            = 0;
    uint32_t                _base_mp            = 0;
    uint32_t                _experience         = 0;
    uint8_t                 _strength           = 0;
    uint8_t                 _intelligence       = 0;
    uint8_t                 _dexteritry         = 0;
    uint8_t                 _damage             = 0; // 공격수정
    uint8_t                 _hit                = 0; // 명중수정
    uint8_t                 _regenerative       = 0; // 재생력
    NATION                  _nation             = NATION::GOGURYEO;
    CREATURE                _creature           = CREATURE::DRAGON;
    SEX                     _sex                = SEX::ALL;
    STATE                   _state              = STATE::NORMAL;
    uint8_t                 _level              = 1;
    CLASS                   _class              = CLASS::NONE;
    uint8_t                 _promotion          = 0;
    uint32_t                _money              = 0;
    std::optional<uint16_t> _disguise           = 0;
    uint32_t                _deposited_money    = 0;
    std::vector<item*>      _deposited_items;
    std::string             _title;
    shared_group_lock       _group             = nullptr;
    shared_clan_lock        _clan              = nullptr;
    uint16_t                _unread_mail       = 0;
    bool                    _options[0x0B + 1] = {
        0,
    };

public:
    fb::game::trade                            trade  = fb::game::trade(*this);
    fb::game::items                            items  = fb::game::items(*this);
    fb::game::dialog                           dialog = fb::game::dialog(*this);
    std::map<uint32_t, std::unique_ptr<trace>> traces; // order required

private:
    using object::based;

public:
    character(fb::game::context& context, fb::socket<fb::game::character>& socket);
    ~character();

private:
    uint32_t limited_exp(uint32_t exp) const;

public:
    async::task<size_t> send(const fb::stream& stream, bool encrypt = true, bool wrap = true) override final;
    async::task<size_t> send(const fb::protocol::header& response,
                             bool                        encrypt = true,
                             bool                        wrap    = true) override final;
    OBJECT_TYPE         what() const override final;
    [[nodiscard]] async::task<bool> map(fb::game::map*              map,
                                        const fb::model::point16_t& position = fb::model::point16_t{0, 0},
                                        DESTROY_TYPE destroy_type            = DESTROY_TYPE::DEFAULT) override final;

public:
    operator fb::socket<fb::game::character>& ();

public:
    bool                       inited() const;
    void                       init(bool value);
    uint32_t                   id() const;
    void                       id(uint32_t id);
    uint32_t                   fd();
    bool                       admin() const;
    void                       admin(bool value);
    bool                       transferring() const;
    void                       attack() override final;
    uint32_t                   auto_attack_damage(MOB_SIZE size) const override final;
    void                       action(ACTION action, DURATION duration, uint8_t sound = 0x00) override final;
    const std::string&         name() const override final;
    void                       name(const std::string& value);
    void                       pw(const std::string& value);
    const fb::model::datetime& updated_date() const;
    void                       updated_date(const fb::model::datetime& value);
    uint16_t                   look() const override final;
    void                       look(uint16_t value);
    uint8_t                    color() const override final;
    void                       color(uint8_t value);
    std::optional<uint8_t>     armor_color() const;
    void                       armor_color(std::optional<uint8_t> value);
    uint8_t                    current_armor_color() const;
    std::optional<uint16_t>    disguise() const;
    void                       disguise(uint16_t value);
    void                       undisguise();
    uint32_t                   defensive_physical() const;
    void                       defensive_physical(uint8_t value);
    uint32_t                   defensive_magical() const;
    void                       defensive_magical(uint8_t value);
    void                       base_hp_up(uint32_t value);
    void                       base_mp_up(uint32_t value);
    void                       base_hp(uint32_t value);
    void                       base_mp(uint32_t value);
    uint32_t                   base_hp() const;
    uint32_t                   base_mp() const;
    NATION                     nation() const;
    bool                       nation(NATION value);
    CREATURE                   creature() const;
    bool                       creature(CREATURE value);
    uint8_t                    level() const;
    void                       level(uint8_t value);
    bool                       level_up();
    bool                       max_level() const;
    SEX                        sex() const;
    void                       sex(SEX value);
    STATE                      state() const;
    void                       state(STATE value);
    CLASS                      cls() const;
    void                       cls(CLASS value);
    uint8_t                    promotion() const;
    void                       promotion(uint8_t value);
    uint8_t                    strength() const;
    void                       strength(uint8_t value);
    void                       strength_up(uint8_t value);
    uint8_t                    intelligence() const;
    void                       intelligence(uint8_t value);
    void                       intelligence_up(uint8_t value);
    uint8_t                    dexteritry() const;
    void                       dexteritry(uint8_t value);
    void                       dexteritry_up(uint8_t value);
    uint32_t                   exp() const;
    void                       exp(uint32_t value);
    uint32_t                   add_exp(uint32_t value, bool limit = false, bool notify = false);
    uint32_t                   reduce_exp(uint32_t value);
    uint32_t                   experience_remained() const;
    float                      experience_percent() const;
    uint32_t                   money() const;
    void                       money(uint32_t value);
    uint32_t                   money_add(uint32_t value);
    uint32_t                   money_reduce(uint32_t value);
    uint32_t                   money_drop(uint32_t value);
    uint32_t                   deposited_money() const;
    void                       deposited_money(uint32_t value);
    uint32_t                   deposit_money(uint32_t value);
    uint32_t                   withdraw_money(uint32_t value);
    bool                       deposit_item(fb::game::item& item);
    bool                       deposit_item(uint8_t index, uint16_t count);
    bool                       deposit_item(const std::string& name, uint16_t count);
    fb::game::item*            deposited_item(const fb::model::item& item) const;
    const std::vector<item*>&  deposited_items() const;
    fb::game::item*            withdraw_item(uint8_t index, uint16_t count);
    fb::game::item*            withdraw_item(const std::string& name, uint16_t count);
    fb::game::item*            withdraw_item(const fb::model::item& item, uint16_t count);
    uint32_t                   damage() const;
    void                       damage(uint8_t value);
    uint32_t                   hit() const;
    void                       hit(uint8_t value);
    uint32_t                   regenerative() const;
    void                       regenerative(uint8_t value);
    bool                       option(OPTION key) const;
    void                       option(OPTION key, bool value, bool notify = true);
    bool                       option_toggle(OPTION key, bool notify = true);
    void                       update_option();
    void update_map(const fb::game::map& map) override final;
    void update_map(const fb::game::map& map, const fb::model::point16_t& begin, const fb::model::size8_t& size);
    void update_bgm(uint16_t bgm, uint8_t volume) override final;
    void update_buff();
    void update_internal();
    void update_time(uint16_t hours);
    void init();
    void update_position() override final;

    const std::string&       title() const;
    void                     title(const std::string& value);
    const shared_group_lock& group() const;
    shared_group_lock&       group();
    void                     group(shared_group_lock& value);
    shared_clan_lock&        clan();
    const shared_clan_lock&  clan() const;
    void                     clan(shared_clan_lock& value);
    void                     assert_state(STATE value) const;
    void                     assert_state(const std::vector<STATE>& values) const;
    bool                     move(const fb::model::point16_t& before);
    bool                     move(DIRECTION direction, const fb::model::point16_t& before);
    void                     ride(fb::game::mob& horse);
    void                     ride();
    void                     unride();
    bool                     alive() const;
    void                     refresh_map();
    bool                     condition(const std::vector<fb::model::dsl>& conditions) const override final;
    void                     message(const std::string& message, MESSAGE_TYPE type = MESSAGE_TYPE::STATE);
    fb::thread*              thread() const override final;
    void                     assert_thread() const override final;
    void                     update(STATE_LEVEL value = STATE_LEVEL::LEVEL_MIN) override final;
    uint16_t                 unread_mail() const;
    void                     unread_mail(uint16_t value);

    void browse_ch(const character& ch);
    void item_tooltip(const fb::game::item& iteem, uint16_t position);
    void show_user_list();
    void show_board();
    void show_board(const fb::model::board&                    section,
                    const std::list<fb::game::board::article>& articles,
                    BOARD_BUTTON_ENABLE                        flag);
    void show_board(const fb::game::board::article& article, BOARD_BUTTON_ENABLE flag);
    void show_mail_box(const std::vector<fb::protocol::internal::MailSummary>& mails, MAIL_BUTTON_ENABLE flag);
    void show_mail_box(const fb::protocol::internal::Mail& mail, MAIL_BUTTON_ENABLE flag);
    void show_board_message(const std::string& message, bool success, bool mail);
    void show_world_map(uint32_t id, uint16_t index);
    void timer(uint32_t time, TIMER_TYPE type);
    void weather(WEATHER_TYPE weather);
    void bright(uint8_t value);
    void update_id() override final;

private:
    bool inline_sell(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool inline_buy(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool inline_repair(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool inline_deposit_money(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool inline_withdraw_money(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool inline_deposit_item(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool inline_withdraw_item(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool inline_sell_list(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool inline_buy_list(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool inline_sell_price(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool inline_buy_price(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool inline_show_deposited_money(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool inline_rename_weapon(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool inline_hold_item_list(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    bool inline_hold_item_count(const std::string& message, const std::vector<fb::game::npc*>& npcs);

public:
    bool inline_interaction(const std::string& message, const std::vector<fb::game::npc*>& npcs);
    fb::protocol::internal::Character to_protocol() const;

public:
    static int builtin_look(lua_State* lua);
    static int builtin_color(lua_State* lua);
    static int builtin_money(lua_State* lua);
    static int builtin_exp(lua_State* lua);
    static int builtin_base_hp(lua_State* lua);
    static int builtin_base_mp(lua_State* lua);
    static int builtin_strength(lua_State* lua);
    static int builtin_dexterity(lua_State* lua);
    static int builtin_intelligence(lua_State* lua);
    static int builtin_item(lua_State* lua);
    static int builtin_items(lua_State* lua);
    static int builtin_item_drop(lua_State* lua);
    static int builtin_mkitem(lua_State* lua);
    static int builtin_rmitem(lua_State* lua);
    static int builtin_state(lua_State* lua);
    static int builtin_disguise(lua_State* lua);
    static int builtin_class(lua_State* lua);
    static int builtin_level(lua_State* lua);
    static int builtin_assert(lua_State* lua);
    static int builtin_admin(lua_State* lua);
    static int builtin_deposited_money(lua_State* lua);
    static int builtin_deposited_item(lua_State* lua);
    static int builtin_deposit_item(lua_State* lua);
    static int builtin_withdraw_item(lua_State* lua);
    static int builtin_group(lua_State* lua);
    static int builtin_create_group(lua_State* lua);
    static int builtin_clan(lua_State* lua);
    static int builtin_create_clan(lua_State* lua);
    static int builtin_destroy_clan(lua_State* lua);
    static int builtin_traces(lua_State* lua);
    static int builtin_trace(lua_State* lua);
    static int builtin_push_trace(lua_State* lua);
    static int builtin_erase_trace(lua_State* lua);
    static int builtin_switch_context(lua_State* lua);
};

class character::container : private std::vector<fb::game::character*>
{
public:
    using std::vector<fb::game::character*>::begin;
    using std::vector<fb::game::character*>::end;
    using std::vector<fb::game::character*>::size;
    using std::vector<fb::game::character*>::operator[];

public:
    container();
    container(const std::vector<fb::game::character*>& right);
    ~container();

public:
    container& push(fb::game::character& ch);
    container& erase(fb::game::character& ch);

public:
    fb::game::character* find(const std::string& name);
    bool                 contains(const fb::game::character& ch) const;

public:
    fb::game::character* operator[] (const std::string& name);
};

struct character::listener : public virtual fb::game::life::listener,
                             public virtual fb::game::dialog::listener,
                             public virtual fb::game::trade::listener,
                             public virtual fb::game::equipment::listener
{
public:
    virtual void on_message(character& me, const std::string& message, MESSAGE_TYPE type = MESSAGE_TYPE::STATE) = 0;

    virtual void on_option_changed(character& me, OPTION option, bool enabled) = 0;

    virtual void on_update_option(character& me) = 0;

    virtual void on_update_map(character& ch, const fb::game::map& map) = 0;

    virtual void on_update_map(character&                  ch,
                               const fb::game::map&        map,
                               const fb::model::point16_t& begin,
                               const fb::model::size8_t&   size) = 0;

    virtual void on_update_bgm(character& ch, uint16_t bgm, uint8_t volume) = 0;

    virtual void on_update_buff(character& ch, const fb::game::buffs& buffs) = 0;

    virtual void on_update_internal(character& ch) = 0;

    virtual void on_update_time(character& ch, uint16_t hours) = 0;

    virtual void on_browse_character(character& ch, const character& target) = 0;

    virtual void on_item_tooltip(character& ch, const fb::game::item& item, uint16_t position) = 0;

    virtual void on_show_user_list(character& ch) = 0;

    virtual void on_show_board(character& ch) = 0;

    virtual void on_show_board(character&                                 ch,
                               const fb::model::board&                    section,
                               const std::list<fb::game::board::article>& articles,
                               BOARD_BUTTON_ENABLE                        flag) = 0;

    virtual void on_show_board(character& ch, const fb::game::board::article& value, BOARD_BUTTON_ENABLE flag) = 0;

    virtual void on_show_mail_box(character&                                              ch,
                                  const std::vector<fb::protocol::internal::MailSummary>& mails,
                                  MAIL_BUTTON_ENABLE                                      flag) = 0;

    virtual void on_show_mail_box(character& ch, const fb::protocol::internal::Mail& mail, MAIL_BUTTON_ENABLE flag) = 0;

    virtual void on_show_board_message(character& ch, const std::string& message, bool success, bool mail) = 0;

    virtual void on_show_world_map(character& ch, uint32_t id, uint16_t index) = 0;

    virtual void on_timer(character& ch, uint32_t time, TIMER_TYPE type) = 0;

    virtual void on_weather(character& ch, WEATHER_TYPE weather) = 0;

    virtual void on_bright(character& ch, uint8_t value) = 0;

    virtual void on_update_id(character& ch) = 0;

    virtual void on_character_init(character& ch) = 0;

    virtual void on_update_position(character& ch) = 0;

    virtual void on_level_up(character& me) = 0;

    virtual void on_update(character& me, STATE_LEVEL level = STATE_LEVEL::LEVEL_MIN) = 0;

    virtual async::task<bool> on_transfer(character& me, fb::game::map& map, const fb::model::point16_t& position) = 0;
};

} // namespace fb::game

#endif // !__CHARACTER_H__
