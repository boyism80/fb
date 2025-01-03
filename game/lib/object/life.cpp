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
    this->hp(this->_hp + std::min(value, this->base_hp() - this->_hp));
    this->update_hp(this->_hp - before, false);
    return this->_hp - before;
}

uint32_t life::damage(uint32_t value, fb::game::object* from, bool critical)
{
    this->assert_thread();

    auto before = this->_hp;
    this->hp(this->_hp - std::min(value, this->_hp));
    this->update_hp(before - this->_hp, critical);
    return before - this->_hp;
}

uint32_t life::mp_up(uint32_t value, fb::game::object* from)
{
    this->assert_thread();

    auto before = this->_mp;
    this->mp(this->_mp + std::min(value, this->base_mp() - this->_mp));
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

void life::attack()
{
    this->assert_thread();

    if (this->_map == nullptr)
        return;

    if (this->alive() == false)
        return;

    auto listener = this->get_listener<life>();
    if (listener != nullptr)
        listener->on_attack(*this);
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

uint32_t life::defensive_physical() const
{
    this->assert_thread();

    return static_cast<const fb::model::life&>(this->_model).defensive_physical;
}

uint32_t life::defensive_magical() const
{
    this->assert_thread();

    return static_cast<const fb::model::life&>(this->_model).defensive_magical;
}

CONDITION life::condition() const
{
    this->assert_thread();

    return this->_condition;
}

CONDITION life::condition_add(CONDITION value)
{
    this->assert_thread();

    this->_condition = CONDITION(this->_condition | value);
    return this->_condition;
}

CONDITION life::condition_remove(CONDITION value)
{
    this->assert_thread();

    this->_condition = CONDITION(this->_condition & ~value);
    return this->_condition;
}

bool life::condition_contains(CONDITION value) const
{
    this->assert_thread();

    return uint32_t(this->_condition) & uint32_t(value);
}

bool life::alive() const
{
    this->assert_thread();

    return this->_hp != 0;
}

bool life::active(const fb::model::spell& spell, const std::string& message)
{
    this->assert_thread();

    auto thread = fb::lua::get();
    if (thread == nullptr)
        return false;

    thread->from(spell.cast.c_str()).func("on_cast");

    if (spell.type != SPELL_TYPE::INPUT)
        return false;

    thread->pushobject(this).pushobject(spell).pushstring(message).resume(3);
    return true;
}

bool life::active(const fb::model::spell& spell, uint32_t fd)
{
    this->assert_thread();

    if (this->_map == nullptr)
        return false;

    auto to = this->_map->objects[fd];
    if (to == nullptr)
        return false;

    return this->active(spell, *to);
}

bool life::active(const fb::model::spell& spell, fb::game::object& to)
{
    this->assert_thread();

    auto thread = fb::lua::get();
    if (thread == nullptr)
        return false;

    thread->from(spell.cast.c_str()).func("on_cast");

    if (spell.type != SPELL_TYPE::TARGET)
        return false;

    auto map = this->map();
    if (map == nullptr)
        return false;

    if (to.map() != this->map())
        return false;

    if (this->sight(to) == false)
        return true;

    thread->pushobject(this).pushobject(&to).pushobject(spell).resume(3);
    return true;
}

bool life::active(const fb::model::spell& spell)
{
    this->assert_thread();

    auto thread = fb::lua::get();
    if (thread == nullptr)
        return false;

    thread->from(spell.cast.c_str()).func("on_cast");

    if (spell.type != SPELL_TYPE::NORMAL)
        return false;

    thread->pushobject(this).pushobject(spell).resume(2);
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

    auto rate              = life.direction() == this->direction() ? 2 : 1;
    auto n                 = (100 - life.defensive_physical()) / 10;
    auto defensive_percent = -125 + (n * (2 * 14.75f - (n - 1) / 2.0f)) / 2.0f;
    auto damage            = value - uint32_t(defensive_percent * (value / 100.0f));
    if (critical)
        rate *= 2;

    return damage * rate;
}