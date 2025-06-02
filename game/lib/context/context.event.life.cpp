#include <fb/game/context.h>

using namespace fb::game;

void context::on_action(life& me, ACTION action, DURATION duration, uint8_t sound)
{
    this->send(me, fb_resp::action(me, action, duration), scope::PIVOT);
}

async::task<void> context::on_attack(life& me, DURATION duration)
{
    auto lua = lua::new_context();
    if (lua == nullptr)
        co_return;

#if defined DEBUG | defined _DEBUG
    lua->load("scripts/interaction.lua");
#endif
    lua->func("on_attack");
    lua->pushobject(me);
    if (co_await lua->call(1, false) == false)
        goto cleanup;

    if (this->alive(me) == false)
        goto cleanup;
    co_await me.thread()->switching();

    if (me.is(OBJECT_TYPE::CHARACTER))
    {
        auto  attack_count = (uint32_t)lua->tointeger(1);
        auto& ch           = static_cast<character&>(me);
        auto  weapon       = ch.items.weapon();
        if (weapon != nullptr)
        {
            auto& model = weapon->based<fb::model::weapon>();
            if (model.on_attack.empty() == false)
            {
                lua->func(model.on_attack);
                lua->pushobject(ch);
                lua->pushobject(weapon);
                co_await lua->call(2, false);

                if (this->alive(ch) == false)
                    goto cleanup;
                co_await ch.thread()->switching();
            }

            if (attack_count > 0 && weapon->durability_down(attack_count))
            {
                ch.message(std::format("{} 깨졌습니다.", weapon->name()));
                delete ch.items.equipment_off(EQUIPMENT_PARTS::WEAPON);
            }
        }
    }

cleanup:
    lua->release();
}

void context::on_dead(life& me, object* you)
{
    switch (me.what())
    {
    case OBJECT_TYPE::MOB:
    {
        auto& mob   = static_cast<fb::game::mob&>(me);
        auto& model = mob.based<fb::model::mob>();
        if (model.script.empty() == false && model.on_die.empty() == false)
        {
            auto lua = fb::lua::new_context();
            if (lua == nullptr)
                return;

#if defined DEBUG | defined _DEBUG
            lua->load(model.script);
#endif
            lua->func(model.on_die);
            lua->pushobject(mob);
            if (you != nullptr)
                lua->pushobject(you);
            else
                lua->pushnil();
            std::ignore = lua->call(2);
        }

        mob.drop_items();

        if (mob.owner != nullptr)
        {
            if (this->alive(*mob.owner))
                mob.owner->detach_spawned_mob(mob);
            return;
        }

        if (you != nullptr && you->is(OBJECT_TYPE::MOB))
            you = static_cast<fb::game::mob*>(you)->owner;

        if (you == nullptr)
            return;

        if (mob.owner == nullptr && you->is(OBJECT_TYPE::CHARACTER))
        {
            auto& ch    = static_cast<character&>(*you);
            auto& group = ch.group();
            auto  map   = ch.map();
            auto  exp   = mob.based<fb::model::mob>().exp;
            if (group != nullptr && map != nullptr)
            {
                group->lock([this, &ch, map, exp](auto& group) {
                    auto nears      = group.nears(*map, ch.position());
                    auto size       = nears.size();
                    auto divide_exp = exp / size;
                    for (auto member : nears)
                    {
                        member->add_exp(divide_exp, true, true);
                    }
                });
            }
            else
            {
                ch.add_exp(exp, true, true);
            }
        }
    }
    break;

    case OBJECT_TYPE::CHARACTER:
    {
        auto& ch = static_cast<character&>(me);
        ch.death_penalty();
        ch.state(STATE::GHOST);
    }
    break;
    }
}

void context::on_update_hp(life& me, uint32_t diff, bool critical)
{
    this->send(me, fb_resp::update_hp(me, diff, critical), scope::PIVOT);
}