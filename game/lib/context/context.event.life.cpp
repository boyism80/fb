#include <fb/game/context.h>

using namespace fb::game;

void context::on_action(life& me, ACTION action, DURATION duration, uint8_t sound)
{
    this->send(me, fb_resp::action(me, action, duration), scope::PIVOT);
}

void context::on_attack(life& me, DURATION duration)
{
    auto lua = lua::new_context();
#if defined DEBUG | defined _DEBUG
    lua->from("scripts/interaction.lua");
#endif
    lua->func("on_attack");
    lua->pushobject(me);
    lua->resume(1, false);
    auto attack_count = (uint32_t)lua->tointeger(1);

    if (me.is(OBJECT_TYPE::CHARACTER))
    {
        auto& ch     = static_cast<character&>(me);
        auto  weapon = ch.items.weapon();
        if (weapon != nullptr)
        {
            auto& model = weapon->based<fb::model::weapon>();
            if (model.script_attack != "")
            {
                lua->from(model.script_attack);
                lua->func("on_attack");
                lua->pushobject(ch);
                lua->pushobject(weapon);
                lua->resume(2, false);
            }

            if (attack_count > 0)
            {
                if (attack_count > weapon->durability())
                {
                    ch.message(std::format("{} 깨졌습니다.", weapon->name()));
                    delete ch.items.equipment_off(EQUIPMENT_PARTS::WEAPON);
                }
                else
                {
                    weapon->durability(weapon->durability().value() - attack_count);
                }
            }
        }
    }
    lua->release();
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

        for (int i = 0; i < CONTAINER_CAPACITY; i++)
        {
            auto item = ch.items[i];
            if (item == nullptr)
                continue;

            auto& model = item->based<fb::model::item>();
            if (model.attr(ITEM_ATTRIBUTE::EQUIPMENT))
            {
                auto  equipment       = static_cast<fb::game::equipment*>(item);
                auto& equipment_model = equipment->based<fb::model::equipment>();
                auto  penalty         = equipment_model.durability * fb::model::const_value::death_penalty::durability;

                if (equipment->durability().value() <= penalty)
                {
                    delete ch.items.remove(i, 1, ITEM_DELETE_TYPE::DESTROY);
                    continue;
                }

                equipment->durability(equipment->durability().value() - penalty);
            }

            if (ENUM_IN(model.death_penalty, DEATH_PENALTY::DROP))
            {
                auto dropped = ch.items.remove(*item, item->count(), ITEM_DELETE_TYPE::NONE);
                dropped->map(ch.map(), ch.position());
            }
        }

        for (auto& [parts, equipment] : ch.items.equipments())
        {
            if (equipment == nullptr)
                continue;

            auto& model   = equipment->based<fb::model::equipment>();
            auto  penalty = model.durability * fb::model::const_value::death_penalty::durability;
            if (equipment->durability().value() <= penalty)
            {
                ch.items.equipment_off(parts);
                ch.message(std::format("{} 깨졌습니다.", equipment->name()));
                delete equipment;
                continue;
            }

            equipment->durability(equipment->durability().value() - penalty);
            if (ENUM_IN(model.death_penalty, DEATH_PENALTY::DROP))
            {
                ch.items.equipment_off(parts);
                equipment->map(ch.map(), ch.position());
            }
            else if (ch.items.free())
            {
                ch.items.equipment_off(parts);
                ch.items.add(equipment);
            }
        }

        auto cls   = ch.cls();
        auto level = ch.level();
        if (this->model.ability.contains(cls) && this->model.ability[cls].contains(level) &&
            this->model.ability[cls].contains(level - 1))
        {
            auto penalty = uint32_t(this->model.ability[cls][level].exp * fb::model::const_value::death_penalty::exp);
            auto gained  = ch.exp() - this->model.ability[cls][level - 1].stacked_exp;

            penalty = std::min(gained, penalty);
            if (penalty > 0)
            {
                ch.exp(ch.exp() - penalty);
                ch.message(std::format("경험치를 {} 잃었습니다.", penalty));
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