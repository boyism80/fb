#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include <algorithm>
#include <fb/game/dialog.h>
#include <fb/string.h>
#include <fb/locker.h>
#include <iostream>
#include <fb/game/mob.h>
#include <sstream>
#include <fb/game/trade.h>
#include <fb/game/trace.h>
#include <fb/game/clan.h>

namespace fb::game {

/**
 * @brief      This class describes a map of .
 */
class map;
/**
 * @brief      This class describes a clan.
 */
class clan;
/**
 * @brief      This class describes a group.
 */
class group;

using group_lock        = fb::locker<fb::game::group>;
using shared_group_lock = std::shared_ptr<group_lock>;

using clan_lock        = fb::locker<fb::game::clan>;
using shared_clan_lock = std::shared_ptr<clan_lock>;

/**
 * @brief      This class describes a character.
 */
class character : public life
{
    friend class group;
    friend class clan;

public:
    using fb::game::object::map;

public:
    /**
     * @brief      This class describes a container.
     */
    class container;

public:
    DECLARE_EXCEPTION(require_class_exception, fb::game::message::exception::REQUIRE_CLASS)
    DECLARE_EXCEPTION(ghost_exception, fb::game::message::exception::GHOST)
    DECLARE_EXCEPTION(ridding_exception, fb::game::message::exception::RIDDING)
    DECLARE_EXCEPTION(no_conveyance_exception, fb::game::message::exception::NO_CONVEYANCE)
    DECLARE_EXCEPTION(disguise_exception, fb::game::message::exception::DISGUISE)

public:
    LUA_PROTOTYPE

public:
    /**
     * @brief      { struct_description }
     */
    struct listener;

private:
    bool                    _init = false;
    uint32_t                _id   = 0xFFFFFFFF;
    fb::socket<character>&  _socket;
    bool                    _admin = false;
    std::string             _name;
    std::string             _pw;
    datetime                _updated_date;
    uint16_t                _look            = 0;
    uint8_t                 _color           = 0;
    std::optional<uint8_t>  _armor_color     = 0;
    defensive               _defensive       = {};
    uint32_t                _base_hp         = 0;
    uint32_t                _base_mp         = 0;
    uint32_t                _experience      = 0;
    uint8_t                 _strength        = 0;
    uint8_t                 _intelligence    = 0;
    uint8_t                 _dexteritry      = 0;
    uint8_t                 _damage          = 0; // 공격수정
    uint8_t                 _hit             = 0; // 명중수정
    uint8_t                 _regenerative    = 0; // 재생력
    NATION                  _nation          = NATION::GOGURYEO;
    CREATURE                _creature        = CREATURE::DRAGON;
    SEX                     _sex             = SEX::ALL;
    STATE                   _state           = STATE::NORMAL;
    uint8_t                 _level           = 1;
    CLASS                   _class           = CLASS::NONE;
    uint8_t                 _promotion       = 0;
    uint32_t                _money           = 0;
    std::optional<uint16_t> _disguise        = 0;
    uint32_t                _deposited_money = 0;
    std::vector<item*>      _deposited_items;
    std::string             _title;
    shared_group_lock       _group             = nullptr;
    shared_clan_lock        _clan              = nullptr;
    bool                    _options[0x0B + 1] = {
        0,
    };

public:
    fb::game::trade                            trade  = fb::game::trade(*this);
    fb::game::items                            items  = fb::game::items(*this);
    fb::game::dialog                           dialog = fb::game::dialog(*this);
    std::map<uint32_t, std::unique_ptr<trace>> traces; // order required
    uint8_t                                    mailed = 0;

private:
    using object::based;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param      socket   The socket
     */
    character(fb::game::context& context, fb::socket<fb::game::character>& socket);

    /**
     * @brief      Destroys the object.
     */
    ~character();

private:
    /**
     * @brief      { function_description }
     *
     * @param[in]  exp   The exponent
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t limited_exp(uint32_t exp) const;

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  stream   The stream
     * @param[in]  encrypt  The encrypt
     * @param[in]  wrap     The wrap
     *
     * @return     { description_of_the_return_value }
     */
    async::task<size_t> send(const fb::stream& stream, bool encrypt = true, bool wrap = true) override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  response  The response
     * @param[in]  encrypt   The encrypt
     * @param[in]  wrap      The wrap
     *
     * @return     { description_of_the_return_value }
     */
    async::task<size_t> send(const fb::protocol::header& response,
                             bool                        encrypt = true,
                             bool                        wrap    = true) override final;

    /**
     * @brief      { function_description }
     *
     * @return     The object type.
     */
    OBJECT_TYPE what() const override final;

    /**
     * @brief      { function_description }
     *
     * @param      map           The map
     * @param[in]  position      The position
     * @param[in]  destroy_type  The destroy type
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> map(fb::game::map*   map,
                                        const point16_t& position     = point16_t{0, 0},
                                        DESTROY_TYPE     destroy_type = DESTROY_TYPE::DEFAULT) override final;

public:
    operator fb::socket<fb::game::character>& ();

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool inited() const;

    /**
     * @brief      Initializes the given value.
     *
     * @param[in]  value  The value
     */
    void init(bool value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t id() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  id    The identifier
     */
    void id(uint32_t id);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t fd();

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool admin() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void admin(bool value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool transferring() const;

    /**
     * @brief      { function_description }
     */
    void attack() override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  size  The size
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t auto_attack_damage(MOB_SIZE size) const override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  action    The action
     * @param[in]  duration  The duration
     * @param[in]  sound     The sound
     */
    void action(ACTION action, DURATION duration, uint8_t sound = 0x00);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const std::string& name() const override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void name(const std::string& value);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void pw(const std::string& value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const datetime& updated_date() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void updated_date(const datetime& value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint16_t look() const override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void look(uint16_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t color() const override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void color(uint8_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    std::optional<uint8_t> armor_color() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void armor_color(std::optional<uint8_t> value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t current_armor_color() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    std::optional<uint16_t> disguise() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void disguise(uint16_t value);

    /**
     * @brief      { function_description }
     */
    void undisguise();

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t defensive_physical() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void defensive_physical(uint8_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t defensive_magical() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void defensive_magical(uint8_t value);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void base_hp_up(uint32_t value);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void base_mp_up(uint32_t value);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void base_hp(uint32_t value);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void base_mp(uint32_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t base_hp() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t base_mp() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    NATION nation() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    bool nation(NATION value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    CREATURE creature() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    bool creature(CREATURE value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t level() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void level(uint8_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool level_up();

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool max_level() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    SEX sex() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void sex(SEX value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    STATE state() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void state(STATE value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    CLASS cls() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void cls(CLASS value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t promotion() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void promotion(uint8_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t strength() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void strength(uint8_t value);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void strength_up(uint8_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t intelligence() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void intelligence(uint8_t value);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void intelligence_up(uint8_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t dexteritry() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void dexteritry(uint8_t value);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void dexteritry_up(uint8_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t exp() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void exp(uint32_t value);

    /**
     * @brief      Adds an exponent.
     *
     * @param[in]  value   The value
     * @param[in]  notify  The notify
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t add_exp(uint32_t value, bool limit = false, bool notify = false);

    /**
     * @brief      Reduces the exponent.
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t reduce_exp(uint32_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t experience_remained() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    float experience_percent() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t money() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void money(uint32_t value);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t money_add(uint32_t value);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t money_reduce(uint32_t value);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t money_drop(uint32_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t deposited_money() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void deposited_money(uint32_t value);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t deposit_money(uint32_t value);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t withdraw_money(uint32_t value);

    /**
     * @brief      { function_description }
     *
     * @param      item  The item
     *
     * @return     { description_of_the_return_value }
     */
    bool deposit_item(fb::game::item& item);

    /**
     * @brief      { function_description }
     *
     * @param[in]  index  The index
     * @param[in]  count  The count
     *
     * @return     { description_of_the_return_value }
     */
    bool deposit_item(uint8_t index, uint16_t count);

    /**
     * @brief      { function_description }
     *
     * @param[in]  name   The name
     * @param[in]  count  The count
     *
     * @return     { description_of_the_return_value }
     */
    bool deposit_item(const std::string& name, uint16_t count);

    /**
     * @brief      { function_description }
     *
     * @param[in]  item  The item
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::item* deposited_item(const fb::model::item& item) const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const std::vector<item*>& deposited_items() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  index  The index
     * @param[in]  count  The count
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::item* withdraw_item(uint8_t index, uint16_t count);

    /**
     * @brief      { function_description }
     *
     * @param[in]  name   The name
     * @param[in]  count  The count
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::item* withdraw_item(const std::string& name, uint16_t count);

    /**
     * @brief      { function_description }
     *
     * @param[in]  item   The item
     * @param[in]  count  The count
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::item* withdraw_item(const fb::model::item& item, uint16_t count);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t damage() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void damage(uint8_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t hit() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void hit(uint8_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t regenerative() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void regenerative(uint8_t value);

    /**
     * @brief      { function_description }
     *
     * @param[in]  key   The key
     *
     * @return     { description_of_the_return_value }
     */
    bool option(SETTING key) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  key     The key
     * @param[in]  value   The value
     * @param[in]  notify  The notify
     */
    void option(SETTING key, bool value, bool notify = true);

    /**
     * @brief      { function_description }
     *
     * @param[in]  key     The key
     * @param[in]  notify  The notify
     *
     * @return     { description_of_the_return_value }
     */
    bool option_toggle(SETTING key, bool notify = true);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const std::string& title() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void title(const std::string& value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const shared_group_lock& group() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    shared_group_lock& group();

    /**
     * @brief      { function_description }
     *
     * @param      value  The value
     */
    void group(shared_group_lock& value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    shared_clan_lock& clan();

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const shared_clan_lock& clan() const;

    /**
     * @brief      { function_description }
     *
     * @param      value  The value
     */
    void clan(shared_clan_lock& value);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void assert_state(STATE value) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  values  The values
     */
    void assert_state(const std::vector<STATE>& values) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  before  The before
     *
     * @return     { description_of_the_return_value }
     */
    bool move(const point16_t& before);

    /**
     * @brief      { function_description }
     *
     * @param[in]  direction  The direction
     * @param[in]  before     The before
     *
     * @return     { description_of_the_return_value }
     */
    bool move(DIRECTION direction, const point16_t& before);

    /**
     * @brief      { function_description }
     *
     * @param      horse  The horse
     */
    void ride(fb::game::mob& horse);

    /**
     * @brief      { function_description }
     */
    void ride();

    /**
     * @brief      { function_description }
     */
    void unride();

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool alive() const;

    /**
     * @brief      { function_description }
     */
    void refresh_map();

    /**
     * @brief      { function_description }
     *
     * @param[in]  conditions  The conditions
     *
     * @return     { description_of_the_return_value }
     */
    bool condition(const std::vector<fb::model::dsl>& conditions) const override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  message  The message
     * @param[in]  type     The type
     */
    void message(const std::string& message, MESSAGE_TYPE type = MESSAGE_TYPE::STATE);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::thread* thread() const override final;

    /**
     * @brief      { function_description }
     */
    void assert_thread() const override final;

    /**
     * @brief      Updates the given value.
     *
     * @param[in]  value  The value
     */
    void update(STATE_LEVEL value = STATE_LEVEL::LEVEL_MIN);

private:
    /**
     * @brief      { function_description }
     *
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool inline_sell(const std::string& message, const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool inline_buy(const std::string& message, const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool inline_repair(const std::string& message, const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool inline_deposit_money(const std::string& message, const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool inline_withdraw_money(const std::string& message, const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool inline_deposit_item(const std::string& message, const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool inline_withdraw_item(const std::string& message, const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool inline_sell_list(const std::string& message, const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool inline_buy_list(const std::string& message, const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool inline_sell_price(const std::string& message, const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool inline_buy_price(const std::string& message, const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool inline_show_deposited_money(const std::string& message, const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool inline_rename_weapon(const std::string& message, const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool inline_hold_item_list(const std::string& message, const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool inline_hold_item_count(const std::string& message, const std::vector<fb::game::npc*>& npcs);

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool inline_interaction(const std::string& message, const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      Returns a protocol representation of the object.
     *
     * @return     Protocol representation of the object.
     */
    fb::protocol::internal::Character to_protocol() const;

public:
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_look(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_color(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_money(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_exp(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_base_hp(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_base_mp(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_strength(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_dexterity(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_intelligence(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_item(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_items(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_item_drop(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_mkitem(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_rmitem(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_state(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_disguise(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_class(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_level(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_assert(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_admin(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_deposited_money(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_deposited_item(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_deposit_item(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_withdraw_item(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_group(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_create_group(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_clan(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_create_clan(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_destroy_clan(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_traces(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_trace(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_push_trace(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_erase_trace(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_switch_context(lua_State* lua);
};

/**
 * @brief      This class describes a container.
 */
class character::container : private std::vector<fb::game::character*>
{
public:
    using std::vector<fb::game::character*>::begin;
    using std::vector<fb::game::character*>::end;
    using std::vector<fb::game::character*>::size;
    using std::vector<fb::game::character*>::operator[];

public:
    /**
     * @brief      Constructs a new instance.
     */
    container();

    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The right
     */
    container(const std::vector<fb::game::character*>& right);

    /**
     * @brief      Destroys the object.
     */
    ~container();

public:
    /**
     * @brief      { function_description }
     *
     * @param      ch    The ch
     *
     * @return     { description_of_the_return_value }
     */
    container& push(fb::game::character& ch);

    /**
     * @brief      { function_description }
     *
     * @param      ch    The ch
     *
     * @return     { description_of_the_return_value }
     */
    container& erase(fb::game::character& ch);

public:
    /**
     * @brief      Searches for the first match.
     *
     * @param[in]  name  The name
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::character* find(const std::string& name);

    /**
     * @brief      { function_description }
     *
     * @param[in]  ch    The ch
     *
     * @return     { description_of_the_return_value }
     */
    bool contains(const fb::game::character& ch) const;

public:
    /**
     * @brief      Array indexer operator.
     *
     * @param[in]  name  The name
     *
     * @return     The result of the array indexer
     */
    fb::game::character* operator[] (const std::string& name);
};

/**
 * @brief      { struct_description }
 */
struct character::listener : public virtual fb::game::life::listener,
                             public virtual fb::game::dialog::listener,
                             public virtual fb::game::trade::listener,
                             public virtual fb::game::equipment::listener
{
public:
    /**
     * @brief      Called on notify.
     *
     * @param      me       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  type     The type
     */
    virtual void on_message(character& me, const std::string& message, MESSAGE_TYPE type = MESSAGE_TYPE::STATE) = 0;

    /**
     * @brief      Called on option.
     *
     * @param      me       { parameter_description }
     * @param[in]  option   The option
     * @param[in]  enabled  Indicates if enabled
     */
    virtual void on_option(character& me, SETTING option, bool enabled) = 0;

    /**
     * @brief      Called on level up.
     *
     * @param      me    { parameter_description }
     */
    virtual void on_level_up(character& me) = 0;

    /**
     * @brief      Called on action.
     *
     * @param      me        { parameter_description }
     * @param[in]  action    The action
     * @param[in]  duration  The duration
     * @param[in]  sound     The sound
     */
    virtual void on_action(character& me, ACTION action, DURATION duration, uint8_t sound) = 0;

    /**
     * @brief      Called when updated.
     *
     * @param      me     { parameter_description }
     * @param[in]  level  The level
     */
    virtual void on_update(character& me, STATE_LEVEL level = STATE_LEVEL::LEVEL_MIN) = 0;

    /**
     * @brief      Called on transfer.
     *
     * @param      me        { parameter_description }
     * @param      map       The map
     * @param[in]  position  The position
     *
     * @return     { description_of_the_return_value }
     */
    virtual async::task<bool> on_transfer(character& me, fb::game::map& map, const point16_t& position) = 0;
};

} // namespace fb::game

#endif // !__CHARACTER_H__
