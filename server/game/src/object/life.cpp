#include <fb/game/server.h>
#include <fb/game/life.h>
#include <fb/game/mob.h>
#include <fb/game/character.h>
#include <fb/game/map.h>
#include <fb/model/model.h>
#include <fb/encoding.h>
#include <json/json.h>
#include <unordered_map>
#include <tuple>

using namespace fb::game;

life::life(fb::game::server& server, const fb::model::life& model, fb::game::stat& stat, const initial_params& params) :
    object(server, model, params),
    listener(server.listener),
    stat(stat)
{ }

life::~life()
{ }

void life::on_init()
{
    this->spells.owner(this->shared_from_this_as<life>());
}

void life::update(UPDATE_STATE_LEVEL value)
{ }

void life::update_hp(uint32_t diff, bool critical, bool notify)
{
    if (!notify)
        return;

    this->listener.on_update_hp(*this, diff, critical);
}

life::batch_update_guard life::batch_update()
{
    return batch_update_guard(*this);
}

life::batch_update_guard::batch_update_guard(life& owner) :
    _owner(owner)
{
    this->_owner.assert_thread();
    this->_owner._batch_mode     = true;
    this->_owner._pending_update = UPDATE_STATE_LEVEL::MINIMUM;
}

life::batch_update_guard::~batch_update_guard()
{
    this->_owner.assert_thread();
    this->_owner._batch_mode = false;
    if (this->_owner._pending_update != UPDATE_STATE_LEVEL::MINIMUM)
    {
        auto pending                 = this->_owner._pending_update;
        this->_owner._pending_update = UPDATE_STATE_LEVEL::MINIMUM;
        this->_owner.update(pending);
    }
}

void life::kill(DESTROY_TYPE destroy_type)
{
    this->assert_thread();
    this->stat.hp(0, false);
}

life::mob_vector life::damage_targets(const damage_list& targets, const damage_opts& opts)
{
    this->assert_thread();

    auto attacker = this->shared_from_this_as<life>();
    auto dead     = mob_vector{};

    for (auto& [target, value] : targets)
    {
        if (target == nullptr)
            continue;

        target->stat.damage(value, attacker, opts.critical, opts.rate, opts.physical, opts.fixed, opts.notify);

        if (target->is(OBJECT_TYPE::MOB))
        {
            auto m = std::static_pointer_cast<mob>(target);

            // Part of an assembly: never settle the part; settle the body if it died.
            auto body = m->body();
            if (body != nullptr)
            {
                if (body->stat.hp() == 0 && body->invincible() == false)
                {
                    body->invincible(true);
                    dead.push_back(body);
                }
                continue;
            }

            // character::alive() means "not ghost", so death must be detected by HP.
            if (m->stat.hp() != 0)
                continue;

            // Already settling ON_MOB_KILL / ON_MOB_DIE — do not re-enter kill flow.
            if (m->invincible())
                continue;

            m->invincible(true);
            dead.push_back(m);
        }
        else if (target->is(OBJECT_TYPE::CHARACTER))
        {
            if (target->stat.hp() != 0)
                continue;

            auto ch = std::static_pointer_cast<character>(target);
            if (ch->alive() == false)
                continue;

            ch->kill(DESTROY_TYPE::DEAD);
            ch->notify_death(attacker);
        }
    }

    return dead;
}

async::task<void> life::settle_deaths(mob_vector dead)
{
    this->assert_thread();

    auto groups = std::unordered_map<uint32_t, mob_vector>{};
    for (auto& m : dead)
    {
        if (m == nullptr)
            continue;
        groups[m->based<fb::model::mob>().id].push_back(m);
    }

    for (auto& [id, mobs] : groups)
    {
        auto path = std::format("scripts/mob/{}.lua", id);
        auto func = std::format("ON_MOB_DIE_{}", id);
        auto lua  = this->server.lua.open(path, func);
        if (lua)
        {
            lua->pushobject(*mobs.front());
            try
            {
                std::ignore = co_await lua->call(1);
            }
            catch (std::exception& e)
            {
                fb::logger::fatal("error in ON_MOB_DIE_{}: {}", id, e.what());
            }
            catch (...)
            {
                fb::logger::fatal("unknown error in ON_MOB_DIE_{}", id);
            }
        }

        for (auto& m : mobs)
        {
            auto owner = m->owner.lock();
            if (owner != nullptr)
            {
                owner->detach_spawned_mob(*m);
                m->kill(DESTROY_TYPE::DEAD);
                co_await m->destroy(DESTROY_TYPE::DEAD);
                continue;
            }

            this->listener.on_dead(*m, nullptr);
            co_await m->drop_items();
            m->kill(DESTROY_TYPE::DEAD);
            co_await m->destroy(DESTROY_TYPE::DEAD);
        }
    }
}

async::task<void> life::damage_to(const damage_list& targets)
{
    co_await this->damage_to(targets, damage_opts{});
    co_return;
}

async::task<void> life::damage_to(const damage_list& targets, const damage_opts& opts)
{
    this->assert_thread();
    auto dead = this->damage_targets(targets, opts);
    if (dead.empty() == false)
        co_await this->settle_deaths(std::move(dead));
    co_return;
}

async::task<void> life::attack(DURATION duration)
{
    this->assert_thread();
    if (this->_map == nullptr)
        co_return;

    if (this->alive() == false)
        co_return;

    // Execute attack interaction script and get attack count
    uint32_t attack_count = 0;
    auto     lua          = this->server.lua.open("scripts/interaction.lua", "on_attack");
    if (lua)
    {
        lua->pushobject(*this);
        if (co_await lua->call(1))
            attack_count = (uint32_t)lua->tointeger(1);
    }

    // Call listener for packet response
    this->listener.on_attack(*this, duration);

    // Handle weapon durability and script logic
    if (this->is(OBJECT_TYPE::CHARACTER))
    {
        auto ch     = static_cast<character*>(this);
        auto weapon = ch->items.weapon();
        if (weapon != nullptr)
        {
            auto& model = weapon->based<fb::model::weapon>();
            auto  path  = std::format("scripts/item/{}.lua", model.id);
            auto  func  = std::format("ON_ATTACK_{}", model.id);

            auto weapon_lua = this->server.lua.open(path, func);
            if (weapon_lua)
            {
                weapon_lua->pushobject(ch);
                weapon_lua->pushobject(weapon);
                std::ignore = weapon_lua->call(2);
            }

            // Handle weapon durability
            if (attack_count > 0 && weapon->durability_down(attack_count))
            {
                ch->message(std::format(_TEXT(MESSAGE_EQUIPMENT_BROKEN), weapon->name()));
                std::ignore = ch->items.equipment_off(EQUIPMENT_PARTS::WEAPON);
            }
        }
    }
    co_return;
}

uint32_t life::exp() const
{
    this->assert_thread();
    return static_cast<const fb::model::life&>(this->_model).exp;
}

bool life::alive() const
{
    this->assert_thread();
    return this->stat.hp() != 0;
}

bool life::active(fb::game::spell& spell, std::string_view message)
{
    this->assert_thread();

    auto& model = spell.model;
    auto  path  = std::format("scripts/spell/{}.lua", model.id);
    auto  func  = std::format("ON_CAST_{}", model.id);

    auto lua = this->server.lua.open(path, func);
    if (!lua)
        return false;

    if (spell.model.type != SPELL_TYPE::INPUT)
        return false;

    lua->pushobject(this);
    lua->pushobject(spell.model);
    lua->pushstring(message);
    std::ignore = lua->call(3);
    return true;
}

bool life::active(fb::game::spell& spell, uint32_t oid)
{
    this->assert_thread();
    if (this->_map == nullptr)
        return false;

    auto to = this->_map->objects[oid];
    if (to == nullptr)
        return false;

    return this->active(spell, *to);
}

bool life::active(fb::game::spell& spell, fb::game::object& to)
{
    this->assert_thread();
    auto& model = spell.model;
    auto  path  = std::format("scripts/spell/{}.lua", model.id);
    auto  func  = std::format("ON_CAST_{}", model.id);

    auto lua = this->server.lua.open(path, func);
    if (!lua)
        return false;

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
    std::ignore = lua->call(3);
    return true;
}

bool life::active(fb::game::spell& spell)
{
    this->assert_thread();
    auto& model = spell.model;
    auto  path  = std::format("scripts/spell/{}.lua", model.id);
    auto  func  = std::format("ON_CAST_{}", model.id);

    auto lua = this->server.lua.open(path, func);
    if (!lua)
        return false;

    if (spell.model.type != SPELL_TYPE::NORMAL)
        return false;

    lua->pushobject(this);
    lua->pushobject(spell.model);
    std::ignore = lua->call(2);
    return true;
}

void life::action(ACTION action, DURATION duration, uint8_t sound)
{
    this->listener.on_action(*this, action, duration, sound);
}

bool life::calculate_critical(life& you) const
{
    this->assert_thread();

#if defined DEBUG || defined _DEBUG
    return true;
#else
    return std::rand() % 100 < 20;
#endif
}

bool life::calculate_miss(life& you) const
{
    this->assert_thread();

#if defined DEBUG || defined _DEBUG
    return false;
#else
    return std::rand() % 3 == 0;
#endif
}

uint32_t life::calculate_damage(uint32_t value, const life& target, bool critical, float rate, bool physical) const
{
    this->assert_thread();
    auto def               = physical ? target.stat.phydef() : target.stat.magdef();
    auto n                 = (100 - def) / 10;
    auto defensive_percent = -125 + (n * (2 * 14.75f - (n - 1) / 2.0f)) / 2.0f;
    auto damage            = value - uint32_t(defensive_percent * (value / 100.0f));

    if (physical && target.direction() == this->direction())
        rate *= 2.0f;

    if (critical)
        rate *= 2.0f;

    rate /= (target.damage_derate() / 1000.0f);
    return static_cast<uint32_t>(damage * rate);
}

uint32_t life::damage_rate() const
{
    return this->_damage_rate;
}

void life::damage_rate(uint32_t value)
{
    this->assert_thread();
    this->_damage_rate = value;
}

uint32_t life::skill_damage_rate() const
{
    return this->_skill_damage_rate;
}

void life::skill_damage_rate(uint32_t value)
{
    this->assert_thread();
    this->_skill_damage_rate = value;
}

uint32_t life::damage_derate() const
{
    return this->_damage_derate;
}

void life::damage_derate(uint32_t value)
{
    this->assert_thread();
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
