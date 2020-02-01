#include "session.h"
#include "map.h"
#include "item.h"
#include "db.h"

using namespace fb::game;

session::session(SOCKET socket) : 
    fb_session(socket),
    life((life::core*)nullptr, socket, 0, 0, 0),
    life::core(socket, "fb", 0, 0, 0, 0),
    _nation(fb::game::nation::GOGURYEO),
    _creature(fb::game::creature::DRAGON),
    _state(state::NORMAL),
    _level(1),
    _class(0),
    _promotion(0),
    _money(0),
    _weapon(nullptr), _armor(nullptr), _helmet(nullptr), _shield(NULL),
    _trade(*this)
{
    memset(this->_items, NULL, sizeof(this->_items));
    memset(this->_spells, NULL, sizeof(this->_items));
    memset(this->_rings, NULL, sizeof(this->_rings));
    memset(this->_auxiliaries, NULL, sizeof(this->_auxiliaries));
    memset(this->_options, 0, sizeof(this->_options));
}

session::~session()
{
    for(int i = 0; i < item::MAX_SLOT; i++)
    {
        if(this->_items[i])
            delete this->_items[i];
    }

    if(this->_weapon)
        delete this->_weapon;

    if(this->_armor)
        delete this->_armor;

    if(this->_helmet)
        delete this->_helmet;

    if(this->_shield)
        delete this->_shield;

    if(this->_rings[0])
        delete this->_rings[0];

    if(this->_rings[1])
        delete this->_rings[1];

    if(this->_auxiliaries[0])
        delete this->_auxiliaries[0];

    if(this->_auxiliaries[1])
        delete this->_auxiliaries[1];

    delete this->_core;
}

const std::string& fb::game::session::name() const
{
    return life::core::name();
}

void fb::game::session::name(const std::string& value)
{
    life::core::name(value);
}

uint16_t fb::game::session::look() const
{
    return life::core::look();
}

void fb::game::session::look(uint16_t value)
{
    life::core::look(value);
}

uint16_t fb::game::session::color() const
{
    return life::core::color();
}

void fb::game::session::color(uint16_t value)
{
    life::core::color(value);
}

uint32_t fb::game::session::defensive_physical() const
{
    return life::core::defensive_physical();
}

void fb::game::session::defensive_physical(uint8_t value)
{
    life::core::defensive_physical(value);
}

uint32_t fb::game::session::defensive_magical() const
{
    return life::core::defensive_magical();
}

void fb::game::session::defensive_magical(uint8_t value)
{
    life::core::defensive_magical(value);
}

void fb::game::session::base_hp_up(uint32_t value)
{
    life::core::_hp += value;
}

void fb::game::session::base_mp_up(uint32_t value)
{
    life::core::_mp += value;
}

void fb::game::session::base_hp(uint32_t value)
{
    life::core::_hp = value;
}

void fb::game::session::base_mp(uint32_t value)
{
    life::core::_mp = value;
}

uint32_t fb::game::session::base_hp() const
{
    return life::core::_hp;
}

uint32_t fb::game::session::base_mp() const
{
    return life::core::_mp;
}

void fb::game::session::hp(uint32_t value)
{
    life::_hp = value;
}

void fb::game::session::mp(uint32_t value)
{
    life::_mp = value;
}

uint32_t fb::game::session::hp() const
{
    return life::_hp;
}

uint32_t fb::game::session::mp() const
{
    return life::_mp;
}

uint32_t fb::game::session::id() const
{
    return life::id();
}

fb::game::nation fb::game::session::nation() const
{
    return this->_nation;
}

bool fb::game::session::nation(fb::game::nation value)
{
    if(value != fb::game::nation::GOGURYEO &&
        value != fb::game::nation::BUYEO)
        return false;

    this->_nation = value;
    return true;
}

fb::game::creature fb::game::session::creature() const
{
    return this->_creature;
}

bool fb::game::session::creature(fb::game::creature value)
{
    if(value != fb::game::creature::DRAGON &&
        value != fb::game::creature::PHOENIX &&
        value != fb::game::creature::TIGER &&
        value != fb::game::creature::TURTLE)
        return false;

    this->_creature = value;
    return true;
}

uint8_t fb::game::session::level() const
{
    return this->_level;
}

void fb::game::session::level(uint8_t value)
{
    this->_level = value;
}

bool fb::game::session::level_up()
{
    if(this->max_level())
        return false;

    this->_level++;
    return true;
}

bool fb::game::session::max_level() const
{
    return this->_level >= 99;
}

fb::game::sex fb::game::session::sex() const
{
    return this->_sex;
}

void fb::game::session::sex(fb::game::sex value)
{
    this->_sex = value;
}

fb::game::state fb::game::session::state() const
{
    return this->_state;
}

void fb::game::session::state(fb::game::state value)
{
    this->_state = value;
}

uint8_t fb::game::session::cls() const
{
    return this->_class;
}

void fb::game::session::cls(uint8_t value)
{
    this->_class = value;
}

uint8_t fb::game::session::promotion() const
{
    return uint8_t();
}

void fb::game::session::promotion(uint8_t value)
{
    this->_promotion = value;
}

uint8_t fb::game::session::strength() const
{
    return this->_strength;
}

void fb::game::session::strength(uint8_t value)
{
    this->_strength = value;
}

void fb::game::session::strength_up(uint8_t value)
{
    this->_strength + value;
}

uint8_t fb::game::session::intelligence() const
{
    return this->_intelligence;
}

void fb::game::session::intelligence(uint8_t value)
{
    this->_intelligence = value;
}

void fb::game::session::intelligence_up(uint8_t value)
{
    this->_intelligence += value;
}

uint8_t fb::game::session::dexteritry() const
{
    return this->_dexteritry;
}

void fb::game::session::dexteritry(uint8_t value)
{
    this->_dexteritry = value;
}

void fb::game::session::dexteritry_up(uint8_t value)
{
    this->_dexteritry += value;
}

uint32_t fb::game::session::experience() const
{
    return this->_experience;
}

uint32_t fb::game::session::experience_add(uint32_t value)
{
    uint32_t capacity = 0xFFFFFFFF - this->_experience;
    
    if(value > capacity)
    {
        this->_experience += capacity;
        return value - capacity;
    }
    else
    {
        this->_experience += value;
        return 0;
    }
}

uint32_t fb::game::session::experience_reduce(uint32_t value)
{
    if(this->_experience < value)
    {
        uint32_t lack = value - this->_experience;
        this->_experience = 0;
        return lack;
    }
    else
    {
        this->_experience -= value;
        return 0;
    }
}

uint32_t fb::game::session::experience_remained() const
{
    if(this->max_level())
        return 0;

    if(this->_class == 0 && this->_level >= 5)
        return 0;

    return db::required_exp(this->_class, this->_level+1) - this->experience();
}

uint32_t fb::game::session::money() const
{
    return this->_money;
}

void fb::game::session::money(uint32_t value)
{
    this->_money = value;
}

uint32_t fb::game::session::money_add(uint32_t value) // 먹고 남은 값 리턴
{
    uint32_t capacity = 0xFFFFFFFF - this->_money;
    if(value > capacity)
    {
        this->_money += capacity;
        return value - capacity;
    }
    else
    {
        this->_money += value;
        return 0;
    }
}

uint32_t fb::game::session::money_reduce(uint32_t value)
{
    if(this->_money < value)
    {
        uint32_t lack = value - this->_money;
        this->_money = 0;
        return lack;
    }
    else
    {
        this->_money -= value;
        return 0;
    }
}

uint32_t fb::game::session::random_damage() const
{
    return this->_damage;
}

void fb::game::session::random_damage(uint8_t value)
{
    this->_damage = value;
}

uint32_t fb::game::session::random_damage(fb::game::life& life, bool& critical) const
{
    uint32_t random_damage = 0;

    if(this->_weapon == nullptr) // no weapon
    {
        random_damage = 1 + std::rand() % 5;
        critical = false;
    }
    else if(std::rand() % 100 < 20) // critical
    {
        fb::game::range32_t range = this->_weapon->damage_large();
        random_damage = std::max(uint32_t(1), range.min) + std::rand() % std::max(uint32_t(1), range.max);
        critical = true;
    }
    else // normal
    {
        fb::game::range32_t range = this->_weapon->damage_small();
        random_damage = std::max(uint32_t(1), range.min) + std::rand() % std::max(uint32_t(1), range.max);
        critical = false;
    }

    return __super::random_damage(random_damage, life);
}

uint32_t fb::game::session::hit() const
{
    return this->_hit;
}

void fb::game::session::hit(uint8_t value)
{
    this->_hit = value;
}

uint32_t fb::game::session::regenerative() const
{
    return this->_regenerative;
}

void fb::game::session::regenerative(uint8_t value)
{
    this->_regenerative = value;
}

bool fb::game::session::option(options key) const
{
    if(key > 0x1B)
        return false;

    return this->_options[key];
}

void fb::game::session::option(options key, bool value)
{
    if(key > 0x1B)
        return;

    this->_options[key] = value;
}

bool fb::game::session::option_toggle(options key)
{
    if(key > 0x1B)
        return false;

    this->_options[key] = !this->_options[key];
    return this->_options[key];
}

const std::vector<legend>& fb::game::session::legends() const
{
    return this->_legends;
}

void fb::game::session::legends_add(uint8_t look, uint8_t color, const std::string& content)
{
    this->_legends.push_back(legend(look, color, content));
}

const std::string& fb::game::session::title() const
{
    return this->_title;
}

void fb::game::session::title(const std::string& value)
{
    this->_title = value;
}

item* fb::game::session::item(uint8_t index) const
{
    if(index > fb::game::item::MAX_SLOT - 1)
        return nullptr;

    return this->_items[index];
}

bool fb::game::session::item_add(fb::game::item& item)
{
    for(int i = 0; i < fb::game::item::MAX_SLOT; i++)
    {
        if(this->_items[i] != nullptr)
            continue;

        this->_items[i] = &item;
        return true;
    }

    return false;
}

uint8_t fb::game::session::item_add(fb::game::item* item)
{
    if(item == nullptr)
        return -1;


    // 번들 형식의 아이템인 경우
    if(item->attr() & item::attrs::ITEM_ATTR_BUNDLE)
    {
        for(int i = 0; i < fb::game::item::MAX_SLOT; i++)
        {
            if(this->_items[i] == nullptr)
                continue;

            if(item->based() != this->_items[i]->based())
                continue;


            // 아이템을 합치고 남은 갯수로 설정한다.
            uint16_t remain = this->_items[i]->fill(item->count());
            item->count(remain);

            return i;
        }
    }

    // 그 이외의 아이템인 경우
    for(int i = 0; i < fb::game::item::MAX_SLOT; i++)
    {
        if(this->_items[i] != nullptr)
            continue;

        this->_items[i] = item;
        return i;
    }

    return -1;
}

bool fb::game::session::item_remove(uint8_t index)
{
    fb::game::item* item = this->item(index);
    if(item == nullptr)
        return false;

    this->_items[index] = nullptr;
    return true;
}

bool fb::game::session::item_reduce(uint8_t index, uint16_t count)
{
    fb::game::item* item = this->item(index);
    if(item == nullptr)
        return false;

    this->_items[index]->reduce(count);
    return true;
}

fb::game::item* fb::game::session::item_active(uint8_t index, uint8_t* updated_index, equipment::eq_slots& slot)
{
    slot = equipment::eq_slots::UNKNOWN_SLOT;
    
    auto                    item = this->_items[index];
    if(item == nullptr)
        return nullptr;

    auto                    attr(item->attr());
    if((attr & item::attrs::ITEM_ATTR_EQUIPMENT))
        this->equipment_on(index, slot, updated_index);

    item->handle_acive(*this);
    if(item->empty())
        this->_items[index] = nullptr;

    return item;
}

uint8_t fb::game::session::item2index(const fb::game::item::core* item) const
{
    for(int i = 0; i < item::MAX_SLOT; i++)
    {
        if(this->_items[i] == nullptr)
            continue;

        if(this->_items[i]->based<item::core>() == item)
            return i;
    }

    return 0xFF;
}

void fb::game::session::equipment_on(uint8_t index, equipment::eq_slots& slot, uint8_t* updated_index)
{
    if(updated_index != nullptr)
        *updated_index = 0xFF;

    fb::game::item*         item = this->item(index);
    if(item == nullptr)
        throw std::exception();

    fb::game::item*         before = nullptr;
    auto                    attr(fb::game::item::attrs(item->attr() & ~item::attrs::ITEM_ATTR_EQUIPMENT));
    switch(attr)
    {
    case item::attrs::ITEM_ATTR_WEAPON:
        before = this->weapon(static_cast<fb::game::weapon*>(item));
        slot = equipment::eq_slots::WEAPON_SLOT;
        break;

    case item::attrs::ITEM_ATTR_ARMOR:
        before = this->armor(static_cast<fb::game::armor*>(item));
        slot = equipment::eq_slots::ARMOR_SLOT;
        break;

    case item::attrs::ITEM_ATTR_SHIELD:
        before = this->shield(static_cast<fb::game::shield*>(item));
        slot = equipment::eq_slots::SHIELD_SLOT;
        break;

    case item::attrs::ITEM_ATTR_HELMET:
        before = this->helmet(static_cast<fb::game::helmet*>(item));
        slot = equipment::eq_slots::HELMET_SLOT;
        break;

    case item::attrs::ITEM_ATTR_RING:
        if(this->_rings[0] == nullptr)
        {
            slot = equipment::eq_slots::LEFT_HAND_SLOT;
        }
        else
        {
            slot = equipment::eq_slots::RIGHT_HAND_SLOT;
        }
        
        before = this->ring(static_cast<fb::game::ring*>(item));
        break;


    case item::attrs::ITEM_ATTR_AUXILIARY:
        if(this->_auxiliaries[0] == nullptr)
        {
            slot = equipment::eq_slots::LEFT_AUX_SLOT;
        }
        else
        {
            slot = equipment::eq_slots::RIGHT_AUX_SLOT;
        }

        before = this->auxiliary(static_cast<fb::game::auxiliary*>(item));
        break;
    
    default:
        throw equipment::not_equipment_exception();
    }


    this->_items[index] = nullptr;
    uint8_t updated = this->item_add(before);
    if(updated_index != nullptr)
        *updated_index = updated;
}

uint8_t fb::game::session::equipment_off(equipment::eq_slots slot)
{
    if(this->inventory_free() == false)
        throw item::full_inven_exception();

    fb::game::item* item;
    switch(slot)
    {
    case equipment::eq_slots::WEAPON_SLOT:
        item = this->_weapon;
        if(this->_weapon != nullptr)
            this->_weapon = nullptr;

        break;

    case equipment::eq_slots::ARMOR_SLOT:
        item = this->_armor;
        if(this->_armor != nullptr)
            this->_armor = nullptr;
        break;

    case equipment::eq_slots::SHIELD_SLOT:
        item = this->_shield;
        if(this->_shield != nullptr)
            this->_shield = nullptr;
        break;

    case equipment::eq_slots::HELMET_SLOT:
        item = this->_helmet;
        if(this->_helmet != nullptr)
            this->_helmet = nullptr;
        break;

    case equipment::eq_slots::LEFT_HAND_SLOT:
        item = this->_rings[0];
        if(this->_rings[0] != nullptr)
            this->_rings[0] = nullptr;
        break;

    case equipment::eq_slots::RIGHT_HAND_SLOT:
        item = this->_rings[1];
        if(this->_rings[1] != nullptr)
            this->_rings[1] = nullptr;
        break;

    case equipment::eq_slots::LEFT_AUX_SLOT:
        item = this->_auxiliaries[0];
        if(this->_auxiliaries[0] != nullptr)
            this->_auxiliaries[0] = nullptr;
        break;

    case equipment::eq_slots::RIGHT_AUX_SLOT:
        item = this->_auxiliaries[1];
        if(this->_auxiliaries[1] != nullptr)
            this->_auxiliaries[1] = nullptr;
        break;
    }

    return this->item_add(item);
}

fb::game::spell* fb::game::session::spell(uint8_t index) const
{
    if(index > fb::game::spell::MAX_SLOT - 1)
        return nullptr;

    return this->_spells[index];
}

uint8_t fb::game::session::spell_add(fb::game::spell* spell)
{
    for(int i = 0; i < spell::MAX_SLOT; i++)
    {
        if(this->_spells[i] != nullptr)
            continue;

        this->_spells[i] = spell;
        return i;
    }

    return -1;
}

bool fb::game::session::spell_remove(uint8_t index)
{
    if(index > spell::MAX_SLOT - 1)
        return false;

    this->_spells[index] = nullptr;
    return true;
}

bool fb::game::session::spell_swap(uint8_t src, uint8_t dest)
{
    if(src == dest)
        return false;

    if(src > spell::MAX_SLOT - 1 || dest > spell::MAX_SLOT - 1)
        return false;

    std::swap(this->_spells[src], this->_spells[dest]);
    return true;
}

weapon* fb::game::session::weapon() const
{
    return this->_weapon;
}

fb::game::weapon* fb::game::session::weapon(fb::game::weapon* weapon)
{
    fb::game::weapon* before = this->_weapon;

    this->_weapon = weapon;
    return before;
}

armor* fb::game::session::armor() const
{
    return this->_armor;
}

fb::game::armor* fb::game::session::armor(fb::game::armor* armor)
{
    fb::game::armor* before = this->_armor;
    
    this->_armor = armor;
    return before;
}

shield* fb::game::session::shield() const
{
    return this->_shield;
}

fb::game::shield* fb::game::session::shield(fb::game::shield* shield)
{
    fb::game::shield* before = this->_shield;

    this->_shield = shield;
    return before;
}

helmet* fb::game::session::helmet() const
{
    return this->_helmet;
}

fb::game::helmet* fb::game::session::helmet(fb::game::helmet* helmet)
{
    fb::game::helmet* before = this->_helmet;

    this->_helmet = helmet;
    return before;
}

ring* fb::game::session::ring(equipment::EQUIPMENT_POSITION position) const
{
    return this->_rings[position];
}

fb::game::ring* fb::game::session::ring(fb::game::ring* ring)
{
    fb::game::ring* before = nullptr;

    if(this->_rings[0] == nullptr)
    {
        before = this->_rings[0];
        this->_rings[0] = ring;
    }
    else
    {
        before = this->_rings[1];
        this->_rings[1] = ring;
    }

    return before;
}


auxiliary* fb::game::session::auxiliary(equipment::EQUIPMENT_POSITION position) const
{
    return this->_auxiliaries[position];
}

fb::game::auxiliary* fb::game::session::auxiliary(fb::game::auxiliary* auxiliary)
{
    fb::game::auxiliary* before = nullptr;

    if(this->_auxiliaries[0] == nullptr)
    {
        before = this->_auxiliaries[0];
        this->_auxiliaries[0] = auxiliary;
    }
    else
    {
        before = this->_auxiliaries[1];
        this->_auxiliaries[1] = auxiliary;
    }

    return before;
}

bool fb::game::session::inventory_free() const
{
    for(int i = 0; i < item::MAX_SLOT; i++)
    {
        if(this->_items[i] == nullptr)
            return true;
    }

    return false;
}

uint8_t fb::game::session::inventory_free_size() const
{
    uint8_t count = 0;
    for(int i = 0; i < item::MAX_SLOT; i++)
    {
        if(this->_items[i] == nullptr)
            count++;
    }

    return count;
}

uint16_t fb::game::session::map(fb::game::map* map)
{
    if(this->_map != nullptr)
        this->_map->session_delete(this);

    this->_map = map;
    this->_map->session_add(this);
    return this->id();
}

uint16_t fb::game::session::map(fb::game::map* map, const point16_t position)
{
    uint16_t seq = this->map(map);
    this->position(position);

    return seq;
}

fb::game::trade& fb::game::session::trade()
{
    return this->_trade;
}

fb::game::group* fb::game::session::group() const
{
    return this->_group;
}

bool fb::game::session::group_enter(fb::game::group* gs)
{
    if(this->_group != nullptr)
        return false;

    this->_group = gs;
    return true;
}

bool fb::game::session::group_leave()
{
    if(this->_group == nullptr)
        return false;

    this->_group = nullptr;
    return true;
}

void fb::game::session::state_assert(fb::game::state flags) const
{
    if((flags & state::GHOST) == state::GHOST && this->_state == state::GHOST)
        throw ghost_exception();

    if((flags & state::RIDING) == state::RIDING && this->_state == state::RIDING)
        throw ridding_exception();

    if((flags & state::DISGUISE) == state::DISGUISE && this->_state == state::DISGUISE)
        throw disguise_exception();
}

void fb::game::session::state_assert(uint8_t flags) const
{
    return this->state_assert(fb::game::state(flags));
}

fb::game::map* fb::game::session::map() const
{
    return __super::map();
}

fb::ostream fb::game::session::make_id_stream() const
{
    fb::ostream             ostream;
    ostream.write_u8(0x05)
        .write_u32(this->id())
        .write_u32(this->direction()) // side
        .write_u32(0x00); // class

    return ostream;
}

fb::ostream fb::game::session::make_state_stream(fb::game::state_level level) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x08) // cmd
        .write_u8(level);

    if(level & state_level::BASED)
    {
        ostream.write_u8(this->nation()) // nation
            .write_u8(this->creature()) // creature
            .write_u8(0x00) // Unknown (clan?)
            .write_u8(this->level()) // level
            .write_u32(this->base_hp()) // base hp
            .write_u32(this->base_mp()) // base mp
            .write_u8(this->strength())
            .write_u8(this->intelligence())
            .write_u8(0x03)
            .write_u8(0x03)
            .write_u8(this->dexteritry())
            .write_u8(0x03)
            .write_u32(0x00)
            .write_u8(0x00);
    }

    if(level & state_level::HP_MP)
    {
        ostream.write_u32(this->hp())  // current hp
            .write_u32(this->mp()); // current mp
    }

    if(level & state_level::EXP_MONEY)
    {
        ostream.write_u32(this->experience())  // exp
            .write_u32(this->money()); // money
    }

    if(level & state_level::CONDITION)
    {
        ostream.write_u8(this->condition_contains(fb::game::condition::MOVE))  // condition::move
            .write_u8(this->condition_contains(fb::game::condition::SIGHT))  // condition::sight
            .write_u8(this->condition_contains(fb::game::condition::HEAR))  // condition::hear?
            .write_u8(this->condition_contains(fb::game::condition::ORAL))  // condition:oral
            .write_u8(this->condition_contains(fb::game::condition::MAP)); // condition:map?
    }

    ostream.write_u8(0x00)  // mail count
        .write_u8(true)  // fast move
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::session::make_show_objects_stream() const
{
    fb::game::map*          map = this->map();
    if(map == nullptr)
        return fb::ostream();

    std::vector<object*>    visibles;
    for(auto object : map->objects())
    {
        if(this->sight(*object) == false)
            continue;

        visibles.push_back(object);
    }

    return fb::game::object::make_show_stream(visibles);
}

fb::ostream fb::game::session::make_position_stream() const
{
    fb::ostream             ostream;
    ostream.write_u8(0x04)
        .write_u16(this->_position.x)  // 실제 x 좌표
        .write_u16(this->_position.y); // 실제 y 좌표

                                 // 스크린에서의 x 좌표
    fb::game::map*          map = this->map();
    if(map->width() < map::MAX_SCREEN_WIDTH)
        ostream.write_u16(this->_position.x + map::HALF_SCREEN_WIDTH - (map->width() / 2));
    else if(this->_position.x < map::HALF_SCREEN_WIDTH)
        ostream.write_u16(this->_position.x);
    else if(this->_position.x >= map->width() - map::HALF_SCREEN_WIDTH)
        ostream.write_u16(this->_position.x + map::MAX_SCREEN_WIDTH - map->width());
    else
        ostream.write_u16(map::HALF_SCREEN_WIDTH);

    // 스크린에서의 y 좌표
    if(map->height() < map::MAX_SCREEN_HEIGHT)
        ostream.write_u16(this->_position.y + map::HALF_SCREEN_HEIGHT - (map->height() / 2));
    else if(this->_position.y < map::HALF_SCREEN_HEIGHT)
        ostream.write_u16(this->_position.y);
    else if(this->_position.y >= (map->height() - map::HALF_SCREEN_HEIGHT))
        ostream.write_u16(this->_position.y + map::MAX_SCREEN_HEIGHT - map->height());
    else
        ostream.write_u16(map::HALF_SCREEN_HEIGHT);

    ostream.write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::session::make_visual_stream(bool light) const
{
    fb::ostream             ostream;

    ostream.write_u8(light ? 0x1D : 0x33); // cmd
    if(light == false)
    {
        ostream.write_u16(this->_position.x) // x
            .write_u16(this->_position.y) // y
            .write_u8(this->_direction); // side
    }

    ostream.write_u32(this->id())
        .write_u8(this->_state == fb::game::state::DISGUISE) // 변신유무
        .write_u8(this->_sex) // sex
        .write_u8(this->_state) // state
        .write_u16(this->look()) // face
        .write_u8(this->color()); // hair color

    if(this->_armor != nullptr)
    {
        ostream.write_u8(this->_armor->dress())
            .write_u8(this->_armor->color());
    }
    else
    {
        ostream.write_u8((uint8_t)this->_sex) // sex
            .write_u8(0x00);
    }

    if(this->_weapon != nullptr)
    {
        ostream.write_u16(this->_weapon->dress())
            .write_u8(this->_weapon->color());
    }
    else
    {
        ostream.write_u16(0xFFFF)
            .write_u8(0x00);
    }

    if(this->_shield != nullptr)
    {
        ostream.write_u8(this->_shield->dress())
            .write_u8(this->_shield->color());
    }
    else
    {
        ostream.write_u8(0xFF) // about shield
            .write_u8(0x00);
    }

    const auto& name = this->name();
    ostream.write_u8(0x04) // head mark
        .write(name, false); // name

    return ostream;
}

fb::ostream fb::game::session::make_effet_stream(uint8_t effect) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x29)
        .write_u32(this->id())
        .write_u8(effect)
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::session::make_update_item_slot_stream(uint8_t index) const
{
    fb::ostream             ostream;
    fb::game::item*         item = this->item(index);
    if(item == nullptr)
        return ostream;

    const std::string name = item->name_styled();

    ostream.write_u8(0x0F)
        .write_u8(index + 1)
        .write_u16(item->look())
        .write_u8(item->color())
        .write(name, false)
        .write_u32(item->count())
        .write_u8(0x00)
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::session::make_delete_item_slot_stream(fb::game::item::delete_attr types, uint32_t slot, uint16_t count) const
{
    fb::ostream             ostream;
    if(slot + 1 > item::MAX_SLOT)
        return ostream;

    ostream.write_u8(0x10)
        .write_u8(slot + 1)
        .write_u8(types)
        .write_u16(count);

    return ostream;
}

fb::ostream fb::game::session::make_update_equipment_stream(equipment::eq_slots slot) const
{
    fb::ostream             ostream;
    fb::game::item*         item;

    switch(slot)
    {
    case equipment::eq_slots::WEAPON_SLOT:
        item = this->weapon();
        break;

    case equipment::eq_slots::ARMOR_SLOT:
        item = this->armor();
        break;

    case equipment::eq_slots::SHIELD_SLOT:
        item = this->shield();
        break;

    case equipment::eq_slots::HELMET_SLOT:
        item = this->helmet();
        break;

    case equipment::eq_slots::LEFT_HAND_SLOT:
        item = this->ring(equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT);
        break;

    case equipment::eq_slots::RIGHT_HAND_SLOT:
        item = this->ring(equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT);
        break;

    case equipment::eq_slots::LEFT_AUX_SLOT:
        item = this->auxiliary(equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT);
        break;

    case equipment::eq_slots::RIGHT_AUX_SLOT:
        item = this->auxiliary(equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT);
        break;

    default:
        return false;
    }

    if(item == nullptr)
        return ostream;

    const std::string& name = item->name();
    ostream.write_u8(0x37)
        .write_u16(item->look())
        .write_u8(item->color())
        .write(name, false);

    return ostream;
}

fb::ostream fb::game::session::make_equipment_off_stream(equipment::eq_slots slot) const
{
    fb::ostream             ostream;
    ostream.write_u8(0x38)
        .write_u8(slot)
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::session::make_internal_info_stream() const
{
    //
    // 키보드 S를 눌렀을 때 표현될 자신의 캐릭터 정보
    //

    fb::ostream             ostream;

    ostream.write_u8(0x39)
        .write_u8(this->_defensive.physical)
        .write_u8(this->random_damage())
        .write_u8(this->hit())
        .write("클랜 정보") // 클랜정보 없음
        .write("클랜 타이틀") // 클랜 타이틀 없음
        .write(this->_title);

    std::string             group_message = "그룹 없음.";
    ostream.write(group_message);

    ostream.write_u8(this->option(options::GROUP)); // 그룹허가


    // 남은 경험치
    uint32_t remained_exp = this->experience_remained();
    ostream.write_u32(remained_exp);

    const std::string*             class_name = db::class2name(this->_class, this->_promotion);
    if(class_name == nullptr)
        return fb::ostream();

    ostream.write(*class_name);

    fb::game::equipment* equipments[] = {this->helmet(), this->ring(equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT), this->ring(equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT), this->auxiliary(equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT), this->auxiliary(equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT)};
    for(int i = 0, size = sizeof(equipments) / sizeof(fb::game::equipment*); i < size; i++)
    {
        if(equipments[i] == nullptr)
        {
            ostream.write_u16(0xFFFF)
                .write_u8(0x00);
        }
        else
        {
            ostream.write_u16(equipments[i]->look())
                .write_u8(equipments[i]->color());
        }
    }

    ostream.write_u8(0x00) // fixed;
        .write_u8(this->_options[options::TRADE]) // trade
        .write_u8(this->_options[options::PK]); // pk

    ostream.write_u8(this->_legends.size());
    for(auto legend : this->_legends)
    {
        ostream.write_u8(legend.look)
            .write_u8(legend.color)
            .write(legend.content);
    }
    ostream.write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::session::make_external_info_stream() const
{
    //
    // 다른 캐릭터를 클릭했을 때 표현될 정보
    //

    fb::ostream             ostream;

    ostream.write_u8(0x34)
        .write(this->_title)
        .write("클랜 타이틀")
        .write("클랜 이름");

    // 클래스 이름
    const auto class_name = db::class2name(this->_class, this->_promotion);
    if(class_name == nullptr)
        return fb::ostream();

    ostream.write(*class_name)  // 직업
        .write(this->_name);    // 이름

    // 캐릭터 상태
    bool transform = false;
    if(transform) // 변신상태
    {
        ostream.write_u8(0x01);
    }
    else // 일반상태
    {
        ostream.write_u8(0x00)
            .write_u8(this->_sex)
            .write_u8(0x00) // state
            .write_u16(this->look())
            .write_u8(this->color());
    }


    // 장비정보
    std::stringstream   sstream;
    auto                armor = this->armor(); // 갑옷
    ostream.write_u8(armor != nullptr ? armor->dress() : 0xFF)
           .write_u8(armor != nullptr ? armor->color() : 0x00);

    auto                weapon = this->weapon(); // 무기
    ostream.write_u16(weapon != nullptr ? weapon->dress() : 0xFFFF)
           .write_u8(weapon != nullptr ? weapon->color() : 0x00);

    auto                shield = this->shield(); // 방패
    ostream.write_u8(shield != nullptr ? shield->dress() : 0xFF)
           .write_u8(shield != nullptr ? shield->color() : 0x00);

    auto                helmet = this->helmet(); // 투구
    ostream.write_u16(helmet != nullptr ? helmet->look() : 0xFFFF)
           .write_u8(helmet != nullptr ? helmet->color() : 0x00);

    auto                ring_l = this->ring(equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT); // 왼손
    ostream.write_u16(ring_l != nullptr ? ring_l->look() : 0xFFFF)
           .write_u8(ring_l != nullptr ? ring_l->color() : 0x00);

    auto                ring_r = this->ring(equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT); // 오른손
    ostream.write_u16(ring_r != nullptr ? ring_r->look() : 0xFFFF)
           .write_u8(ring_r != nullptr ? ring_r->color() : 0x00);

    auto                aux_l = this->auxiliary(equipment::EQUIPMENT_POSITION::EQUIPMENT_LEFT); // 보조1
    ostream.write_u16(aux_l != nullptr ? aux_l->look() : 0xFFFF)
           .write_u8(aux_l != nullptr ? aux_l->color() : 0x00);

    auto                aux_r = this->auxiliary(equipment::EQUIPMENT_POSITION::EQUIPMENT_RIGHT); // 보조2
    ostream.write_u16(aux_r != nullptr ? aux_r->look() : 0xFFFF)
           .write_u8(aux_r != nullptr ? aux_r->color() : 0x00);


    // 장비정보 텍스트
    sstream << " w:무기  :" << (weapon != nullptr ? weapon->name() : "없음") << std::endl;
    sstream << " a:갑옷  :" << (armor  != nullptr ? armor->name()  : "없음") << std::endl;
    sstream << " s:방패  :" << (shield != nullptr ? shield->name() : "없음") << std::endl;
    sstream << " h:머리  :" << (helmet != nullptr ? helmet->name() : "없음") << std::endl;
    sstream << " l:왼손  :" << (ring_l != nullptr ? ring_l->name() : "없음") << std::endl;
    sstream << " r:오른손:" << (ring_r != nullptr ? ring_r->name() : "없음") << std::endl;
    sstream << " [:보조1 :" << (aux_l  != nullptr ? aux_l->name()  : "없음") << std::endl;
    sstream << " ]:보조2 :" << (aux_r  != nullptr ? aux_r->name()  : "없음") << std::endl;
    ostream.write(sstream.str());


    ostream.write_u32(this->id())
        .write_u8(this->_options[fb::game::options::GROUP])
        .write_u8(this->_options[fb::game::options::TRADE])
        .write_u32(0x00000000); // unknown

                                // 업적
    ostream.write_u8(this->_legends.size());
    for(auto legend : this->_legends)
    {
        ostream.write_u8(legend.look)
            .write_u8(legend.color)
            .write(legend.content);
    }
    ostream.write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::session::make_option_stream() const
{
    fb::ostream             ostream;
    ostream.write_u8(0x23)
        .write_u8(this->_options[options::WEATHER_EFFECT]) // weather
        .write_u8(this->_options[options::MAGIC_EFFECT]) // magic effect
        .write_u8(this->_options[options::ROAR_WORLDS]) // listen news
        .write_u8(this->_options[options::FAST_MOVE]) // fast move
        .write_u8(this->_options[options::EFFECT_SOUND]) // effect sound
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::session::make_chat_stream(const std::string& message, bool shout) const
{
    fb::ostream             ostream;
    
    std::stringstream       sstream;
    sstream << this->_name << ": " << message;

    ostream.write_u8(0x0D)
        .write_u8(shout)
        .write_u32(this->id())
        .write(sstream.str());

    return ostream;
}
