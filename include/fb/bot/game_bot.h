#ifndef __BOT_GAME_H__
#define __BOT_GAME_H__

#include <fb/bot/bot.h>
#include <fb/game/protocol.h>
#include <shared_mutex>
#include <set>
#include <string>
#include <map>
#include <optional>
#include <fb/bot/integration/dialog_bot.h>
#include <fb/bot/integration/dialog_ext_bot.h>
#include <string_view>

namespace fb::bot {

using namespace fb::model::enum_value;

class game_bot_controller;
template <typename ControllerType> class bot;

struct spawned_monster_info
{
    uint32_t                   oid;
    fb::model::point<uint16_t> position;
    uint32_t                   look;
};

class game_bot : public bot<game_bot>
{
public:
    using bot_controller_type = game_bot_controller;

    struct simple_item
    {
        std::string name;
        uint32_t    count = 0;

        simple_item() = default;
        simple_item(std::string_view n, uint32_t c) :
            name(std::string(n)),
            count(c)
        { }

        bool        is_equipment(const game_bot_controller& controller) const;
        std::string get_equipment_info(const game_bot_controller& controller) const;
        uint32_t    get_price(const game_bot_controller& controller) const;
    };

    struct simple_spell
    {
        std::string name;
        uint8_t     type;

        simple_spell() = default;
        simple_spell(std::string_view n, uint8_t t) :
            name(std::string(n)),
            type(t)
        { }
    };

    struct simple_npc
    {
        const uint32_t    oid;
        const std::string name;

        simple_npc(uint32_t o, std::string_view n) :
            oid(o),
            name(std::string(n))
        { }
    };

private:
    typedef struct _pattern_params_tag
    {
        std::function<async::task<void>()>  fn;
        std::chrono::steady_clock::duration min;
        std::chrono::steady_clock::duration max;
    } pattern_params;

private:
    bool                            _inited = false;
    std::vector<pattern_params>     _pattern_params;
    datetime                        _next_action_time;
    uint16_t                        _map = 0xFFFF;
    uint32_t                        _oid = 0;
    point<uint16_t>                 _position;
    fb::stream                      _transfer_buffer;
    DIRECTION                       _direction = DIRECTION::BOTTOM;
    uint16_t                        _look      = 0;
    uint8_t                         _color     = 0;
    bool                            _dead      = false;
    std::set<std::string>           _active_buffs;
    std::map<uint8_t, simple_item>  _items;
    std::map<uint8_t, simple_spell> _spells;
    uint8_t                         _nation        = 0;
    uint8_t                         _creature      = 0;
    uint8_t                         _level         = 0;
    uint32_t                        _base_hp       = 0;
    uint32_t                        _base_mp       = 0;
    uint8_t                         _strength      = 0;
    uint8_t                         _intelligence  = 0;
    uint8_t                         _dexterity     = 0;
    uint32_t                        _hp            = 0;
    uint32_t                        _mp            = 0;
    uint32_t                        _exp           = 0;
    uint32_t                        _money         = 0;
    uint32_t                        _crowd_control = 0;
    uint8_t                         _mail_count    = 0;
    uint8_t                         _fast_move     = 0;
    uint8_t                         _disguised     = 0;
    GENDER                          _gender        = GENDER::MAN;
    STATE                           _state         = STATE::NORMAL;
    uint8_t                         _armor_dress   = 0;
    uint8_t                         _armor_color   = 0;
    uint16_t                        _weapon_dress  = 0;
    uint8_t                         _weapon_color  = 0;
    uint8_t                         _shield_dress  = 0;
    uint8_t                         _shield_color  = 0;
    uint8_t                         _head_marker   = 0;
    std::string                     _name;
    std::string                     _clan_name;
    std::string                     _clan_title;
    std::string                     _title;
    std::string                     _group_info;
    uint8_t                         _group_option = 0;
    uint32_t                        _remained_exp = 0;

public:
    game_bot(bot_controller<game_bot>& bot_controller, uint32_t id);
    game_bot(bot_controller<game_bot>& bot_controller, uint32_t id, const fb::stream& params);
    ~game_bot();

private:
    template <typename Class> void pattern(async::task<void> (Class::*fn)(),
                                           const std::chrono::steady_clock::duration& min,
                                           const std::chrono::steady_clock::duration& max)
    {
        this->_pattern_params.push_back(pattern_params{std::bind(fn, static_cast<Class*>(this)), min, max});
    }

public:
    async::task<void> process_random_pattern(const fb::model::datetime& now);

public:
    uint32_t oid() const;
    void     set_oid(uint32_t value);

    uint16_t map() const;
    void     set_map(uint16_t value);

    point<uint16_t> position() const;
    void            set_position(const point<uint16_t>& value);

    bool inited() const;
    void inited(bool value);

    const fb::stream& transfer_buffer() const;

    DIRECTION direction() const;
    void      set_direction(DIRECTION value);

    uint16_t look() const;
    void     set_look(uint16_t value);

    uint8_t color() const;
    void    set_color(uint8_t value);

    bool is_dead() const;
    void set_dead(bool value);

    const std::set<std::string>& active_buffs() const;
    void                         add_buff(std::string_view name);
    void                         remove_buff(std::string_view name);
    bool                         has_buff(std::string_view name) const;

    void                                   update_spell(uint8_t slot, std::string_view name, uint8_t type);
    void                                   remove_spell(uint8_t slot);
    bool                                   has_spell(uint8_t slot) const;
    const std::map<uint8_t, simple_spell>& spells() const;

    uint8_t            nation() const;
    void               set_nation(uint8_t value);
    uint8_t            creature() const;
    void               set_creature(uint8_t value);
    uint8_t            level() const;
    void               set_level(uint8_t value);
    uint32_t           base_hp() const;
    void               set_base_hp(uint32_t value);
    uint32_t           base_mp() const;
    void               set_base_mp(uint32_t value);
    uint8_t            strength() const;
    void               set_strength(uint8_t value);
    uint8_t            intelligence() const;
    void               set_intelligence(uint8_t value);
    uint8_t            dexterity() const;
    void               set_dexterity(uint8_t value);
    uint32_t           hp() const;
    void               set_hp(uint32_t value);
    uint32_t           mp() const;
    void               set_mp(uint32_t value);
    uint32_t           exp() const;
    void               set_exp(uint32_t value);
    uint32_t           money() const;
    void               set_money(uint32_t value);
    uint32_t           crowd_control() const;
    void               set_crowd_control(uint32_t value);
    uint8_t            mail_count() const;
    void               set_mail_count(uint8_t value);
    uint8_t            fast_move() const;
    void               set_fast_move(uint8_t value);
    uint8_t            disguised() const;
    void               set_disguised(uint8_t value);
    GENDER             gender() const;
    void               set_gender(GENDER value);
    STATE              state() const;
    void               set_state(STATE value);
    uint8_t            armor_dress() const;
    void               set_armor_dress(uint8_t value);
    uint8_t            armor_color() const;
    void               set_armor_color(uint8_t value);
    uint16_t           weapon_dress() const;
    void               set_weapon_dress(uint16_t value);
    uint8_t            weapon_color() const;
    void               set_weapon_color(uint8_t value);
    uint8_t            shield_dress() const;
    void               set_shield_dress(uint8_t value);
    uint8_t            shield_color() const;
    void               set_shield_color(uint8_t value);
    uint8_t            head_marker() const;
    void               set_head_marker(uint8_t value);
    const std::string& name() const;
    void               set_name(std::string_view value);
    const std::string& clan_name() const;
    void               set_clan_name(std::string_view value);
    const std::string& clan_title() const;
    void               set_clan_title(std::string_view value);
    const std::string& title() const;
    void               set_title(std::string_view value);
    const std::string& group_info() const;
    void               set_group_info(std::string_view value);
    uint8_t            group_option() const;
    void               set_group_option(uint8_t value);
    uint32_t           remained_exp() const;
    void               set_remained_exp(uint32_t value);
    async::task<void>  move(DIRECTION direction, int step = 1, const fb::model::timespan& delay = 500ms);
    async::task<void>  map_move(std::string_view map_name, uint16_t x, uint16_t y, std::chrono::milliseconds timeout);
    async::task<void>  change_level(uint8_t level, std::chrono::milliseconds timeout);
    async::task<void>  change_stats(uint8_t str, uint8_t dex, uint8_t intelligence, std::chrono::milliseconds timeout);
    async::task<void>  change_str(uint8_t str, std::chrono::milliseconds timeout);
    async::task<void>  change_dex(uint8_t dex, std::chrono::milliseconds timeout);
    async::task<void>  change_int(uint8_t intelligence, std::chrono::milliseconds timeout);

    async::task<void> change_gender(GENDER gender, std::chrono::milliseconds timeout);
    async::task<void> change_base_hp(uint32_t hp, std::chrono::milliseconds timeout);
    async::task<void> change_base_mp(uint32_t mp, std::chrono::milliseconds timeout);
    async::task<void> change_hp(uint32_t hp, std::chrono::milliseconds timeout);
    async::task<void> change_mp(uint32_t mp, std::chrono::milliseconds timeout);
    async::task<void> direction(DIRECTION direction, std::chrono::milliseconds timeout);
    async::task<void> pattern_chat();
    async::task<void> pattern_attack();
    async::task<void> pattern_direction();
    async::task<void> pattern_move();
    async::task<void> pattern_loot();
    async::task<void> pattern_emotion();
    async::task<void> pattern_bulletin_sections();

public:
    void                                  update_item(uint8_t slot, std::string_view name, uint32_t count);
    void                                  remove_item(uint8_t slot);
    std::optional<simple_item>            get_item(uint8_t slot) const;
    std::optional<simple_spell>           get_spell(uint8_t slot) const;
    bool                                  has_item(uint8_t slot) const;
    const std::map<uint8_t, simple_item>& items() const;
    bool                                  has_item_by_name(std::string_view name) const;
    uint16_t                              get_item_count_by_name(std::string_view name) const;
    uint8_t                               get_item_slot_by_name(std::string_view name) const;
    uint8_t                               get_spell_slot_by_name(std::string_view name) const;
    void                                  remove_buffs();
    void                                  chat(std::string_view message);
    async::task<void> create_item(std::string_view item_name, uint32_t count, std::chrono::milliseconds timeout);
    async::task<simple_npc> create_npc(std::string_view npc_name, std::chrono::milliseconds timeout);
    async::task<void>       change_money(uint32_t amount, std::chrono::milliseconds timeout);
    async::task<void>       drop_item(uint8_t index, bool all, std::chrono::milliseconds timeout);
    async::task<void>       drop_money(uint32_t amount, std::chrono::milliseconds timeout);
    async::task<bool>       equip(uint8_t slot, std::chrono::milliseconds timeout);
    async::task<bool>       unequip(EQUIPMENT_PARTS parts, std::chrono::milliseconds timeout);
    async::task<void>       sleep(std::chrono::milliseconds timeout);
    async::task<void>       change_class(std::string_view class_name, std::chrono::milliseconds timeout);
    async::task<spawned_monster_info>
    spawn_monster_with_validator(std::shared_ptr<fb::bot::game_bot>                               bot,
                                 std::string_view                                                  monster_name,
                                 uint16_t                                                         x,
                                 uint16_t                                                         y,
                                 std::function<bool(const fb::protocol::game::response::update&)> validator,
                                 std::chrono::milliseconds                                        timeout);

    async::task<spawned_monster_info>
    spawn_monster(std::string_view monster_name, uint16_t x, uint16_t y, std::chrono::milliseconds timeout);

    async::task<void> spawn_monsters_bulk(std::string_view        monster_name,
                                          uint8_t                   range,
                                          std::chrono::milliseconds timeout);

    async::task<std::vector<spawned_monster_info>>
    spawn_monsters_relative_with_validator(std::shared_ptr<fb::bot::game_bot>      bot,
                                           std::string_view                        monster_name,
                                           const std::vector<std::pair<int, int>>& relative_positions,
                                           std::function<bool(const fb::protocol::game::response::update&)> validator,
                                           std::chrono::milliseconds                                        timeout);

    async::task<std::vector<spawned_monster_info>>
    spawn_monsters_relative(std::string_view                        monster_name,
                            const std::vector<std::pair<int, int>>& relative_positions,
                            std::chrono::milliseconds               timeout);

    async::task<spawned_monster_info> spawn_monster_relative(std::string_view        monster_name,
                                                             int                       relative_x,
                                                             int                       relative_y,
                                                             std::chrono::milliseconds timeout);

    async::task<bool> set_max_hp_mp(int max_hp, int max_mp, std::chrono::milliseconds timeout);
    async::task<bool> set_current_hp_mp(int current_hp, int current_mp, std::chrono::milliseconds timeout);
    async::task<bool> setup_bot_stats(int                       max_hp,
                                      int                       max_mp,
                                      std::optional<int>        current_hp,
                                      std::optional<int>        current_mp,
                                      std::chrono::milliseconds timeout);

    async::task<uint8_t> learn_spell(std::string_view spell_name, std::chrono::milliseconds timeout);
    async::task<size_t>  learn_spells(const std::vector<std::string>& spell_names, std::chrono::milliseconds timeout);
    async::task<void>    clear_all_spells(std::chrono::milliseconds timeout);
    async::task<void>    clear_all_drop_items(std::chrono::milliseconds timeout);
    async::task<void>    clear_inventory(std::chrono::milliseconds timeout);
    async::task<void>    fill_inventory(std::string_view name, std::chrono::milliseconds timeout);
    async::task<void>    move_bot_back_to_position(const fb::model::point<uint16_t>& original_position,
                                                   std::chrono::milliseconds         interval,
                                                   std::chrono::milliseconds         timeout);

    async::task<void> reverse_condition(const std::vector<fb::model::dsl>& conditions,
                                        std::chrono::milliseconds          timeout);

    async::task<void> apply_condition(const std::vector<fb::model::dsl>& conditions, std::chrono::milliseconds timeout);
    async::task<void> update_internal_info(std::chrono::milliseconds timeout);
    async::task<bool> invite_group(std::shared_ptr<game_bot> target, std::chrono::milliseconds timeout);
    async::task<bool> leave_group(std::chrono::milliseconds timeout);
    async::task<bool> kick_group(std::shared_ptr<game_bot> target, std::chrono::milliseconds timeout);

    async::task<std::shared_ptr<game_bot>> transfer(const fb::protocol::header& protocol,
                                                    const fb::model::timespan&  timeout = 15s,
                                                    bool                        encrypt = true,
                                                    bool                        wrap    = true);
};

} // namespace fb::bot

#endif