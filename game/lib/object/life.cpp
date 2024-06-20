#include <fb/game/context.h>
#include <fb/game/map.h>
#include <fb/game/life.h>

fb::game::life::model::model(const fb::game::life::model::config& config) : 
    object::model(config),
    defensive(config.defensive),
    hp(config.hp),
    mp(config.mp),
    experience(config.exp)
{ }

fb::game::life::model::~model()
{ }

fb::game::life::life(fb::game::context& context, const model* model, const fb::game::life::config& config) : 
    object(context, model, config),
    _hp(config.hp),
    _mp(config.mp),
    spells(*this)
{ }

fb::game::life::~life()
{ }

uint32_t fb::game::life::calculate_damage(uint32_t value, const fb::game::life& life) const
{
    auto rate = life.direction() == this->direction() ? 2 : 1;
    auto n = (100 - life.defensive_physical()) / 10;
    auto defensive_percent = -125 + (n * (2*14.75f - (n-1)/2.0f))/2.0f;
    auto damage = value - uint32_t(defensive_percent * (value/100.0f));

    return damage * rate;
}

uint32_t fb::game::life::hp_up(uint32_t value, fb::game::object* from)
{
    auto listener = this->get_listener<fb::game::life>();

    value = std::min(value, this->base_hp() - this->_hp);
    this->hp(this->_hp + value);
    if(listener != nullptr)
        listener->on_heal_hp(*this, value, from);

    return value;
}

uint32_t fb::game::life::hp_down(uint32_t value, fb::game::object* from, bool critical)
{
    auto listener = this->get_listener<fb::game::life>();

    value = std::min(value, this->_hp);
    this->hp(this->_hp - value);

    this->on_damaged(from, value, critical);
    if(this->_hp == 0)
    {
        if(from != nullptr)
            from->on_kill(*this);
        this->on_die(from);
    }

    return value;
}

uint32_t fb::game::life::mp_up(uint32_t value, fb::game::object* from)
{
    auto listener = this->get_listener<fb::game::life>();

    value = std::min(value, this->base_mp() - this->_mp);
    this->mp(this->_mp + value);
    if(listener != nullptr)
        listener->on_heal_mp(*this, value, from);

    return value;
}

uint32_t fb::game::life::mp_down(uint32_t value, fb::game::object* from)
{
    value = std::min(value, this->_mp);
    this->mp(this->_mp - value);

    return value;
}

void fb::game::life::attack()
{
    if(this->_map == nullptr)
        return;

    if(this->alive() == false)
        return;

    auto front = this->forward(OBJECT_TYPE::UNKNOWN);
    this->on_attack(front);

    if(front == nullptr || front->is(OBJECT_TYPE::LIFE) == false)
        return;

    auto you = static_cast<fb::game::life*>(front);
    if(you == nullptr)
        return;

    auto miss = this->on_calculate_miss(*you);
    if(miss)
        return;

    auto critical = this->on_calculate_critical(*you);
    auto damage = this->on_calculate_damage(critical);

    this->on_hit(*you, damage, critical);
}

uint32_t fb::game::life::hp() const
{
    return this->_hp;
}

void fb::game::life::hp(uint32_t value)
{
    auto before = this->_hp;
    this->_hp = value;

    this->on_update();

    auto listener = this->get_listener<fb::game::life>();
    if(listener != nullptr)
        listener->on_hp(*this, before, this->_hp);
}

uint32_t fb::game::life::mp() const
{
    return this->_mp;
}

void fb::game::life::mp(uint32_t value)
{
    auto before = this->_mp;
    this->_mp = value;

    this->on_update();

    auto listener = this->get_listener<fb::game::life>();
    if(listener != nullptr)
        listener->on_mp(*this, before, this->_hp);
}

uint32_t fb::game::life::base_hp() const
{
    return static_cast<const model*>(this->_model)->hp;
}

uint32_t fb::game::life::base_mp() const
{
    return static_cast<const model*>(this->_model)->mp;
}

uint32_t fb::game::life::experience() const
{
    return static_cast<const model*>(this->_model)->experience;
}

uint32_t fb::game::life::defensive_physical() const
{
    return static_cast<const model*>(this->_model)->defensive.physical;
}

uint32_t fb::game::life::defensive_magical() const
{
    return static_cast<const model*>(this->_model)->defensive.magical;
}

fb::game::CONDITION_TYPE fb::game::life::condition() const
{
    return this->_condition;
}

fb::game::CONDITION_TYPE fb::game::life::condition_add(fb::game::CONDITION_TYPE value)
{
    this->_condition = fb::game::CONDITION_TYPE(this->_condition | value);
    return this->_condition;
}

fb::game::CONDITION_TYPE fb::game::life::condition_remove(fb::game::CONDITION_TYPE value)
{
    this->_condition = fb::game::CONDITION_TYPE(this->_condition & ~value);
    return this->_condition;
}

bool fb::game::life::condition_contains(fb::game::CONDITION_TYPE value) const
{
    return uint32_t(this->_condition) & uint32_t(value);
}

bool fb::game::life::alive() const
{
    return this->_hp != 0;
}

void fb::game::life::kill()
{
    this->_hp = 0;

    auto listener = this->get_listener<fb::game::life>();
    if(listener != nullptr)
        listener->on_hide(*this);
}

bool fb::game::life::active(fb::model::spell& spell, const std::string& message)
{
    auto thread = fb::game::lua::get();
    if(thread == nullptr)
        return false;

    thread->from(spell.cast.c_str())
        .func("on_cast");

    if(spell.type != SPELL_TYPE::INPUT)
        return false;

    thread->pushobject(this)
        .pushobject(spell)
        .pushstring(message)
        .resume(3);
    return true;
}

bool fb::game::life::active(fb::model::spell& spell, uint32_t fd)
{
    if(this->_map == nullptr)
        return false;

    auto to = this->_map->objects[fd];
    if(to == nullptr)
        return false;

    return this->active(spell, *to);
}

bool fb::game::life::active(fb::model::spell& spell, fb::game::object& to)
{
    auto thread = fb::game::lua::get();
    if(thread == nullptr)
        return false;

    thread->from(spell.cast.c_str())
        .func("on_cast");

    if(spell.type != SPELL_TYPE::TARGET)
        return false;

    auto                map = this->map();
    if(map == nullptr)
        return false;

    if(to.map() != this->map())
        return false;

    if(this->sight(to) == false)
        return true;

    thread->pushobject(this)
        .pushobject(&to)
        .pushobject(spell)
        .resume(3);
    return true;
}

bool fb::game::life::active(fb::model::spell& spell)
{
    auto thread = fb::game::lua::get();
    if(thread == nullptr)
        return false;

    thread->from(spell.cast.c_str())
        .func("on_cast");

    if(spell.type != SPELL_TYPE::NORMAL)
        return false;

    thread->pushobject(this)
        .pushobject(spell)
        .resume(2);
    return true;
}

bool fb::game::life::on_calculate_critical(fb::game::life& you) const
{
#if defined DEBUG | defined _DEBUG
    return true;
#else
    return std::rand() % 100 < 20;
#endif
}

bool fb::game::life::on_calculate_miss(fb::game::life& you) const
{
#if defined DEBUG | defined _DEBUG
    return false;
#else
    return std::rand() % 3 == 0;
#endif
}

void fb::game::life::on_attack(fb::game::object* you)
{
    auto listener = this->get_listener<fb::game::life>();
    if(listener != nullptr)
        listener->on_attack(*this, you);
}

void fb::game::life::on_hit(fb::game::life& you, uint32_t damage, bool critical)
{
    auto listener = this->get_listener<fb::game::life>();
    if(listener != nullptr)
        listener->on_hit(*this, you, damage, critical);
}

void fb::game::life::on_damaged(fb::game::object* from, uint32_t damage, bool critical)
{
    auto listener = this->get_listener<fb::game::life>();
    if(listener != nullptr)
        listener->on_damaged(*this, from, damage, critical);
}

void fb::game::life::on_die(fb::game::object* from)
{
    auto listener = this->get_listener<fb::game::life>();
    if(listener != nullptr)
        listener->on_die(*this, from);
}

void fb::game::life::on_kill(fb::game::life& you)
{
    auto listener = this->get_listener<fb::game::life>();
    if(listener != nullptr)
        listener->on_kill(*this, you);
}