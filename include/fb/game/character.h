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

using group_lock        = fb::locker<group>;
using shared_group_lock = std::shared_ptr<group_lock>;

using clan_lock        = fb::locker<clan>;
using shared_clan_lock = std::shared_ptr<clan_lock>;

/**
 * @brief      This class describes a character.
 */
class character : public life
{
    friend class group;
    friend class clan;

public:
    using object::map;

public:
    /**
     * @brief      This class describes a container.
     */
    class container;

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
    uint8_t                 _dexterity          = 0;
    uint8_t                 _damage             = 0; // 공격수정
    uint8_t                 _hit                = 0; // 명중수정
    uint8_t                 _regenerative       = 0; // 재생력
    uint8_t                 _buff_str           = 0;
    uint8_t                 _buff_int           = 0;
    uint8_t                 _buff_dex           = 0;
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
        1,
    };

public:
    fb::game::trade                            trade  = fb::game::trade(*this);
    fb::game::items                            items  = fb::game::items(*this);
    fb::game::dialog                           dialog = fb::game::dialog(*this);
    std::map<uint32_t, std::unique_ptr<trace>> traces; // order required

private:
    using object::based;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param      socket   The socket
     */
    character(fb::game::context& context, fb::socket<character>& socket);

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
    [[nodiscard]] async::task<bool> map(fb::game::map*              map,
                                        const fb::model::point16_t& position,
                                        DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) override final;

public:
    operator fb::socket<character>& ();

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
     * @param[in]  value     The value
     * @param      from      The from
     * @param[in]  critical  The critical
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t damage(uint32_t value, fb::game::object* from = nullptr, bool critical = false) override final;

    /**
     * @brief      { function_description }
     *
     * @param[in]  action    The action
     * @param[in]  duration  The duration
     * @param[in]  sound     The sound
     */
    void action(ACTION action, DURATION duration, uint8_t sound = 0x00) override final;

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
    const fb::model::datetime& updated_date() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void updated_date(const fb::model::datetime& value);

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
    uint8_t dexterity() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void dexterity(uint8_t value);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void dexterity_up(uint8_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t buff_str() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void buff_str(uint8_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t buff_dex() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void buff_dex(uint8_t value);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t buff_int() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void buff_int(uint8_t value);

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
     * @param[in]  limit   The limit
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
    bool deposit_item(item& item);

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
    item* deposited_item(const fb::model::item& item) const;

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
    item* withdraw_item(uint8_t index, uint16_t count);

    /**
     * @brief      { function_description }
     *
     * @param[in]  name   The name
     * @param[in]  count  The count
     *
     * @return     { description_of_the_return_value }
     */
    item* withdraw_item(const std::string& name, uint16_t count);

    /**
     * @brief      { function_description }
     *
     * @param[in]  item   The item
     * @param[in]  count  The count
     *
     * @return     { description_of_the_return_value }
     */
    item* withdraw_item(const fb::model::item& item, uint16_t count);

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
    bool option(OPTION key) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  key     The key
     * @param[in]  value   The value
     * @param[in]  notify  The notify
     */
    void option(OPTION key, bool value, bool notify = true);

    /**
     * @brief      { function_description }
     *
     * @param[in]  key     The key
     * @param[in]  notify  The notify
     *
     * @return     { description_of_the_return_value }
     */
    bool option_toggle(OPTION key, bool notify = true);

    /**
     * @brief      { function_description }
     */
    void update_option();

    /**
     * @brief      { function_description }
     *
     * @param[in]  map   The map
     */
    void update_map(const fb::game::map& map) override final;

    /**
     * @brief      { function_description }
     */
    void update_map();

    /**
     * @brief      { function_description }
     *
     * @param[in]  map    The map
     * @param[in]  begin  The begin
     * @param[in]  size   The size
     */
    void update_map(const fb::game::map& map, const fb::model::point16_t& begin, const fb::model::size8_t& size);

    /**
     * @brief      { function_description }
     *
     * @param[in]  bgm     The bgm
     * @param[in]  volume  The volume
     */
    void update_bgm(uint16_t bgm, uint8_t volume) override final;

    /**
     * @brief      { function_description }
     */
    void update_buff();

    /**
     * @brief      { function_description }
     */
    void update_internal();

    /**
     * @brief      { function_description }
     *
     * @param[in]  hours  The hours
     */
    void update_time(uint16_t hours);

    /**
     * @brief      Initializes the object.
     */
    void init();

    /**
     * @brief      { function_description }
     */
    void update_position() override final;

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
    bool move(const fb::model::point16_t& before);

    /**
     * @brief      { function_description }
     *
     * @param[in]  direction  The direction
     * @param[in]  before     The before
     *
     * @return     { description_of_the_return_value }
     */
    bool move(DIRECTION direction, const fb::model::point16_t& before);

    /**
     * @brief      { function_description }
     *
     * @param      horse  The horse
     */
    void ride(mob& horse);

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
    void update(STATE_LEVEL value = STATE_LEVEL::LEVEL_MIN) override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint16_t unread_mail() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void unread_mail(uint16_t value);

    /**
     * @brief      { function_description }
     *
     * @param[in]  ch    { parameter_description }
     */
    void browse_ch(const character& ch);

    /**
     * @brief      { function_description }
     *
     * @param[in]  iteem     The iteem
     * @param[in]  position  The position
     */
    void item_tooltip(const item& iteem, uint16_t position);

    /**
     * @brief      Shows the user list.
     */
    void show_user_list();

    /**
     * @brief      Shows the board.
     */
    void show_board();

    /**
     * @brief      Shows the board.
     *
     * @param[in]  section   The section
     * @param[in]  articles  The articles
     * @param[in]  flag      The flag
     */
    void show_board(const fb::model::board&          section,
                    const std::list<board::article>& articles,
                    BOARD_BUTTON_ENABLE              flag);

    /**
     * @brief      Shows the board.
     *
     * @param[in]  article  The article
     * @param[in]  flag     The flag
     */
    void show_board(const board::article& article, BOARD_BUTTON_ENABLE flag);

    /**
     * @brief      Shows the mail box.
     *
     * @param[in]  mails  The mails
     * @param[in]  flag   The flag
     */
    void show_mail_box(const std::vector<fb::protocol::internal::MailSummary>& mails, MAIL_BUTTON_ENABLE flag);

    /**
     * @brief      Shows the mail box.
     *
     * @param[in]  mail  The mail
     * @param[in]  flag  The flag
     */
    void show_mail_box(const fb::protocol::internal::Mail& mail, MAIL_BUTTON_ENABLE flag);

    /**
     * @brief      Shows the board message.
     *
     * @param[in]  message  The message
     * @param[in]  success  The success
     * @param[in]  mail     The mail
     */
    void show_board_message(const std::string& message, bool success, bool mail);

    /**
     * @brief      Shows the world map.
     *
     * @param[in]  id     The identifier
     * @param[in]  index  The index
     */
    void show_world_map(uint32_t id, uint16_t index);

    /**
     * @brief      { function_description }
     *
     * @param[in]  time  The time
     * @param[in]  type  The type
     */
    void timer(uint32_t time, TIMER_TYPE type);

    /**
     * @brief      { function_description }
     *
     * @param[in]  weather  The weather
     */
    void weather(WEATHER_TYPE weather);

    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void bright(uint8_t value);

    /**
     * @brief      { function_description }
     */
    void update_id() override final;

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
    static int builtin_str(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_dex(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_int(lua_State* lua);

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

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_whisper(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_send_mail(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_assert_state(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_message(lua_State* lua);

    static int builtin_buff_str(lua_State* lua);
    static int builtin_buff_dex(lua_State* lua);
    static int builtin_buff_int(lua_State* lua);
};

/**
 * @brief      This class describes a container.
 */
class character::container : private std::vector<character*>
{
public:
    using std::vector<character*>::begin;
    using std::vector<character*>::end;
    using std::vector<character*>::size;
    using std::vector<character*>::operator[];

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
    container(const std::vector<character*>& right);

    /**
     * @brief      Destroys the object.
     */
    ~container();

public:
    /**
     * @brief      { function_description }
     *
     * @param      ch    { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    container& push(character& ch);

    /**
     * @brief      { function_description }
     *
     * @param      ch    { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    container& erase(character& ch);

public:
    /**
     * @brief      Searches for the first match.
     *
     * @param[in]  name  The name
     *
     * @return     { description_of_the_return_value }
     */
    character* find(const std::string& name);

    /**
     * @brief      { function_description }
     *
     * @param[in]  ch    { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool contains(const character& ch) const;

public:
    /**
     * @brief      Array indexer operator.
     *
     * @param[in]  name  The name
     *
     * @return     The result of the array indexer
     */
    character* operator[] (const std::string& name);
};

/**
 * @brief      { struct_description }
 */
struct character::listener : public virtual life::listener,
                             public virtual dialog::listener,
                             public virtual trade::listener,
                             public virtual equipment::listener
{
public:
    /**
     * @brief      Called on message.
     *
     * @param      me       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  type     The type
     */
    virtual void on_message(character& me, const std::string& message, MESSAGE_TYPE type = MESSAGE_TYPE::STATE) = 0;

    /**
     * @brief      Called when option changed.
     *
     * @param      me       { parameter_description }
     * @param[in]  option   The option
     * @param[in]  enabled  Indicates if enabled
     */
    virtual void on_option_changed(character& me, OPTION option, bool enabled) = 0;

    /**
     * @brief      Called on update option.
     *
     * @param      me    { parameter_description }
     */
    virtual void on_update_option(character& me) = 0;

    /**
     * @brief      Called on update map.
     *
     * @param      ch    { parameter_description }
     * @param[in]  map   The map
     */
    virtual void on_update_map(character& ch, const fb::game::map& map) = 0;

    /**
     * @brief      Called on update map.
     *
     * @param      ch     { parameter_description }
     * @param[in]  map    The map
     * @param[in]  begin  The begin
     * @param[in]  size   The size
     */
    virtual void on_update_map(character&                  ch,
                               const fb::game::map&        map,
                               const fb::model::point16_t& begin,
                               const fb::model::size8_t&   size) = 0;

    /**
     * @brief      Called on update bgm.
     *
     * @param      ch      { parameter_description }
     * @param[in]  bgm     The bgm
     * @param[in]  volume  The volume
     */
    virtual void on_update_bgm(character& ch, uint16_t bgm, uint8_t volume) = 0;

    /**
     * @brief      Called on update buffer.
     *
     * @param      ch     { parameter_description }
     * @param[in]  buffs  The buffs
     */
    virtual void on_update_buff(character& ch, const fb::game::buffs& buffs) = 0;

    /**
     * @brief      Called on update internal.
     *
     * @param      ch    { parameter_description }
     */
    virtual void on_update_internal(character& ch) = 0;

    /**
     * @brief      Called on update time.
     *
     * @param      ch     { parameter_description }
     * @param[in]  hours  The hours
     */
    virtual void on_update_time(character& ch, uint16_t hours) = 0;

    /**
     * @brief      Called on browse character.
     *
     * @param      ch      { parameter_description }
     * @param[in]  target  The target
     */
    virtual void on_browse_character(character& ch, const character& target) = 0;

    /**
     * @brief      Called on item tooltip.
     *
     * @param      ch        { parameter_description }
     * @param[in]  item      The item
     * @param[in]  position  The position
     */
    virtual void on_item_tooltip(character& ch, const item& item, uint16_t position) = 0;

    /**
     * @brief      Called on show user list.
     *
     * @param      ch    { parameter_description }
     */
    virtual void on_show_user_list(character& ch) = 0;

    /**
     * @brief      Called on show board.
     *
     * @param      ch    { parameter_description }
     */
    virtual void on_show_board(character& ch) = 0;

    /**
     * @brief      Called on show board.
     *
     * @param      ch        { parameter_description }
     * @param[in]  section   The section
     * @param[in]  articles  The articles
     * @param[in]  flag      The flag
     */
    virtual void on_show_board(character&                       ch,
                               const fb::model::board&          section,
                               const std::list<board::article>& articles,
                               BOARD_BUTTON_ENABLE              flag) = 0;

    /**
     * @brief      Called on show board.
     *
     * @param      ch     { parameter_description }
     * @param[in]  value  The value
     * @param[in]  flag   The flag
     */
    virtual void on_show_board(character& ch, const board::article& value, BOARD_BUTTON_ENABLE flag) = 0;

    /**
     * @brief      Called on show mail box.
     *
     * @param      ch     { parameter_description }
     * @param[in]  mails  The mails
     * @param[in]  flag   The flag
     */
    virtual void on_show_mail_box(character&                                              ch,
                                  const std::vector<fb::protocol::internal::MailSummary>& mails,
                                  MAIL_BUTTON_ENABLE                                      flag) = 0;

    /**
     * @brief      Called on show mail box.
     *
     * @param      ch    { parameter_description }
     * @param[in]  mail  The mail
     * @param[in]  flag  The flag
     */
    virtual void on_show_mail_box(character& ch, const fb::protocol::internal::Mail& mail, MAIL_BUTTON_ENABLE flag) = 0;

    /**
     * @brief      Called on show board message.
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  success  The success
     * @param[in]  mail     The mail
     */
    virtual void on_show_board_message(character& ch, const std::string& message, bool success, bool mail) = 0;

    /**
     * @brief      Called on show world map.
     *
     * @param      ch     { parameter_description }
     * @param[in]  id     The identifier
     * @param[in]  index  The index
     */
    virtual void on_show_world_map(character& ch, uint32_t id, uint16_t index) = 0;

    /**
     * @brief      Called on timer.
     *
     * @param      ch    { parameter_description }
     * @param[in]  time  The time
     * @param[in]  type  The type
     */
    virtual void on_timer(character& ch, uint32_t time, TIMER_TYPE type) = 0;

    /**
     * @brief      Called on weather.
     *
     * @param      ch       { parameter_description }
     * @param[in]  weather  The weather
     */
    virtual void on_weather(character& ch, WEATHER_TYPE weather) = 0;

    /**
     * @brief      Called on bright.
     *
     * @param      ch     { parameter_description }
     * @param[in]  value  The value
     */
    virtual void on_bright(character& ch, uint8_t value) = 0;

    /**
     * @brief      Called on update identifier.
     *
     * @param      ch    { parameter_description }
     */
    virtual void on_update_id(character& ch) = 0;

    /**
     * @brief      Called on character initialize.
     *
     * @param      ch    { parameter_description }
     */
    virtual void on_character_init(character& ch) = 0;

    /**
     * @brief      Called on update position.
     *
     * @param      ch    { parameter_description }
     */
    virtual void on_update_position(character& ch) = 0;

    /**
     * @brief      Called on level up.
     *
     * @param      me    { parameter_description }
     */
    virtual void on_level_up(character& me) = 0;

    /**
     * @brief      Called on update.
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
    virtual async::task<bool> on_transfer(character& me, fb::game::map& map, const fb::model::point16_t& position) = 0;
};

} // namespace fb::game

#endif // !__CHARACTER_H__
