#include <fb/game/context.h>
#include <fb/game/life.h>
#include <fb/game/map.h>

fb::game::life::life(fb::game::context& context, const fb::model::life& model, const initial_params& params) :
    object(context, model, params),
    _hp(params.hp),
    _mp(params.mp),
    spells(*this)
{ }

fb::game::life::~life()
{ }

uint32_t fb::game::life::heal(uint32_t value, fb::game::object* from)
{
    this->assert_thread();

    auto before = this->_hp;
    this->hp(this->_hp + std::min(value, this->base_hp() - this->_hp));

    auto listener = this->get_listener<fb::game::life>();
    if (listener != nullptr)
        listener->on_hp_changed(*this, before, this->_hp, false, from);

    return this->_hp - before;
}

uint32_t fb::game::life::damage(uint32_t value, fb::game::object* from, bool critical)
{
    this->assert_thread();

    auto before = this->_hp;
    this->hp(this->_hp - std::min(value, this->_hp));

    auto listener = this->get_listener<fb::game::life>();
    if (listener != nullptr)
        listener->on_hp_changed(*this, before, this->_hp, critical, from);

    if (this->_hp == 0)
        this->kill(from, DESTROY_TYPE::DEAD);

    return before - this->_hp;
}

uint32_t fb::game::life::mp_up(uint32_t value, fb::game::object* from)
{
    this->assert_thread();

    auto before = this->_mp;
    this->mp(this->_mp + std::min(value, this->base_mp() - this->_mp));

    auto listener = this->get_listener<fb::game::life>();
    if (listener != nullptr)
        listener->on_mp_changed(*this, before, this->_mp, false, from);

    return this->_mp - before;
}

uint32_t fb::game::life::mp_down(uint32_t value, fb::game::object* from)
{
    this->assert_thread();

    auto before = this->_mp;
    this->mp(this->_mp - std::min(value, this->_mp));

    auto listener = this->get_listener<fb::game::life>();
    if (listener != nullptr)
        listener->on_mp_changed(*this, before, this->_mp, false, from);

    return before - this->_mp;
}

void fb::game::life::kill(fb::game::object* from, DESTROY_TYPE destroy_type)
{
    this->_hp     = 0;
    auto listener = this->get_listener<fb::game::life>();
    if (listener != nullptr)
        listener->on_dead(*this, from);
}

void fb::game::life::attack()
{
    this->assert_thread();

    if (this->_map == nullptr)
        return;

    if (this->alive() == false)
        return;

    auto listener = this->get_listener<fb::game::life>();
    if (listener != nullptr)
        listener->on_attack(*this);
}

uint32_t fb::game::life::hp() const
{
    this->assert_thread();

    return this->_hp;
}

void fb::game::life::hp(uint32_t value)
{
    this->assert_thread();

    auto before = this->_hp;
    this->_hp   = value;

    auto listener = this->get_listener<fb::game::life>();
    if (listener != nullptr)
        listener->on_hp_changed(*this, before, this->_hp, false, nullptr);
}

uint32_t fb::game::life::mp() const
{
    this->assert_thread();

    return this->_mp;
}

void fb::game::life::mp(uint32_t value)
{
    this->assert_thread();

    auto before = this->_mp;
    this->_mp   = value;

    auto listener = this->get_listener<fb::game::life>();
    if (listener != nullptr)
        listener->on_mp_changed(*this, before, this->_mp, false, nullptr);
}

uint32_t fb::game::life::base_hp() const
{
    this->assert_thread();

    return static_cast<const fb::model::life&>(this->_model).hp;
}

uint32_t fb::game::life::base_mp() const
{
    this->assert_thread();

    return static_cast<const fb::model::life&>(this->_model).mp;
}

uint32_t fb::game::life::exp() const
{
    this->assert_thread();

    return static_cast<const fb::model::life&>(this->_model).exp;
}

uint32_t fb::game::life::defensive_physical() const
{
    this->assert_thread();

    return static_cast<const fb::model::life&>(this->_model).defensive_physical;
}

uint32_t fb::game::life::defensive_magical() const
{
    this->assert_thread();

    return static_cast<const fb::model::life&>(this->_model).defensive_magical;
}

CONDITION fb::game::life::condition() const
{
    this->assert_thread();

    return this->_condition;
}

CONDITION fb::game::life::condition_add(CONDITION value)
{
    this->assert_thread();

    this->_condition = CONDITION(this->_condition | value);
    return this->_condition;
}

CONDITION fb::game::life::condition_remove(CONDITION value)
{
    this->assert_thread();

    this->_condition = CONDITION(this->_condition & ~value);
    return this->_condition;
}

bool fb::game::life::condition_contains(CONDITION value) const
{
    this->assert_thread();

    return uint32_t(this->_condition) & uint32_t(value);
}

bool fb::game::life::alive() const
{
    this->assert_thread();

    return this->_hp != 0;
}

bool fb::game::life::active(const fb::model::spell& spell, const std::string& message)
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

bool fb::game::life::active(const fb::model::spell& spell, uint32_t fd)
{
    this->assert_thread();

    if (this->_map == nullptr)
        return false;

    auto to = this->_map->objects[fd];
    if (to == nullptr)
        return false;

    return this->active(spell, *to);
}

bool fb::game::life::active(const fb::model::spell& spell, fb::game::object& to)
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

bool fb::game::life::active(const fb::model::spell& spell)
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

bool fb::game::life::calculate_critical(fb::game::life& you) const
{
    this->assert_thread();

#if defined DEBUG | defined _DEBUG
    return true;
#else
    return std::rand() % 100 < 20;
#endif
}

bool fb::game::life::calculate_miss(fb::game::life& you) const
{
    this->assert_thread();

#if defined DEBUG | defined _DEBUG
    return false;
#else
    return std::rand() % 3 == 0;
#endif
}

uint32_t fb::game::life::calculate_damage(uint32_t value, const fb::game::life& life, bool critical) const
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