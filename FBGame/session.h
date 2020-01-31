#ifndef __MMO_H__
#define __MMO_H__

#include <stdint.h>
#include <string>
#include "socket.h"
#include "acceptor.h"
#include <algorithm>
#include "item.h"
#include "object.h"
#include "spell.h"

namespace fb { namespace game {

class map;
class item;
class session;

class trade_system
{
private:
    session*                _owner;
    session*                _partner;
    std::vector<item*>      _items;
    item*                   _selected;
    uint32_t                _money;
    bool                    _locked;

public:
    trade_system(session& owner);
    ~trade_system();

private:
    uint8_t                 contains_core(item* item) const;

public:
    session*                partner() const;

    bool                    begin(session* partner);
    bool                    end();
    bool                    trading() const;

    item*                   selected();
    void                    select(item* item);

    uint8_t                 add(item* item);
    void                    money(uint32_t value);
    uint32_t                money() const;
    std::vector<uint8_t>    restore();
    void                    flush(session& session, std::vector<uint8_t>& indices);
    bool                    flushable(session& session) const;

    bool                    lock() const;
    void                    lock(bool value);

public:
    fb::ostream             make_show_stream(bool mine, uint8_t index) const;
    fb::ostream             make_money_stream(bool mine) const;
    fb::ostream             make_dialog_stream() const;
    fb::ostream             make_bundle_stream() const;
    fb::ostream             make_close_stream(const std::string& message) const;
    fb::ostream             make_lock_stream() const;
};


class group_system
{
private:
    session*                _leader;
    std::vector<session*>   _members;

public:
    group_system(session& leader);
    ~group_system();

public:
    bool                    add(session& session);
    bool                    remove(session& session);
};



class session : public fb_session, public life, public life::core
{
public:
    DECLARE_EXCEPTION(require_class_exception, "직업을 선택해야 합니다.")
    DECLARE_EXCEPTION(ghost_exception, "귀신은 할 수 없습니다.")
    DECLARE_EXCEPTION(ridding_exception, "말을 타고는 할 수 없습니다.")
    DECLARE_EXCEPTION(no_horse_exception, "탈 것이 없습니다.")
    DECLARE_EXCEPTION(disguise_exception, "변신 중에는 할 수 없습니다.")

private:
    uint8_t                 _strength, _intelligence, _dexteritry;
    uint8_t                 _damage; // 공격수정
    uint8_t                 _hit; // 명중수정
    uint8_t                 _regenerative; // 재생력

    fb::game::nation        _nation;
    fb::game::creature      _creature;
    fb::game::sex           _sex;
    fb::game::state         _state;
    uint8_t                 _level;
    uint8_t                 _class, _promotion;
    uint32_t                _money;

    bool                    _options[0x0B+1];
    std::vector<legend>     _legends;

    std::string             _title;

    item*                   _items[item::MAX_SLOT];
    spell*                  _spells[spell::MAX_SLOT];
    weapon*                 _weapon;
    armor*                  _armor;
    helmet*                 _helmet;
    shield*                 _shield;
    ring*                   _rings[2];
    auxiliary*              _auxiliaries[2];

    trade_system            _trade_system;
    group_system*           _group;

public:
    session(SOCKET socket);
    ~session();

public:
    const std::string&      name() const;
    void                    name(const std::string& value);

    uint16_t                look() const;
    void                    look(uint16_t value);

    uint16_t                color() const;
    void                    color(uint16_t value);

    uint32_t                defensive_physical() const;
    void                    defensive_physical(uint8_t value);

    uint32_t                defensive_magical() const;
    void                    defensive_magical(uint8_t value);

    void                    base_hp_up(uint32_t value);
    void                    base_mp_up(uint32_t value);

    void                    base_hp(uint32_t value);
    void                    base_mp(uint32_t value);

    uint32_t                base_hp() const;
    uint32_t                base_mp() const;

    void                    hp(uint32_t value);
    void                    mp(uint32_t value);

    uint32_t                hp() const;
    uint32_t                mp() const;

    uint32_t                id() const;
    fb::game::nation        nation() const;
    bool                    nation(fb::game::nation value);

    fb::game::creature      creature() const;
    bool                    creature(fb::game::creature value);

    uint8_t                 level() const;
    void                    level(uint8_t value);
    bool                    level_up();
    bool                    max_level() const;

    fb::game::sex           sex() const;
    void                    sex(fb::game::sex value);

    fb::game::state         state() const;
    void                    state(fb::game::state value);

    uint8_t                 cls() const;
    void                    cls(uint8_t value);

    uint8_t                 promotion() const;
    void                    promotion(uint8_t value);

    uint8_t                 strength() const;
    void                    strength(uint8_t value);
    void                    strength_up(uint8_t value);

    uint8_t                 intelligence() const;
    void                    intelligence(uint8_t value);
    void                    intelligence_up(uint8_t value);

    uint8_t                 dexteritry() const;
    void                    dexteritry(uint8_t value);
    void                    dexteritry_up(uint8_t value);

    uint32_t                experience() const;
    uint32_t                experience_add(uint32_t value);
    uint32_t                experience_reduce(uint32_t value);
    uint32_t                experience_remained() const;

    uint32_t                money() const;
    void                    money(uint32_t value);
    uint32_t                money_add(uint32_t value);
    uint32_t                money_reduce(uint32_t value);

    uint32_t                random_damage() const;
    void                    random_damage(uint8_t value);

    uint32_t                random_damage(fb::game::life& life, bool& critical) const;

    uint32_t                hit() const;
    void                    hit(uint8_t value);

    uint32_t                regenerative() const;
    void                    regenerative(uint8_t value);

    bool                    option(options key) const;
    void                    option(options key, bool value);
    bool                    option_toggle(options key);

    const std::vector<legend>&  legends() const;
    void                    legends_add(uint8_t look, uint8_t color, const std::string& content);

    const std::string&      title() const;
    void                    title(const std::string& value);

    fb::game::item*         item(uint8_t index) const;
    bool                    item_add(fb::game::item& item);
    uint8_t                 item_add(fb::game::item* item);
    bool                    item_remove(uint8_t index);
    bool                    item_reduce(uint8_t index, uint16_t count);
    fb::game::item*         item_active(uint8_t index, uint8_t* updated_index, fb::game::equipment::eq_slots& slot);
    uint8_t                 item2index(const fb::game::item::core* item) const;
    void                    equipment_on(uint8_t index, fb::game::equipment::eq_slots& slot, uint8_t* updated_index);
    uint8_t                 equipment_off(fb::game::equipment::eq_slots slot);


    fb::game::spell*        spell(uint8_t index) const;
    uint8_t                 spell_add(fb::game::spell* spell);
    bool                    spell_remove(uint8_t index);
    bool                    spell_swap(uint8_t src, uint8_t dest);

    fb::game::weapon*       weapon() const;
    fb::game::weapon*       weapon(fb::game::weapon* weapon);

    fb::game::armor*        armor() const;
    fb::game::armor*        armor(fb::game::armor* armor);

    fb::game::shield*       shield() const;
    fb::game::shield*       shield(fb::game::shield* shield);

    fb::game::helmet*       helmet() const;
    fb::game::helmet*       helmet(fb::game::helmet* helmet);

    fb::game::ring*         ring(equipment::EQUIPMENT_POSITION position) const;
    fb::game::ring*         ring(fb::game::ring* ring);

    fb::game::auxiliary*    auxiliary(equipment::EQUIPMENT_POSITION position) const;
    fb::game::auxiliary*    auxiliary(fb::game::auxiliary* auxiliary);

    bool                    inventory_free() const;
    uint8_t                 inventory_free_size() const;

    fb::game::map*          map() const;
    uint16_t                map(fb::game::map* map);
    uint16_t                map(fb::game::map* map, const point16_t position);

    trade_system&           trade_system();

    fb::game::group_system* group_system() const;
    bool                    group_enter(fb::game::group_system* gs);
    bool                    group_leave();

    void                    state_assert(fb::game::state flags) const;

public:
    fb::ostream             make_id_stream() const;
    fb::ostream             make_state_stream(fb::game::state_level level) const;
    fb::ostream             make_show_objects_stream() const;
    fb::ostream             make_position_stream() const;
    fb::ostream             make_visual_stream(bool light) const;
    fb::ostream             make_effet_stream(uint8_t effect) const;


    fb::ostream             make_update_item_slot_stream(uint8_t index) const;
    fb::ostream             make_delete_item_slot_stream(fb::game::item::delete_attr types, uint32_t slot, uint16_t count = 0) const;
    fb::ostream             make_update_equipment_stream(fb::game::equipment::eq_slots slot) const;
    fb::ostream             make_equipment_off_stream(fb::game::equipment::eq_slots slot) const;
    fb::ostream             make_internal_info_stream() const;
    fb::ostream             make_external_info_stream() const;
    fb::ostream             make_option_stream() const;
    fb::ostream             make_chat_stream(const std::string& message, bool shout) const;
};

} }

#endif // !__MMO_H__
