#include <fb/game/context.h>

using namespace fb::game;

void context::on_action(life& me, ACTION action, DURATION duration, uint8_t sound)
{
    this->send(me, fb_resp::action(me, action, duration), scope::PIVOT);
}

void context::on_attack(life& me, DURATION duration)
{
    auto thread = lua::new_context();
#if defined DEBUG | defined _DEBUG
    thread->from("scripts/interaction.lua");
#endif
    thread->func("on_attack");
    thread->pushobject(me);
    thread->resume(1);
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