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

namespace {

uint8_t character_speed_cap(const character& ch)
{
    if (ch.client_version == fb::protocol::CLIENT_VERSION::v651 && ch.role() >= ROLE::ADMIN)
        return 10;
    return 5;
}

void notify_walk_speed(character& ch)
{
    if (ch.client_version == fb::protocol::CLIENT_VERSION::v651)
        ch.update_external();
    else
        ch.update(UPDATE_STATE_LEVEL::EXP_MONEY | UPDATE_STATE_LEVEL::CROWD_CONTROL);
}

uint64_t apply_percent_pool(int64_t base, int64_t buff, float percent)
{
    constexpr auto max_signed = std::numeric_limits<int64_t>::max();
    if (buff > 0 && base > max_signed - buff)
        return static_cast<uint64_t>(max_signed);

    auto sum = base + buff;
    if (sum < 1)
        return 1;

    if (percent == 0.0f)
        return static_cast<uint64_t>(sum);

    auto pct_bonus = static_cast<int64_t>(static_cast<double>(sum) * percent / 100.0);
    if (pct_bonus > 0 && sum > max_signed - pct_bonus)
        return static_cast<uint64_t>(max_signed);

    auto total = sum + pct_bonus;
    if (total < 1)
        return 1;
    return static_cast<uint64_t>(total);
}

} // namespace

fb::game::stat::stat(life& owner) :
    owner(owner)
{ }

fb::game::stat::stat(const stat& other) :
    owner(other.owner),
    _hp(other._hp),
    _mp(other._mp),
    _buff_hp(other._buff_hp),
    _buff_mp(other._buff_mp),
    _buff_hp_percent(other._buff_hp_percent),
    _buff_mp_percent(other._buff_mp_percent),
    _buff_str(other._buff_str),
    _buff_dex(other._buff_dex),
    _buff_int(other._buff_int),
    _buff_phydef(other._buff_phydef),
    _buff_magdef(other._buff_magdef),
    _buff_dam(other._buff_dam),
    _buff_hit(other._buff_hit),
    _buff_speed(other._buff_speed),
    _buff_regenerative(other._buff_regenerative),
    _buff_resist(other._buff_resist)
{ }

fb::game::stat::stat(stat&& other) :
    owner(other.owner),
    _hp(other._hp),
    _mp(other._mp),
    _buff_hp(other._buff_hp),
    _buff_mp(other._buff_mp),
    _buff_hp_percent(other._buff_hp_percent),
    _buff_mp_percent(other._buff_mp_percent),
    _buff_str(other._buff_str),
    _buff_dex(other._buff_dex),
    _buff_int(other._buff_int),
    _buff_phydef(other._buff_phydef),
    _buff_magdef(other._buff_magdef),
    _buff_dam(other._buff_dam),
    _buff_hit(other._buff_hit),
    _buff_speed(other._buff_speed),
    _buff_regenerative(other._buff_regenerative),
    _buff_resist(std::move(other._buff_resist))
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

float fb::game::stat::buff_hp_percent() const
{
    this->owner.assert_thread();
    return this->_buff_hp_percent;
}

void fb::game::stat::buff_hp_percent(float value)
{
    this->owner.assert_thread();
    this->_buff_hp_percent = value;
}

float fb::game::stat::buff_mp_percent() const
{
    this->owner.assert_thread();
    return this->_buff_mp_percent;
}

void fb::game::stat::buff_mp_percent(float value)
{
    this->owner.assert_thread();
    this->_buff_mp_percent = value;
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

int8_t fb::game::stat::buff_speed() const
{
    this->owner.assert_thread();
    return this->_buff_speed;
}

void fb::game::stat::buff_speed(int8_t value)
{
    this->owner.assert_thread();
    this->_buff_speed = value;
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

float fb::game::stat::base_resist(RESIST type) const
{
    this->owner.assert_thread();
    return 0.0f;
}

float fb::game::stat::buff_resist(RESIST type) const
{
    this->owner.assert_thread();
    auto i = this->_buff_resist.find(type);
    if (i == this->_buff_resist.end())
        return 0.0f;
    return i->second;
}

void fb::game::stat::buff_resist(RESIST type, float value)
{
    this->owner.assert_thread();
    this->_buff_resist[type] = value;
}

float fb::game::stat::resist(RESIST type) const
{
    this->owner.assert_thread();
    auto value = this->base_resist(type) + this->buff_resist(type);
    if (value < 0.0f)
        return 0.0f;
    if (value > 1.0f)
        return 1.0f;
    return value;
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
    auto before  = this->hp();
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
    auto base =
        static_cast<int64_t>(std::min(this->base_hp(), static_cast<uint64_t>(std::numeric_limits<int64_t>::max())));
    return apply_percent_pool(base, this->buff_hp(), this->buff_hp_percent());
}

uint64_t fb::game::stat::maxmp() const
{
    this->owner.assert_thread();
    auto base =
        static_cast<int64_t>(std::min(this->base_mp(), static_cast<uint64_t>(std::numeric_limits<int64_t>::max())));
    return apply_percent_pool(base, this->buff_mp(), this->buff_mp_percent());
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

uint8_t fb::game::stat::base_speed() const
{
    this->owner.assert_thread();
    return 0;
}

uint8_t fb::game::stat::speed() const
{
    this->owner.assert_thread();
    auto sum = static_cast<int>(this->base_speed()) + static_cast<int>(this->buff_speed());
    if (sum < 0)
        return 0;
    if (sum > 5)
        return 5;
    return static_cast<uint8_t>(sum);
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

void character_stat::base_speed(uint8_t value, bool notify)
{
    this->owner.assert_thread();

    auto cap = character_speed_cap(this->owner);
    if (value > cap)
        value = cap;

    if (this->_speed == value)
        return;

    this->_speed = value;
    if (notify)
        notify_walk_speed(this->owner);
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

uint8_t character_stat::base_dex() const
{
    this->owner.assert_thread();
    return this->_dex;
}

uint8_t character_stat::base_int() const
{
    this->owner.assert_thread();
    return this->_int;
}

int8_t character_stat::base_phydef() const
{
    this->owner.assert_thread();
    return this->_phydef;
}

int8_t character_stat::base_magdef() const
{
    this->owner.assert_thread();
    return this->_magdef;
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

uint8_t character_stat::base_speed() const
{
    this->owner.assert_thread();
    return this->_speed;
}

uint8_t character_stat::speed() const
{
    this->owner.assert_thread();
    auto sum = static_cast<int>(this->base_speed()) + static_cast<int>(fb::game::stat::buff_speed());
    auto cap = character_speed_cap(this->owner);
    if (sum < 0)
        return 0;
    if (sum > cap)
        return static_cast<uint8_t>(cap);
    return static_cast<uint8_t>(sum);
}

void character_stat::buff_speed(int8_t value)
{
    this->owner.assert_thread();

    auto before = this->speed();
    fb::game::stat::buff_speed(value);
    if (before != this->speed())
        notify_walk_speed(this->owner);
}

uint64_t character_stat::base_regenerative() const
{
    this->owner.assert_thread();
    return this->_regenerative;
}

void character_stat::equipment_on(const fb::model::equipment& model)
{
    this->owner.assert_thread();

    this->buff_hp(this->buff_hp() + model.base_hp);
    this->buff_mp(this->buff_mp() + model.base_mp);
    this->buff_hp_percent(this->buff_hp_percent() + model.hp_percentage);
    this->buff_mp_percent(this->buff_mp_percent() + model.mp_percentage);
    this->buff_str(this->buff_str() + model.strength);
    this->buff_dex(this->buff_dex() + model.dexterity);
    this->buff_int(this->buff_int() + model.intelligence);
    this->buff_phydef(this->buff_phydef() + model.defensive_physical);
    this->buff_magdef(this->buff_magdef() + model.defensive_magical);
    this->buff_dam(this->buff_dam() + model.damage);
    this->buff_hit(this->buff_hit() + model.hit);
    this->buff_regenerative(this->buff_regenerative() + static_cast<int64_t>(model.healing_cycle));
}

void character_stat::equipment_off(const fb::model::equipment& model)
{
    this->owner.assert_thread();

    this->buff_hp(this->buff_hp() - model.base_hp);
    this->buff_mp(this->buff_mp() - model.base_mp);
    this->buff_hp_percent(this->buff_hp_percent() - model.hp_percentage);
    this->buff_mp_percent(this->buff_mp_percent() - model.mp_percentage);
    this->buff_str(this->buff_str() - model.strength);
    this->buff_dex(this->buff_dex() - model.dexterity);
    this->buff_int(this->buff_int() - model.intelligence);
    this->buff_phydef(this->buff_phydef() - model.defensive_physical);
    this->buff_magdef(this->buff_magdef() - model.defensive_magical);
    this->buff_dam(this->buff_dam() - model.damage);
    this->buff_hit(this->buff_hit() - model.hit);
    this->buff_regenerative(this->buff_regenerative() - static_cast<int64_t>(model.healing_cycle));
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

        auto& model = equipment->model();
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
    auto& model = this->owner.model();
    return model.hp;
}

uint64_t mob_stat::base_mp() const
{
    this->owner.assert_thread();
    auto& model = this->owner.model();
    return model.mp;
}

uint8_t mob_stat::base_str() const
{
    this->owner.assert_thread();
    // auto& model = this->owner.model();
    // return model.str;
    return 0;
}

uint8_t mob_stat::base_dex() const
{
    this->owner.assert_thread();
    // auto& model = this->owner.model();
    // return model.dex;
    return 0;
}

uint8_t mob_stat::base_int() const
{
    this->owner.assert_thread();
    // auto& model = this->owner.model();
    // return model.int;
    return 0;
}

int8_t mob_stat::base_phydef() const
{
    this->owner.assert_thread();
    auto& model = this->owner.model();
    return model.defensive_physical;
}

int8_t mob_stat::base_magdef() const
{
    this->owner.assert_thread();
    auto& model = this->owner.model();
    return model.defensive_magical;
}

uint8_t mob_stat::base_dam() const
{
    this->owner.assert_thread();
    // auto& model = this->owner.model();
    // return model.dam;
    return 0;
}

uint8_t mob_stat::base_hit() const
{
    this->owner.assert_thread();
    // auto& model = this->owner.model();
    // return model.hit;
    return 0;
}

uint64_t mob_stat::base_regenerative() const
{
    this->owner.assert_thread();
    return 0;
}

float mob_stat::base_resist(RESIST type) const
{
    this->owner.assert_thread();
    auto& model = this->owner.model();
    auto  i     = model.resist.find(type);
    if (i == model.resist.end())
        return 0.0f;
    return i->second;
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
