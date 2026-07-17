#include <fb/game/server.h>
#include <fb/game/stat.h>
#include <fb/game/character.h>
#include <fb/game/mob.h>
#include <fb/game/ai.h>
#include <fb/game/client_amount.h>
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

int64_t fb::game::stat::buff_hp() const
{
    this->owner.assert_thread();
    return this->_buff_hp;
}

void fb::game::stat::buff_hp(int64_t value)
{
    this->owner.assert_thread();
    this->_buff_hp = value;
}

int64_t fb::game::stat::buff_mp() const
{
    this->owner.assert_thread();
    return this->_buff_mp;
}

void fb::game::stat::buff_mp(int64_t value)
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

int8_t fb::game::stat::buff_dam() const
{
    this->owner.assert_thread();
    return this->_buff_dam;
}

void fb::game::stat::buff_dam(int8_t value)
{
    this->owner.assert_thread();
    this->_buff_dam = value;
}

int8_t fb::game::stat::buff_hit() const
{
    this->owner.assert_thread();
    return this->_buff_hit;
}

void fb::game::stat::buff_hit(int8_t value)
{
    this->owner.assert_thread();
    this->_buff_hit = value;
}

uint64_t fb::game::stat::buff_regenerative() const
{
    this->owner.assert_thread();
    return this->_buff_regenerative;
}

void fb::game::stat::buff_regenerative(uint64_t value)
{
    this->owner.assert_thread();
    this->_buff_regenerative = value;
}

uint64_t fb::game::stat::hp() const
{
    this->owner.assert_thread();
    return this->_hp;
}

void fb::game::stat::hp(uint64_t value, bool notify)
{
    this->owner.assert_thread();
    this->_hp = std::min(value, this->maxhp());
    if (notify)
    {
        auto level = UPDATE_STATE_LEVEL::HP_MP;
        if (client_pool_exceeds_u32(this->_hp, this->maxhp()))
            level |= UPDATE_STATE_LEVEL::BASED;
        this->owner.update(level);
    }
}

uint64_t fb::game::stat::heal(uint64_t value, fb::game::object* from, bool notify)
{
    this->owner.assert_thread();
    auto before = this->hp();
    auto maximum = this->maxhp();
    auto room    = maximum > before ? maximum - before : 0;
    this->hp(before + std::min(value, room), notify);
    this->owner.update_hp(this->hp() - before, false, notify);
    return this->hp() - before;
}

uint64_t fb::game::stat::damage(uint64_t                          value,
                                std::shared_ptr<fb::game::object> from,
                                bool                              critical,
                                float                             rate,
                                bool                              physical,
                                bool                              fixed,
                                bool                              notify)
{
    this->owner.assert_thread();
    if (from != nullptr && from->is(OBJECT_TYPE::CHARACTER))
    {
        auto ch = std::static_pointer_cast<fb::game::character>(from);
        for (const auto& mob : ch->spawned_mobs())
        {
            if (mob.get() == &this->owner)
                continue;

            mob->target(std::static_pointer_cast<fb::game::life>(this->owner.shared_from_this_as<fb::game::life>()));
        }
    }

    if (this->owner.invincible())
        return 0;

    uint64_t final_value = value;
    if (!fixed && from != nullptr && from->is(OBJECT_TYPE::LIFE))
    {
        auto attacker = std::static_pointer_cast<fb::game::life>(from);
        final_value   = attacker->calculate_damage(value, this->owner, critical, rate, physical);
    }

    auto before = this->hp();
    this->hp(this->hp() - std::min(final_value, this->hp()), notify);
    this->owner.update_hp(before - this->hp(), critical, notify);
    return before - this->hp();
}

uint64_t fb::game::stat::mp() const
{
    this->owner.assert_thread();
    return this->_mp;
}

void fb::game::stat::mp(uint64_t value, bool notify)
{
    this->owner.assert_thread();
    this->_mp = std::min(value, this->maxmp());
    if (notify)
    {
        auto level = UPDATE_STATE_LEVEL::HP_MP;
        if (client_pool_exceeds_u32(this->_mp, this->maxmp()))
            level |= UPDATE_STATE_LEVEL::BASED;
        this->owner.update(level);
    }
}

uint64_t fb::game::stat::mp_up(uint64_t value, fb::game::object* from, bool notify)
{
    this->owner.assert_thread();
    auto before = this->mp();
    this->mp(this->mp() + std::min(value, this->maxmp() - this->mp()), notify);
    return this->mp() - before;
}

uint64_t fb::game::stat::mp_down(uint64_t value, fb::game::object* from, bool notify)
{
    this->owner.assert_thread();
    auto before = this->mp();
    this->mp(this->mp() - std::min(value, this->mp()), notify);
    return before - this->mp();
}

uint64_t fb::game::stat::maxhp() const
{
    this->owner.assert_thread();
    auto base = static_cast<int64_t>(std::min(this->base_hp(), static_cast<uint64_t>(std::numeric_limits<int64_t>::max())));
    auto buff = this->buff_hp();
    if (buff > 0 && base > std::numeric_limits<int64_t>::max() - buff)
        return static_cast<uint64_t>(std::numeric_limits<int64_t>::max());

    auto sum = base + buff;
    if (sum < 1)
        return 1;
    return static_cast<uint64_t>(sum);
}

uint64_t fb::game::stat::maxmp() const
{
    this->owner.assert_thread();
    auto base = static_cast<int64_t>(std::min(this->base_mp(), static_cast<uint64_t>(std::numeric_limits<int64_t>::max())));
    auto buff = this->buff_mp();
    if (buff > 0 && base > std::numeric_limits<int64_t>::max() - buff)
        return static_cast<uint64_t>(std::numeric_limits<int64_t>::max());

    auto sum = base + buff;
    if (sum < 1)
        return 1;
    return static_cast<uint64_t>(sum);
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

int8_t fb::game::stat::dam() const
{
    this->owner.assert_thread();
    auto base = static_cast<int16_t>(this->base_dam());
    auto buff = static_cast<int16_t>(this->buff_dam());
    auto sum  = base + buff;
    return static_cast<int8_t>(std::max<int16_t>(-128, std::min<int16_t>(127, sum)));
}

int8_t fb::game::stat::hit() const
{
    this->owner.assert_thread();
    auto base = static_cast<int16_t>(this->base_hit());
    auto buff = static_cast<int16_t>(this->buff_hit());
    auto sum  = base + buff;
    return static_cast<int8_t>(std::max<int16_t>(-128, std::min<int16_t>(127, sum)));
}

uint64_t fb::game::stat::regenerative() const
{
    this->owner.assert_thread();
    auto base = this->base_regenerative();
    auto buff = this->buff_regenerative();
    auto max  = std::numeric_limits<uint64_t>::max();
    if (max - base < buff)
        return max;

    return base + buff;
}

character_stat::character_stat(character& owner) :
    fb::game::stat(owner),
    owner(owner)
{ }

void character_stat::base_hp(uint64_t value, bool notify)
{
    this->owner.assert_thread();

    if (this->_max_hp == value)
        return;

    auto old_base_hp = this->_max_hp;
    this->_max_hp    = value;
    this->hp(this->hp(), false);
    if (notify)
        this->owner.update(UPDATE_STATE_LEVEL::BASED | UPDATE_STATE_LEVEL::HP_MP);

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(this->owner.id);
    log_data["character_name"] = UTF8(this->owner.name(), PLATFORM::WINDOWS);
    log_data["old_base_hp"]    = static_cast<Json::Int64>(old_base_hp);
    log_data["new_base_hp"]    = static_cast<Json::Int64>(value);
    this->owner.server.log.write("base_hp_change", log_data);
}

void character_stat::base_mp(uint64_t value, bool notify)
{
    this->owner.assert_thread();

    if (this->_max_mp == value)
        return;

    auto old_base_mp = this->_max_mp;
    this->_max_mp    = value;
    this->mp(this->mp(), false);
    if (notify)
        this->owner.update(UPDATE_STATE_LEVEL::BASED | UPDATE_STATE_LEVEL::HP_MP);

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(this->owner.id);
    log_data["character_name"] = UTF8(this->owner.name(), PLATFORM::WINDOWS);
    log_data["old_base_mp"]    = static_cast<Json::Int64>(old_base_mp);
    log_data["new_base_mp"]    = static_cast<Json::Int64>(value);
    this->owner.server.log.write("base_mp_change", log_data);
}

void character_stat::base_str(uint8_t value, bool notify)
{
    this->owner.assert_thread();
    this->_str = value;
    if (notify)
        this->owner.update(UPDATE_STATE_LEVEL::BASED);
}

void character_stat::base_dex(uint8_t value, bool notify)
{
    this->owner.assert_thread();
    this->_dex = value;
    if (notify)
        this->owner.update(UPDATE_STATE_LEVEL::BASED);
}

void character_stat::base_int(uint8_t value, bool notify)
{
    this->owner.assert_thread();
    this->_int = value;
    if (notify)
        this->owner.update(UPDATE_STATE_LEVEL::BASED);
}

void character_stat::base_phydef(int8_t value, bool notify)
{
    this->owner.assert_thread();
    this->_phydef = value;
    if (notify)
        this->owner.update(UPDATE_STATE_LEVEL::BASED);
}

void character_stat::base_magdef(int8_t value, bool notify)
{
    this->owner.assert_thread();
    this->_magdef = value;
}

void character_stat::base_dam(uint8_t value, bool notify)
{
    this->owner.assert_thread();
    this->_dam = value;
}

void character_stat::base_hit(uint8_t value, bool notify)
{
    this->owner.assert_thread();
    this->_hit = value;
}

void character_stat::base_regenerative(uint64_t value, bool notify)
{
    this->owner.assert_thread();
    this->_regenerative = value;
}

uint64_t character_stat::base_hp() const
{
    this->owner.assert_thread();
    return this->_max_hp;
}

uint64_t character_stat::base_mp() const
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
    auto additional = (uint64_t)0;
    for (auto& [_, equipment] : this->owner.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.strength;
    }

    auto limit = std::numeric_limits<uint64_t>::max();
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
    auto additional = (uint64_t)0;
    for (auto& [_, equipment] : this->owner.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.dexterity;
    }

    auto limit = std::numeric_limits<uint64_t>::max();
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
    auto additional   = (uint64_t)0;
    for (auto& [_, equipment] : this->owner.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.intelligence;
    }

    auto limit = std::numeric_limits<uint64_t>::max();
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
    auto additional = (uint64_t)0;
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
    auto additional = (uint64_t)0;
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

int8_t character_stat::dam() const
{
    this->owner.assert_thread();
    auto    dam_val    = fb::game::stat::dam();
    int16_t additional = 0;
    for (auto& [_, equipment] : this->owner.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += static_cast<int16_t>(static_cast<int8_t>(model.damage));
    }
    int16_t sum = static_cast<int16_t>(dam_val) + additional;
    return static_cast<int8_t>(std::max<int16_t>(-128, std::min<int16_t>(127, sum)));
}

uint8_t character_stat::base_dam() const
{
    this->owner.assert_thread();
    return static_cast<uint8_t>(this->_dam);
}

uint8_t character_stat::base_hit() const
{
    this->owner.assert_thread();
    return this->_hit;
}

int8_t character_stat::hit() const
{
    this->owner.assert_thread();
    auto    hit_val    = fb::game::stat::hit();
    int16_t additional = 0;
    for (auto& [_, equipment] : this->owner.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += static_cast<int16_t>(static_cast<int8_t>(model.hit));
    }
    int16_t sum = static_cast<int16_t>(hit_val) + additional;
    return static_cast<int8_t>(std::max<int16_t>(-128, std::min<int16_t>(127, sum)));
}

uint64_t character_stat::base_regenerative() const
{
    this->owner.assert_thread();
    return this->_regenerative;
}

uint64_t character_stat::maxhp() const
{
    this->owner.assert_thread();
    constexpr auto max_signed = std::numeric_limits<int64_t>::max();
    int64_t base_flat =
        static_cast<int64_t>(std::min(this->base_hp(), static_cast<uint64_t>(max_signed))) + this->buff_hp();
    float hp_pct = 0.0f;

    for (auto& [_, equipment] : this->owner.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model = equipment->based<fb::model::equipment>();
        if (model.base_hp > 0 && base_flat > max_signed - model.base_hp)
            base_flat = max_signed;
        else if (model.base_hp < 0 && base_flat < std::numeric_limits<int64_t>::min() - model.base_hp)
            base_flat = std::numeric_limits<int64_t>::min();
        else
            base_flat += model.base_hp;
        hp_pct += model.hp_percentage;
    }

    if (base_flat < 1)
        return 1;

    auto pct_bonus = static_cast<int64_t>(static_cast<double>(base_flat) * hp_pct / 100.0);
    if (pct_bonus > 0 && base_flat > max_signed - pct_bonus)
        return static_cast<uint64_t>(max_signed);

    auto total = base_flat + pct_bonus;
    if (total < 1)
        return 1;
    return static_cast<uint64_t>(total);
}

uint64_t character_stat::maxmp() const
{
    this->owner.assert_thread();
    constexpr auto max_signed = std::numeric_limits<int64_t>::max();
    int64_t        base_flat =
        static_cast<int64_t>(std::min(this->base_mp(), static_cast<uint64_t>(max_signed))) + this->buff_mp();
    float mp_pct = 0.0f;

    for (auto& [_, equipment] : this->owner.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model = equipment->based<fb::model::equipment>();
        if (model.base_mp > 0 && base_flat > max_signed - model.base_mp)
            base_flat = max_signed;
        else if (model.base_mp < 0 && base_flat < std::numeric_limits<int64_t>::min() - model.base_mp)
            base_flat = std::numeric_limits<int64_t>::min();
        else
            base_flat += model.base_mp;
        mp_pct += model.mp_percentage;
    }

    if (base_flat < 1)
        return 1;

    auto pct_bonus = static_cast<int64_t>(static_cast<double>(base_flat) * mp_pct / 100.0);
    if (pct_bonus > 0 && base_flat > max_signed - pct_bonus)
        return static_cast<uint64_t>(max_signed);

    auto total = base_flat + pct_bonus;
    if (total < 1)
        return 1;
    return static_cast<uint64_t>(total);
}

uint64_t character_stat::regenerative() const
{
    this->owner.assert_thread();
    auto base       = fb::game::stat::regenerative();
    auto additional = uint64_t(0);

    for (auto& [_, equipment] : this->owner.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto& model  = equipment->based<fb::model::equipment>();
        additional  += model.healing_cycle;
    }

    auto max_val = std::numeric_limits<uint64_t>::max();
    if (max_val - base < additional)
        return max_val;

    return base + additional;
}

uint64_t character_stat::damage(uint64_t                          value,
                                std::shared_ptr<fb::game::object> from,
                                bool                              critical,
                                float                             rate,
                                bool                              physical,
                                bool                              fixed,
                                bool                              notify)
{
    this->owner.assert_thread();

    if (this->owner.alive() == false)
        return 0;

    auto result = fb::game::stat::damage(value, from, critical, rate, physical, fixed, notify);
    if (from == nullptr)
        return result;

    for (const auto& mob : this->owner.spawned_mobs())
    {
        if (mob->target() != nullptr)
            continue;

        if (from->is(OBJECT_TYPE::LIFE) == false)
            continue;

        mob->target(std::static_pointer_cast<fb::game::life>(from));
    }

    if (this->hp() == 0)
        return result;

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

uint64_t mob_stat::base_hp() const
{
    this->owner.assert_thread();
    auto& model = this->owner.based<fb::model::mob>();
    return model.hp;
}

uint64_t mob_stat::base_mp() const
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

uint64_t mob_stat::base_regenerative() const
{
    this->owner.assert_thread();
    return 0;
}

uint64_t mob_stat::damage(uint64_t                value,
                          std::shared_ptr<object> from,
                          bool                    critical,
                          float                   rate,
                          bool                    physical,
                          bool                    fixed,
                          bool                    notify)
{
    this->owner.assert_thread();

    if (this->owner._forwarding_damage)
    {
        auto result = fb::game::stat::damage(value, from, critical, rate, physical, fixed, notify);
        if (this->owner.alive() && this->owner._ai_strategy && from && from->is(OBJECT_TYPE::LIFE))
        {
            this->owner._ai_strategy->on_damage(this->owner,
                                                std::static_pointer_cast<fb::game::life>(from),
                                                this->owner.server.now());
        }
        return result;
    }

    // Assembly body ignores direct damage; only parts forward damage.
    if (this->owner.has_parts())
        return 0;

    if (this->owner.body() != nullptr)
        return this->owner.damage_as_part(value, from, critical, rate, physical, fixed, notify);

    auto result = fb::game::stat::damage(value, from, critical, rate, physical, fixed, notify);
    if (!this->owner.alive())
        return result;

    // Handle damage in AI strategy
    if (this->owner._ai_strategy && from && from->is(OBJECT_TYPE::LIFE))
    {
        this->owner._ai_strategy->on_damage(this->owner,
                                            std::static_pointer_cast<fb::game::life>(from),
                                            this->owner.server.now());
    }

    return result;
}

uint64_t mob_stat::hp() const
{
    return fb::game::stat::hp();
}

void mob_stat::hp(uint64_t value, bool notify)
{
    this->owner.assert_thread();

    auto before = fb::game::stat::hp();
    fb::game::stat::hp(value, notify);

    if (this->owner._forwarding_damage)
        return;

    auto after = fb::game::stat::hp();
    if (after > before)
        this->owner.on_part_hp_increased(after - before);
}

uint64_t mob_stat::heal(uint64_t value, fb::game::object* from, bool notify)
{
    this->owner.assert_thread();
    // hp() override syncs PARTS-mode body HP when a part is healed
    return fb::game::stat::heal(value, from, notify);
}
