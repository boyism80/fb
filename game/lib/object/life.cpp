#include <fb/game/context.h>
#include <fb/game/life.h>
#include <fb/game/map.h>

using namespace fb::game;

life::life(fb::game::context& context, const fb::model::life& model, const initial_params& params) :
    object(context, model, params),
    _hp(params.hp),
    _mp(params.mp),
    spells(*this)
{ }

life::~life()
{ }

uint32_t life::heal(uint32_t value, fb::game::object* from)
{
    this->assert_thread();
    auto before = this->_hp;
    this->hp(this->_hp + std::min(value, this->maxhp() - this->_hp));
    this->update_hp(this->_hp - before, false);
    return this->_hp - before;
}

uint32_t life::damage(uint32_t value, fb::game::object* from, bool critical)
{
    this->assert_thread();
    if (from != nullptr && from->is(OBJECT_TYPE::CHARACTER))
    {
        auto ch = static_cast<fb::game::character*>(from);
        for (auto mob : ch->spawned_mobs())
        {
            if (mob == this)
                continue;

            mob->target(this);
        }
    }

    if (this->invincible())
        return 0;

    auto before = this->_hp;
    this->hp(this->_hp - std::min(value, this->_hp));
    this->update_hp(before - this->_hp, critical);
    return before - this->_hp;
}

uint32_t life::mp_up(uint32_t value, fb::game::object* from)
{
    this->assert_thread();
    auto before = this->_mp;
    this->mp(this->_mp + std::min(value, this->maxmp() - this->_mp));
    return this->_mp - before;
}

uint32_t life::mp_down(uint32_t value, fb::game::object* from)
{
    this->assert_thread();
    auto before = this->_mp;
    this->mp(this->_mp - std::min(value, this->_mp));
    return before - this->_mp;
}

void life::update(STATE_LEVEL value)
{ }

void life::update_hp(uint32_t diff, bool critical)
{
    auto listener = this->get_listener<life>();
    if (listener != nullptr)
        listener->on_update_hp(*this, diff, critical);
}

void life::kill(fb::game::object* from, DESTROY_TYPE destroy_type)
{
    this->_hp     = 0;
    auto listener = this->get_listener<life>();
    if (listener != nullptr)
        listener->on_dead(*this, from);
}

void life::attack(DURATION duration)
{
    this->assert_thread();
    if (this->_map == nullptr)
        return;

    if (this->alive() == false)
        return;

    auto listener = this->get_listener<life>();
    if (listener != nullptr)
        listener->on_attack(*this, duration);
}

uint32_t life::hp() const
{
    this->assert_thread();
    return this->_hp;
}

void life::hp(uint32_t value)
{
    this->assert_thread();
    auto before = this->_hp;
    this->_hp   = value;
    this->update(STATE_LEVEL::HP_MP);
}

uint32_t life::mp() const
{
    this->assert_thread();
    return this->_mp;
}

void life::mp(uint32_t value)
{
    this->assert_thread();
    auto before = this->_mp;
    this->_mp   = value;
    this->update(STATE_LEVEL::HP_MP);
}

uint32_t life::base_hp() const
{
    this->assert_thread();
    return static_cast<const fb::model::life&>(this->_model).hp;
}

uint32_t life::base_mp() const
{
    this->assert_thread();
    return static_cast<const fb::model::life&>(this->_model).mp;
}

uint32_t life::exp() const
{
    this->assert_thread();
    return static_cast<const fb::model::life&>(this->_model).exp;
}

CROWD_CONTROL life::crowd_control() const
{
    this->assert_thread();
    return this->_crowd_control;
}

void life::crowd_control(CROWD_CONTROL value)
{
    this->_crowd_control = value;
    this->update(STATE_LEVEL::CROWD_CONTROL);
}

CROWD_CONTROL life::add_cc(CROWD_CONTROL value)
{
    this->assert_thread();
    this->crowd_control(CROWD_CONTROL(this->_crowd_control | value));
    return this->_crowd_control;
}

CROWD_CONTROL life::remove_cc(CROWD_CONTROL value)
{
    this->assert_thread();
    this->crowd_control(CROWD_CONTROL(this->_crowd_control & ~value));
    return this->_crowd_control;
}

bool life::condition_contains(CROWD_CONTROL value) const
{
    this->assert_thread();
    return uint32_t(this->_crowd_control) & uint32_t(value);
}

bool life::alive() const
{
    this->assert_thread();
    return this->_hp != 0;
}

bool life::active(fb::game::spell& spell, const std::string& message)
{
    this->assert_thread();
    auto lua = fb::lua::new_context();
    if (lua == nullptr)
        return false;

#if defined DEBUG | defined _DEBUG
    lua->load("scripts/spell.lua");
#endif
    lua->load(spell.model.cast.c_str());
    lua->func("on_cast");
    if (spell.model.type != SPELL_TYPE::INPUT)
        return false;

    lua->pushobject(this);
    lua->pushobject(spell.model);
    lua->pushstring(message);
    lua->resume(3);
    return true;
}

bool life::active(fb::game::spell& spell, uint32_t fd)
{
    this->assert_thread();
    if (this->_map == nullptr)
        return false;

    auto to = this->_map->objects[fd];
    if (to == nullptr)
        return false;

    return this->active(spell, *to);
}

bool life::active(fb::game::spell& spell, fb::game::object& to)
{
    this->assert_thread();
    auto lua = fb::lua::new_context();
    if (lua == nullptr)
        return false;

#if defined DEBUG | defined _DEBUG
    lua->load("scripts/spell.lua");
#endif
    lua->load(spell.model.cast.c_str());
    lua->func("on_cast");
    if (spell.model.type != SPELL_TYPE::TARGET)
        return false;

    auto map = this->map();
    if (map == nullptr)
        return false;

    if (to.map() != this->map())
        return false;

    if (this->sight(to) == false)
        return true;

    lua->pushobject(this);
    lua->pushobject(&to);
    lua->pushobject(spell.model);
    lua->resume(3);
    return true;
}

bool life::active(fb::game::spell& spell)
{
    this->assert_thread();
    auto lua = fb::lua::new_context();
    if (lua == nullptr)
        return false;

#if defined DEBUG | defined _DEBUG
    lua->load("scripts/spell.lua");
#endif
    lua->load(spell.model.cast.c_str());
    lua->func("on_cast");
    if (spell.model.type != SPELL_TYPE::NORMAL)
        return false;

    lua->pushobject(this);
    lua->pushobject(spell.model);
    lua->resume(2);
    return true;
}

void life::action(ACTION action, DURATION duration, uint8_t sound)
{
    auto listener = this->get_listener<life>();
    if (listener != nullptr)
        listener->on_action(*this, action, duration, sound);
}

bool life::calculate_critical(life& you) const
{
    this->assert_thread();

#if defined DEBUG | defined _DEBUG
    return true;
#else
    return std::rand() % 100 < 20;
#endif
}

bool life::calculate_miss(life& you) const
{
    this->assert_thread();

#if defined DEBUG | defined _DEBUG
    return false;
#else
    return std::rand() % 3 == 0;
#endif
}

uint32_t life::calculate_damage(uint32_t value, const life& life, bool critical) const
{
    this->assert_thread();
    auto n                 = (100 - life.phydef()) / 10;
    auto defensive_percent = -125 + (n * (2 * 14.75f - (n - 1) / 2.0f)) / 2.0f;
    auto damage            = value - uint32_t(defensive_percent * (value / 100.0f));

    auto rate = this->damage_rate() / 1000.0f;
    if (life.direction() == this->direction())
        rate *= 2;

    if (critical)
        rate *= 2;

    rate /= (life.damage_derate() / 1000.0f);
    return static_cast<uint32_t>(damage * rate);
}

uint32_t life::damage_rate() const
{
    return this->_damage_rate;
}

void life::damage_rate(uint32_t value)
{
    this->_damage_rate = value;
}

uint32_t life::skill_damage_rate() const
{
    return this->_skill_damage_rate;
}

void life::skill_damage_rate(uint32_t value)
{
    this->_skill_damage_rate = value;
}

uint32_t life::damage_derate() const
{
    return this->_damage_derate;
}

void life::damage_derate(uint32_t value)
{
    this->_damage_derate = value;
}

void life::paralysis(bool value)
{
    this->assert_thread();
    this->_paralysis = value;
}

bool life::paralysis() const
{
    return this->_paralysis;
}

void life::invincible(bool value)
{
    this->assert_thread();
    this->_invincible = value;
}

bool life::invincible() const
{
    return this->_invincible;
}

void life::cover(bool value)
{
    this->assert_thread();
    this->_cover = value;
}

bool life::cover() const
{
    return this->_cover;
}

uint32_t life::maxhp() const
{
    auto base = this->base_hp();
    auto buff = this->buff_hp();
    auto max  = std::numeric_limits<uint32_t>::max();
    if (max - base < buff)
        return max;

    return base + buff;
}

uint32_t life::maxmp() const
{
    auto base = this->base_mp();
    auto buff = this->buff_mp();
    auto max  = std::numeric_limits<uint32_t>::max();
    if (max - base < buff)
        return max;

    return base + buff;
}

uint8_t life::str() const
{
    auto base = this->base_str();
    auto buff = this->buff_str();
    auto max  = std::numeric_limits<uint8_t>::max();
    if (max - base < buff)
        return max;

    return base + buff;
}

uint8_t life::dex() const
{
    auto base = this->base_dex();
    auto buff = this->buff_dex();
    auto max  = std::numeric_limits<uint8_t>::max();
    if (max - base < buff)
        return max;

    return base + buff;
}

uint8_t life::intelligence() const
{
    auto base = this->base_int();
    auto buff = this->buff_int();
    auto max  = std::numeric_limits<uint8_t>::max();
    if (max - base < buff)
        return max;

    return base + buff;
}

int8_t life::phydef() const
{
    auto base = this->base_phydef();
    auto buff = this->buff_phydef();
    auto sum  = (int16_t)base + (int16_t)buff;
    return (int8_t)std::max<int16_t>(-127, std::min<int16_t>(128, sum));
}

int8_t life::magdef() const
{
    auto base = this->base_magdef();
    auto buff = this->buff_magdef();
    auto sum  = (int16_t)base + (int16_t)buff;
    return (int8_t)std::max<int16_t>(-127, std::min<int16_t>(128, sum));
}

uint8_t life::dam() const
{
    auto base = this->base_dam();
    auto buff = this->buff_dam();
    auto max  = std::numeric_limits<uint8_t>::max();
    if (max - base < buff)
        return max;

    return base + buff;
}

uint8_t life::hit() const
{
    auto base = this->base_hit();
    auto buff = this->buff_hit();
    auto max  = std::numeric_limits<uint8_t>::max();
    if (max - base < buff)
        return max;

    return base + buff;
}