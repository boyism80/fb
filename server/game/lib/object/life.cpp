#include <fb/game/server.h>
#include <fb/game/life.h>
#include <fb/game/map.h>

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

void life::update_hp(uint32_t diff, bool critical)
{
    this->listener.on_update_hp(*this, diff, critical);
}

void life::kill(std::shared_ptr<fb::game::object> from, DESTROY_TYPE destroy_type)
{
    this->stat.hp(0);

    // Call listener for packet response
    this->listener.on_dead(*this, from);

    // Handle death logic based on object type
    switch (this->what())
    {
    case OBJECT_TYPE::MOB:
    {
        auto& mob   = static_cast<fb::game::mob&>(*this);
        auto& model = mob.based<fb::model::mob>();

        // Execute mob death script
        if (model.script.empty() == false && model.on_die.empty() == false)
        {
            auto lua = fb::lua::new_context();
            if (lua != nullptr)
            {
#if defined DEBUG | defined _DEBUG
                lua->load(model.script);
#endif
                lua->func(model.on_die);
                lua->pushobject(mob);
                if (from != nullptr)
                    lua->pushobject(from);
                else
                    lua->pushnil();
                std::ignore = lua->call(2);
            }
        }

        // Drop items when mob dies
        std::ignore = mob.drop_items();

        // Handle spawned mob ownership
        auto owner = mob.owner.lock();
        if (owner != nullptr)
        {
            owner->detach_spawned_mob(mob);
            return;
        }

        // Handle experience distribution
        if (from != nullptr && from->is(OBJECT_TYPE::MOB))
            from = std::static_pointer_cast<fb::game::mob>(from)->owner.lock();

        if (from == nullptr)
            return;

        if (owner == nullptr && from->is(OBJECT_TYPE::CHARACTER))
        {
            auto& ch       = static_cast<character&>(*from);
            auto& group_id = ch.group_id();
            auto  map      = ch.map();
            auto  exp      = mob.based<fb::model::mob>().exp;

            if (group_id.has_value() && map != nullptr)
            {
                // Group experience distribution
                auto server = &ch.server;
                server->groups.read(group_id.value(), [server, &ch, map, exp](auto& group) {
                    auto nears      = group->nears(*map, ch.position());
                    auto size       = nears.size();
                    auto divide_exp = exp / size;
                    for (auto& member : nears)
                    {
                        auto shared_ptr = member.lock();
                        if (shared_ptr == nullptr)
                            continue;

                        shared_ptr->add_exp(divide_exp, true, true);
                    }
                });
            }
            else
            {
                // Solo experience
                ch.add_exp(exp, true, true);
            }
        }
    }
    break;

    case OBJECT_TYPE::CHARACTER:
    {
        auto& ch = static_cast<character&>(*this);
        ch.death_penalty();
        ch.state(STATE::GHOST);
    }
    break;
    }
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
    auto     lua          = fb::lua::new_context();
    if (lua != nullptr)
    {
#if defined DEBUG | defined _DEBUG
        lua->load("scripts/interaction.lua");
#endif
        lua->func("on_attack");
        lua->pushobject(*this);
        if (co_await lua->call(1))
        {
            attack_count = (uint32_t)lua->tointeger(1);
        }
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

            // Execute weapon's on_attack script
            if (model.on_attack.empty() == false)
            {
                auto weapon_lua = fb::lua::new_context();
                if (weapon_lua != nullptr)
                {
#if defined DEBUG | defined _DEBUG
                    weapon_lua->load(model.script);
#endif
                    weapon_lua->func(model.on_attack);
                    weapon_lua->pushobject(ch);
                    weapon_lua->pushobject(weapon);
                    std::ignore = weapon_lua->call(2);
                }
            }

            // Handle weapon durability
            if (attack_count > 0 && weapon->durability_down(attack_count))
            {
                ch->message(std::format("{} 깨졌습니다.", weapon->name()));
                ch->items.equipment_off(EQUIPMENT_PARTS::WEAPON);
            }
        }
    }
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

bool life::active(fb::game::spell& spell, const std::string& message)
{
    this->assert_thread();

    if (spell.model.cast.empty())
        return false;

    auto lua = fb::lua::new_context();
    if (lua == nullptr)
        return false;

#if defined DEBUG | defined _DEBUG
    lua->load("scripts/spell.lua");
    lua->load(spell.model.script);
#endif
    lua->func(spell.model.cast);
    if (spell.model.type != SPELL_TYPE::INPUT)
        return false;

    lua->pushobject(this);
    lua->pushobject(spell.model);
    lua->pushstring(message);
    std::ignore = lua->call(3);
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
    lua->load(spell.model.script);
#endif
    lua->func(spell.model.cast);
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
    auto lua = fb::lua::new_context();
    if (lua == nullptr)
        return false;

#if defined DEBUG | defined _DEBUG
    lua->load("scripts/spell.lua");
    lua->load(spell.model.script);
#endif
    lua->func(spell.model.cast);
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
    auto n                 = (100 - life.stat.phydef()) / 10;
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