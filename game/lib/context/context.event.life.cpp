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
    thread->resume(1, false);

    if (me.is(OBJECT_TYPE::CHARACTER))
    {
        auto& ch     = static_cast<character&>(me);
        auto  weapon = ch.items.weapon();
        if (weapon != nullptr)
        {
            auto& model = weapon->based<fb::model::weapon>();
            if (model.script_attack != "")
            {
                thread->from(model.script_attack);
                thread->func("on_attack");
                thread->pushobject(ch);
                thread->pushobject(weapon);
                thread->resume(2, false);
            }
        }
    }
    thread->release();
}

void context::on_dead(life& me, object* you)
{
    switch (me.what())
    {
    case OBJECT_TYPE::MOB:
    {
        auto& mob = static_cast<fb::game::mob&>(me);
        if (mob.owner != nullptr)
            return;

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

        auto money = ch.money();
        if (money > 0)
        {
            ch.money_reduce(money);
            auto cash = this->make<fb::game::cash>(money);
            cash->owner(&ch);
            cash->map(ch.map(), ch.position());
        }

        for (auto item : ch.items)
        {
            if (item == nullptr)
                continue;

            auto& model = item->based<fb::model::item>();
            if (ENUM_IN(model.death_penalty, DEATH_PENALTY::DROP))
            {
                auto dropped = ch.items.remove(*item, item->count(), ITEM_DELETE_TYPE::NONE);
                dropped->map(ch.map(), ch.position());
            }
        }

        auto weapon = ch.items.weapon();
        if (weapon != nullptr)
        {
            auto& model = weapon->based<fb::model::weapon>();
            if (ENUM_IN(model.death_penalty, DEATH_PENALTY::DROP))
            {
                ch.items.weapon(nullptr);
                weapon->map(ch.map(), ch.position());
            }
            else
            {
                if (ch.items.free())
                    ch.items.inactive(EQUIPMENT_PARTS::WEAPON);
            }
        }

        auto armor = ch.items.armor();
        if (armor != nullptr)
        {
            auto& model = armor->based<fb::model::armor>();
            if (ENUM_IN(model.death_penalty, DEATH_PENALTY::DROP))
            {
                ch.items.armor(nullptr);
                armor->map(ch.map(), ch.position());
            }
            else
            {
                if (ch.items.free())
                    ch.items.inactive(EQUIPMENT_PARTS::WEAPON);
            }
        }

        auto helmet = ch.items.helmet();
        if (helmet != nullptr)
        {
            auto& model = helmet->based<fb::model::helmet>();
            if (ENUM_IN(model.death_penalty, DEATH_PENALTY::DROP))
            {
                ch.items.helmet(nullptr);
                helmet->map(ch.map(), ch.position());
            }
            else
            {
                if (ch.items.free())
                    ch.items.inactive(EQUIPMENT_PARTS::WEAPON);
            }
        }

        auto shield = ch.items.shield();
        if (shield != nullptr)
        {
            auto& model = shield->based<fb::model::shield>();
            if (ENUM_IN(model.death_penalty, DEATH_PENALTY::DROP))
            {
                ch.items.shield(nullptr);
                shield->map(ch.map(), ch.position());
            }
            else
            {
                if (ch.items.free())
                    ch.items.inactive(EQUIPMENT_PARTS::WEAPON);
            }
        }

        auto ring1 = ch.items.ring(EQUIPMENT_POSITION::LEFT);
        if (ring1 != nullptr)
        {
            auto& model = ring1->based<fb::model::ring>();
            if (ENUM_IN(model.death_penalty, DEATH_PENALTY::DROP))
            {
                ch.items.ring(nullptr, EQUIPMENT_POSITION::LEFT);
                ring1->map(ch.map(), ch.position());
            }
            else
            {
                if (ch.items.free())
                    ch.items.inactive(EQUIPMENT_PARTS::WEAPON);
            }
        }

        auto ring2 = ch.items.ring(EQUIPMENT_POSITION::RIGHT);
        if (ring2 != nullptr)
        {
            auto& model = ring2->based<fb::model::ring>();
            if (ENUM_IN(model.death_penalty, DEATH_PENALTY::DROP))
            {
                ch.items.ring(nullptr, EQUIPMENT_POSITION::RIGHT);
                ring2->map(ch.map(), ch.position());
            }
            else
            {
                if (ch.items.free())
                    ch.items.inactive(EQUIPMENT_PARTS::WEAPON);
            }
        }

        auto aux1 = ch.items.auxiliary(EQUIPMENT_POSITION::LEFT);
        if (aux1 != nullptr)
        {
            auto& model = aux1->based<fb::model::auxiliary>();
            if (ENUM_IN(model.death_penalty, DEATH_PENALTY::DROP))
            {
                ch.items.auxiliary(nullptr, EQUIPMENT_POSITION::LEFT);
                aux1->map(ch.map(), ch.position());
            }
            else
            {
                if (ch.items.free())
                    ch.items.inactive(EQUIPMENT_PARTS::WEAPON);
            }
        }

        auto aux2 = ch.items.auxiliary(EQUIPMENT_POSITION::RIGHT);
        if (aux2 != nullptr)
        {
            auto& model = aux2->based<fb::model::auxiliary>();
            if (ENUM_IN(model.death_penalty, DEATH_PENALTY::DROP))
            {
                ch.items.auxiliary(nullptr, EQUIPMENT_POSITION::RIGHT);
                aux2->map(ch.map(), ch.position());
            }
            else
            {
                if (ch.items.free())
                    ch.items.inactive(EQUIPMENT_PARTS::WEAPON);
            }
        }
    }
    break;
    }
}

void context::on_update_hp(life& me, uint32_t diff, bool critical)
{
    this->send(me, fb_resp::update_hp(me, diff, critical), scope::PIVOT);
}