#include <context.h>
#include <life.h>
#include <map.h>

fb::game::life::life(fb::game::context& context, const fb::model::life& model, const initial_params& params) :
    object(context, model, params),
    _hp(params.hp),
    _mp(params.mp),
    spells(*this)
{ }

fb::game::life::~life()
{ }

uint32_t fb::game::life::calculate_damage(uint32_t value, const fb::game::life& life) const
{
    this->assert_thread();

    auto rate              = life.direction() == this->direction() ? 2 : 1;
    auto n                 = (100 - life.defensive_physical()) / 10;
    auto defensive_percent = -125 + (n * (2 * 14.75f - (n - 1) / 2.0f)) / 2.0f;
    auto damage            = value - uint32_t(defensive_percent * (value / 100.0f));

    return damage * rate;
}

uint32_t fb::game::life::hp_up(uint32_t value, fb::game::object* from)
{
    this->assert_thread();

    auto listener = this->get_listener<fb::game::life>();

    value = std::min(value, this->base_hp() - this->_hp);
    this->hp(this->_hp + value);
    if (listener != nullptr)
        listener->on_heal_hp(*this, value, from);

    return value;
}

uint32_t fb::game::life::hp_down(uint32_t value, fb::game::object* from, bool critical)
{
    this->assert_thread();

    auto listener = this->get_listener<fb::game::life>();

    value = std::min(value, this->_hp);
    this->hp(this->_hp - value);

    this->on_damaged(from, value, critical);
    if (this->_hp == 0)
    {
        if (from != nullptr)
            from->on_kill(*this);
        this->on_die(from);
    }

    return value;
}

uint32_t fb::game::life::mp_up(uint32_t value, fb::game::object* from)
{
    this->assert_thread();

    auto listener = this->get_listener<fb::game::life>();

    value = std::min(value, this->base_mp() - this->_mp);
    this->mp(this->_mp + value);
    if (listener != nullptr)
        listener->on_heal_mp(*this, value, from);

    return value;
}

uint32_t fb::game::life::mp_down(uint32_t value, fb::game::object* from)
{
    this->assert_thread();

    value = std::min(value, this->_mp);
    this->mp(this->_mp - value);

    return value;
}

void fb::game::life::attack()
{
    this->assert_thread();

    if (this->_map == nullptr)
        return;

    if (this->alive() == false)
        return;

    auto front = this->forward(OBJECT_TYPE::LIFE);
    this->on_attack(front);

    if (front == nullptr)
        return;

    auto you = static_cast<fb::game::life*>(front);
    if (you == nullptr)
        return;

    auto miss = this->on_calculate_miss(*you);
    if (miss)
        return;

    auto critical = this->on_calculate_critical(*you);
    auto damage   = this->on_calculate_damage(critical);

    this->on_hit(*you, damage, critical);
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

    this->on_update();

    auto listener = this->get_listener<fb::game::life>();
    if (listener != nullptr)
        listener->on_hp(*this, before, this->_hp);
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

    this->on_update();

    auto listener = this->get_listener<fb::game::life>();
    if (listener != nullptr)
        listener->on_mp(*this, before, this->_hp);
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

void fb::game::life::kill()
{
    this->assert_thread();

    this->_hp = 0;

    auto listener = this->get_listener<fb::game::life>();
    if (listener != nullptr)
        listener->on_hide(*this, DESTROY_TYPE::DEAD);
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

void fb::game::life::on_update()
{
    this->assert_thread();

    return;
}

bool fb::game::life::on_calculate_critical(fb::game::life& you) const
{
    this->assert_thread();

#if defined DEBUG | defined _DEBUG
    return true;
#else
    return std::rand() % 100 < 20;
#endif
}

bool fb::game::life::on_calculate_miss(fb::game::life& you) const
{
    this->assert_thread();

#if defined DEBUG | defined _DEBUG
    return false;
#else
    return std::rand() % 3 == 0;
#endif
}

void fb::game::life::on_attack(fb::game::object* you)
{
    this->assert_thread();

    auto listener = this->get_listener<fb::game::life>();
    if (listener != nullptr)
        listener->on_attack(*this, you);
}

void fb::game::life::on_hit(fb::game::life& you, uint32_t damage, bool critical)
{
    this->assert_thread();

    auto listener = this->get_listener<fb::game::life>();
    if (listener != nullptr)
        listener->on_hit(*this, you, damage, critical);
}

void fb::game::life::on_damaged(fb::game::object* from, uint32_t damage, bool critical)
{
    this->assert_thread();

    auto listener = this->get_listener<fb::game::life>();
    if (listener != nullptr)
        listener->on_damaged(*this, from, damage, critical);
}

void fb::game::life::on_die(fb::game::object* from)
{
    this->assert_thread();

    auto listener = this->get_listener<fb::game::life>();
    if (listener != nullptr)
        listener->on_die(*this, from);
}

uint32_t fb::game::life::on_exp() const
{
    this->assert_thread();

    return 0;
}

void fb::game::life::on_kill(fb::game::life& you)
{
    this->assert_thread();

    auto listener = this->get_listener<fb::game::life>();
    if (listener != nullptr)
        listener->on_kill(*this, you);
}