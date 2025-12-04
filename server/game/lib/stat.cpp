#include <fb/game/server.h>
#include <fb/game/stat.h>
#include <fb/game/character.h>
#include <fb/game/mob.h>
#include <fb/game/ai.h>
#include <fb/model/model.h>
#include <fb/encoding.h>
#include <json/json.h>

using namespace fb::game;

fb::game::stat::stat(life& owner) :
    owner(owner)
{ }

fb::game::stat::stat(const stat& other) :
    owner(other.owner),
    _hp(other._hp),
    _mp(other._mp),
    _buff_hp(other._buff_hp),
    _buff_mp(other._buff_mp),
    _buff_str(other._buff_str),
    _buff_dex(other._buff_dex),
    _buff_int(other._buff_int),
    _buff_phydef(other._buff_phydef),
    _buff_magdef(other._buff_magdef),
    _buff_dam(other._buff_dam),
    _buff_hit(other._buff_hit)
{ }

fb::game::stat::stat(stat&& other) :
    owner(other.owner),
    _hp(other._hp),
    _mp(other._mp),
    _buff_hp(other._buff_hp),
    _buff_mp(other._buff_mp),
    _buff_str(other._buff_str),
    _buff_dex(other._buff_dex),
    _buff_int(other._buff_int),
    _buff_phydef(other._buff_phydef),
    _buff_magdef(other._buff_magdef),
    _buff_dam(other._buff_dam),
    _buff_hit(other._buff_hit)
{ }

uint32_t fb::game::stat::buff_hp() const
{
    this->owner.assert_thread();
    return this->_buff_hp;
}

void fb::game::stat::buff_hp(uint32_t value)
{
    this->owner.assert_thread();
    this->_buff_hp = value;
}

uint32_t fb::game::stat::buff_mp() const
{
    this->owner.assert_thread();
    return this->_buff_mp;
}

void fb::game::stat::buff_mp(uint32_t value)
{
    this->owner.assert_thread();
    this->_buff_mp = value;
}

uint8_t fb::game::stat::buff_str() const
{
    this->owner.assert_thread();
    return this->_buff_str;
}

void fb::game::stat::buff_str(uint8_t value)
{
    this->owner.assert_thread();
    this->_buff_str = value;
}

uint8_t fb::game::stat::buff_dex() const
{
    this->owner.assert_thread();
    return this->_buff_dex;
}

void fb::game::stat::buff_dex(uint8_t value)
{
    this->owner.assert_thread();
    this->_buff_dex = value;
}

uint8_t fb::game::stat::buff_int() const
{
    this->owner.assert_thread();
    return this->_buff_int;
}

void fb::game::stat::buff_int(uint8_t value)
{
    this->owner.assert_thread();
    this->_buff_int = value;
}

int8_t fb::game::stat::buff_phydef() const
{
    this->owner.assert_thread();
    return this->_buff_phydef;
}

void fb::game::stat::buff_phydef(int8_t value)
{
    this->owner.assert_thread();
    this->_buff_phydef = value;
}

int8_t fb::game::stat::buff_magdef() const
{
    this->owner.assert_thread();
    return this->_buff_magdef;
}

void fb::game::stat::buff_magdef(int8_t value)
{
    this->owner.assert_thread();
    this->_buff_magdef = value;
}

uint8_t fb::game::stat::buff_dam() const
{
    this->owner.assert_thread();
    return this->_buff_dam;
}

void fb::game::stat::buff_dam(uint8_t value)
{
    this->owner.assert_thread();
    this->_buff_dam = value;
}

uint8_t fb::game::stat::buff_hit() const
{
    this->owner.assert_thread();
    return this->_buff_hit;
}

void fb::game::stat::buff_hit(uint8_t value)
{
    this->owner.assert_thread();
    this->_buff_hit = value;
}

uint32_t fb::game::stat::buff_regenerative() const
{
    this->owner.assert_thread();
    return this->_buff_regenerative;
}

void fb::game::stat::buff_regenerative(uint32_t value)
{
    this->owner.assert_thread();
    this->_buff_regenerative = value;
}

uint32_t fb::game::stat::hp() const
{
    this->owner.assert_thread();
    return this->_hp;
}

void fb::game::stat::hp(uint32_t value)
{
    this->owner.assert_thread();
    this->_hp = value;
    this->owner.update(UPDATE_STATE_LEVEL::HP_MP);
}

uint32_t fb::game::stat::heal(uint32_t value, fb::game::object* from)
{
    this->owner.assert_thread();
    auto before = this->hp();
    this->hp(this->hp() + std::min(value, this->maxhp() - this->hp()));
    this->owner.update_hp(this->hp() - before, false);
    return this->hp() - before;
}

uint32_t fb::game::stat::damage(uint32_t value, std::shared_ptr<fb::game::object> from, bool critical)
{
    this->owner.assert_thread();
    if (from != nullptr && from->is(OBJECT_TYPE::CHARACTER))
    {
        auto ch = std::static_pointer_cast<fb::game::character>(from);
        for (auto mob : ch->spawned_mobs())
        {
            if (mob.get() == &this->owner)
                continue;

            mob->target(std::static_pointer_cast<fb::game::life>(this->owner.shared_from_this_as<fb::game::life>()));
        }
    }

    if (this->owner.invincible())
        return 0;

    auto before = this->hp();
    this->hp(this->hp() - std::min(value, this->hp()));
    this->owner.update_hp(before - this->hp(), critical);
    return before - this->hp();
}

uint32_t fb::game::stat::mp() const
{
    this->owner.assert_thread();
    return this->_mp;
}

void fb::game::stat::mp(uint32_t value)
{
    this->owner.assert_thread();
    this->_mp = value;
    this->owner.update(UPDATE_STATE_LEVEL::HP_MP);
}

uint32_t fb::game::stat::mp_up(uint32_t value, fb::game::object* from)
{
    this->owner.assert_thread();
    auto before = this->mp();
    this->mp(this->mp() + std::min(value, this->maxmp() - this->mp()));
    return this->mp() - before;
}

uint32_t fb::game::stat::mp_down(uint32_t value, fb::game::object* from)
{
    this->owner.assert_thread();
    auto before = this->mp();
    this->mp(this->mp() - std::min(value, this->mp()));
    return before - this->mp();
}

uint32_t fb::game::stat::maxhp() const
{
    this->owner.assert_thread();
    auto base = this->base_hp();
    auto buff = this->buff_hp();
    auto max  = std::numeric_limits<uint32_t>::max();
    if (max - base < buff)
        return max;

    return base + buff;
}

uint32_t fb::game::stat::maxmp() const
{
    this->owner.assert_thread();
    auto base = this->base_mp();
    auto buff = this->buff_mp();
    auto max  = std::numeric_limits<uint32_t>::max();
    if (max - base < buff)
        return max;

    return base + buff;
}

uint8_t fb::game::stat::str() const
{
    this->owner.assert_thread();
    auto base = this->base_str();
    auto buff = this->buff_str();
    auto max  = std::numeric_limits<uint8_t>::max();
    if (max - base < buff)
        return max;

    return base + buff;
}

uint8_t fb::game::stat::dex() const
{
    this->owner.assert_thread();
    auto base = this->base_dex();
    auto buff = this->buff_dex();
    auto max  = std::numeric_limits<uint8_t>::max();
    if (max - base < buff)
        return max;

    return base + buff;
}

uint8_t fb::game::stat::intelligence() const
{
    this->owner.assert_thread();
    auto base = this->base_int();
    auto buff = this->buff_int();
    auto max  = std::numeric_limits<uint8_t>::max();
    if (max - base < buff)
        return max;

    return base + buff;
}

int8_t fb::game::stat::phydef() const
{
    this->owner.assert_thread();
    auto base = this->base_phydef();
    auto buff = this->buff_phydef();
    auto sum  = (int16_t)base + (int16_t)buff;
    return (int8_t)std::max<int16_t>(-127, std::min<int16_t>(128, sum));
}

int8_t fb::game::stat::magdef() const
{
    this->owner.assert_thread();
    auto base = this->base_magdef();
    auto buff = this->buff_magdef();
    auto sum  = (int16_t)base + (int16_t)buff;
    return (int8_t)std::max<int16_t>(-127, std::min<int16_t>(128, sum));
}

uint8_t fb::game::stat::dam() const
{
    this->owner.assert_thread();
    auto base = this->base_dam();
    auto buff = this->buff_dam();
    auto max  = std::numeric_limits<uint8_t>::max();
    if (max - base < buff)
        return max;

    return base + buff;
}

uint8_t fb::game::stat::hit() const
{
    this->owner.assert_thread();
    auto base = this->base_hit();
    auto buff = this->buff_hit();
    auto max  = std::numeric_limits<uint8_t>::max();
    if (max - base < buff)
        return max;

    return base + buff;
}

uint32_t fb::game::stat::regenerative() const
{
    this->owner.assert_thread();
    auto base = this->base_regenerative();
    auto buff = this->buff_regenerative();
    auto max  = std::numeric_limits<uint32_t>::max();
    if (max - base < buff)
        return max;

    return base + buff;
}

character_stat::character_stat(character& owner) :
    fb::game::stat(owner),
    owner(owner)
{ }

void character_stat::base_hp(uint32_t value)
{
    this->owner.assert_thread();

    if (this->_max_hp == value)
        return;

    auto old_base_hp = this->_max_hp;
    this->_max_hp    = value;
    this->owner.update(UPDATE_STATE_LEVEL::BASED);

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(this->owner.id());
    log_data["character_name"] = UTF8(this->owner.name(), PLATFORM::WINDOWS);
    log_data["old_base_hp"]    = static_cast<Json::Int64>(old_base_hp);
    log_data["new_base_hp"]    = static_cast<Json::Int64>(value);
    this->owner.server.log.write("base_hp_change", log_data);
}

void character_stat::base_mp(uint32_t value)
{
    this->owner.assert_thread();

    if (this->_max_mp == value)
        return;

    auto old_base_mp = this->_max_mp;
    this->_max_mp    = value;
    this->owner.update(UPDATE_STATE_LEVEL::BASED);

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(this->owner.id());
    log_data["character_name"] = UTF8(this->owner.name(), PLATFORM::WINDOWS);
    log_data["old_base_mp"]    = static_cast<Json::Int64>(old_base_mp);
    log_data["new_base_mp"]    = static_cast<Json::Int64>(value);
    this->owner.server.log.write("base_mp_change", log_data);
}

void character_stat::base_str(uint8_t value)
{
    this->owner.assert_thread();
    this->_str = value;
    this->owner.update(UPDATE_STATE_LEVEL::BASED);
}

void character_stat::base_dex(uint8_t value)
{
    this->owner.assert_thread();
    this->_dex = value;
    this->owner.update(UPDATE_STATE_LEVEL::BASED);
}

void character_stat::base_int(uint8_t value)
{
    this->owner.assert_thread();
    this->_int = value;
    this->owner.update(UPDATE_STATE_LEVEL::BASED);
}

void character_stat::base_phydef(int8_t value)
{
    this->owner.assert_thread();
    this->_phydef = value;
    this->owner.update(UPDATE_STATE_LEVEL::BASED);
}

void character_stat::base_magdef(int8_t value)
{
    this->owner.assert_thread();
    this->_magdef = value;
}

void character_stat::base_dam(uint8_t value)
{
    this->owner.assert_thread();
    this->_dam = value;
}

void character_stat::base_hit(uint8_t value)
{
    this->owner.assert_thread();
    this->_hit = value;
}

void character_stat::base_regenerative(uint32_t value)
{
    this->owner.assert_thread();
    this->_regenerative = value;
}

uint32_t character_stat::base_hp() const
{
    this->owner.assert_thread();
    return this->_max_hp;
}

uint32_t character_stat::base_mp() const
{
    this->owner.assert_thread();
    return this->_max_mp;
}

uint8_t character_stat::base_str() const
{
    this->owner.assert_thread();
    return this->_str;
}

uint8_t character_stat::str() const
{
    this->owner.assert_thread();
    auto str        = fb::game::stat::str();
    auto additional = (uint32_t)0;
    for (auto& [_, equipment] : this->owner.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.strength;
    }

    auto limit = std::numeric_limits<uint32_t>::max();
    if (limit - str < additional)
        return limit;

    return str + additional;
}

uint8_t character_stat::base_dex() const
{
    this->owner.assert_thread();
    return this->_dex;
}

uint8_t character_stat::dex() const
{
    this->owner.assert_thread();
    auto dex        = fb::game::stat::dex();
    auto additional = (uint32_t)0;
    for (auto& [_, equipment] : this->owner.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.dexterity;
    }

    auto limit = std::numeric_limits<uint32_t>::max();
    if (limit - dex < additional)
        return limit;

    return dex + additional;
}

uint8_t character_stat::base_int() const
{
    this->owner.assert_thread();
    return this->_int;
}

uint8_t character_stat::intelligence() const
{
    this->owner.assert_thread();
    auto intelligence = fb::game::stat::intelligence();
    auto additional   = (uint32_t)0;
    for (auto& [_, equipment] : this->owner.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.intelligence;
    }

    auto limit = std::numeric_limits<uint32_t>::max();
    if (limit - intelligence < additional)
        return limit;

    return intelligence + additional;
}

int8_t character_stat::base_phydef() const
{
    this->owner.assert_thread();
    return this->_phydef;
}

int8_t character_stat::phydef() const
{
    this->owner.assert_thread();
    auto phydef     = fb::game::stat::phydef();
    auto additional = (uint32_t)0;
    for (auto& [_, equipment] : this->owner.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.defensive_physical;
    }

    auto sum = (int16_t)phydef + (int16_t)additional;
    return (int8_t)std::max<int16_t>(-127, std::min<int16_t>(128, sum));
}

int8_t character_stat::base_magdef() const
{
    this->owner.assert_thread();
    return this->_magdef;
}

int8_t character_stat::magdef() const
{
    this->owner.assert_thread();
    auto magdef     = fb::game::stat::magdef();
    auto additional = (uint32_t)0;
    for (auto& [_, equipment] : this->owner.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.defensive_magical;
    }

    auto sum = (int16_t)magdef + (int16_t)additional;
    return (int8_t)std::max<int16_t>(-127, std::min<int16_t>(128, sum));
}

uint8_t character_stat::base_dam() const
{
    this->owner.assert_thread();
    return this->_dam;
}

uint8_t character_stat::dam() const
{
    this->owner.assert_thread();
    auto dam        = fb::game::stat::dam();
    auto additional = (uint32_t)0;
    for (auto& [_, equipment] : this->owner.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.damage;
    }

    auto limit = std::numeric_limits<uint32_t>::max();
    if (limit - dam < additional)
        return limit;

    return dam + additional;
}

uint8_t character_stat::base_hit() const
{
    this->owner.assert_thread();
    return this->_hit;
}

uint8_t character_stat::hit() const
{
    this->owner.assert_thread();
    auto hit        = fb::game::stat::hit();
    auto additional = (uint32_t)0;
    for (auto& [_, equipment] : this->owner.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.hit;
    }

    auto limit = std::numeric_limits<uint32_t>::max();
    if (limit - hit < additional)
        return limit;

    return hit + additional;
}

uint32_t character_stat::base_regenerative() const
{
    this->owner.assert_thread();
    return this->_regenerative;
}

uint32_t character_stat::damage(uint32_t value, std::shared_ptr<fb::game::object> from, bool critical)
{
    this->owner.assert_thread();

    if (this->owner.alive() == false)
        return 0;

    auto result = fb::game::stat::damage(value, from, critical);
    if (from == nullptr)
        return result;

    for (auto mob : this->owner.spawned_mobs())
    {
        if (mob->target() != nullptr)
            continue;

        if (from->is(OBJECT_TYPE::LIFE) == false)
            continue;

        mob->target(std::static_pointer_cast<fb::game::life>(from));
    }

    if (this->hp() == 0)
    {
        this->owner.kill(from, DESTROY_TYPE::DEAD);
        return result;
    }

    for (auto& [parts, equipment] : this->owner.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto durability = equipment->durability();
        if (durability.has_value() == false)
            continue;

        auto& model = equipment->based<fb::model::equipment>();
        if (equipment->durability_down(1))
        {
            auto equipment = this->owner.items.equipment_off(parts);
            this->owner.message(std::format(_TEXT(MESSAGE_EQUIPMENT_BROKEN), equipment->name()));
            equipment.reset();
        }
    }
    return result;
}

mob_stat::mob_stat(mob& owner) :
    fb::game::stat(owner),
    owner(owner)
{ }

mob_stat::mob_stat(const mob_stat& other) :
    fb::game::stat(other),
    owner(other.owner)
{ }

mob_stat::mob_stat(mob_stat&& other) :
    fb::game::stat(other),
    owner(other.owner)
{ }

uint32_t mob_stat::base_hp() const
{
    this->owner.assert_thread();
    auto& model = this->owner.based<fb::model::mob>();
    return model.hp;
}

uint32_t mob_stat::base_mp() const
{
    this->owner.assert_thread();
    auto& model = this->owner.based<fb::model::mob>();
    return model.mp;
}

uint8_t mob_stat::base_str() const
{
    this->owner.assert_thread();
    // auto& model = this->owner.based<fb::model::mob>();
    // return model.str;
    return 0;
}

uint8_t mob_stat::base_dex() const
{
    this->owner.assert_thread();
    // auto& model = this->owner.based<fb::model::mob>();
    // return model.dex;
    return 0;
}

uint8_t mob_stat::base_int() const
{
    this->owner.assert_thread();
    // auto& model = this->owner.based<fb::model::mob>();
    // return model.int;
    return 0;
}

int8_t mob_stat::base_phydef() const
{
    this->owner.assert_thread();
    auto& model = this->owner.based<fb::model::mob>();
    return model.defensive_physical;
}

int8_t mob_stat::base_magdef() const
{
    this->owner.assert_thread();
    auto& model = this->owner.based<fb::model::mob>();
    return model.defensive_magical;
}

uint8_t mob_stat::base_dam() const
{
    this->owner.assert_thread();
    // auto& model = this->owner.based<fb::model::mob>();
    // return model.dam;
    return 0;
}

uint8_t mob_stat::base_hit() const
{
    this->owner.assert_thread();
    // auto& model = this->owner.based<fb::model::mob>();
    // return model.hit;
    return 0;
}

uint32_t mob_stat::base_regenerative() const
{
    this->owner.assert_thread();
    return 0;
}

uint32_t mob_stat::damage(uint32_t value, std::shared_ptr<object> from, bool critical)
{
    this->owner.assert_thread();

    auto result = fb::game::stat::damage(value, from, critical);
    if (!this->owner.alive())
    {
        this->owner.kill(from, DESTROY_TYPE::DEAD);
        return result;
    }

    // Handle damage in AI strategy
    if (this->owner._ai_strategy && from && from->is(OBJECT_TYPE::LIFE))
    {
        this->owner._ai_strategy->on_damage(this->owner, std::static_pointer_cast<fb::game::life>(from), fb::model::datetime());
    }

    return result;
}
