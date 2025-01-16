#include <fb/game/context.h>

using namespace fb::game;

void context::on_action(life& me, ACTION action, DURATION duration, uint8_t sound)
{
    this->send(me, fb_resp::action(me, action, duration), scope::PIVOT);
}

void context::on_attack(life& me, DURATION duration)
{
    static auto PK = false;

    me.action(ACTION::ATTACK, duration);
    if (me.is(OBJECT_TYPE::CHARACTER))
    {
        auto* weapon = static_cast<character&>(me).items.weapon();
        if (weapon != nullptr)
        {
            auto sound = weapon->based<fb::model::weapon>().sound;
            me.sound(sound != 0 ? SOUND(sound) : SOUND::SWING);
        }
    }

    auto front = static_cast<fb::game::life*>(me.forward(OBJECT_TYPE::LIFE));
    if (front == nullptr)
        return;

    if (!PK && me.is(OBJECT_TYPE::CHARACTER) && front->is(OBJECT_TYPE::CHARACTER))
        return;

    auto miss = me.calculate_miss(*front);
    if (miss)
        return;

    if (me.is(OBJECT_TYPE::CHARACTER))
    {
        auto* weapon = static_cast<character&>(me).items.weapon();
        if (weapon != nullptr)
        {
            auto sound = weapon->based<fb::model::weapon>().sound;
            front->sound(SOUND::DAMAGE);
        }
    }

    auto critical = me.calculate_critical(*front);
    auto mob_size = MOB_SIZE::LARGE;
    if (front->is(OBJECT_TYPE::MOB))
    {
        auto& model = static_cast<fb::game::mob*>(front)->based<fb::model::mob>();
        mob_size    = model.size;
    }
    auto damage = me.calculate_damage(me.auto_attack_damage(mob_size), *front, critical);
    if (me.is(OBJECT_TYPE::CHARACTER))
    {
        auto thread = lua::new_context();
        thread->from("scripts/common/attack.lua");
        thread->func("on_attack");
        thread->pushobject(me);
        thread->pushobject(*front);
        thread->pushinteger(damage);
        thread->resume(3, false);
        damage = thread->tointeger(1);
        thread->release();
    }
    front->damage(damage, &me, critical);
}

void context::on_dead(life& me, object* you)
{
    switch (me.what())
    {
    case OBJECT_TYPE::MOB:
    {
        auto& mob = static_cast<fb::game::mob&>(me);
        mob.drop_items();

        if (you != nullptr && you->is(OBJECT_TYPE::CHARACTER))
        {
            auto& ch = static_cast<character&>(*you);

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
        ch.state(STATE::GHOST);
        // 템 떨구기
    }
    break;
    }
}

void context::on_update_hp(life& me, uint32_t diff, bool critical)
{
    this->send(me, fb_resp::update_hp(me, diff, critical), scope::PIVOT);
}