#include <fb/game/server.h>
#include <fb/model/model.h>
#include <fb/protocol/client_version.h>

using namespace fb::game;

namespace game_resp = fb::protocol::game::response;

void listener_impl::on_equipment_on(character& me, item& item, EQUIPMENT_PARTS parts)
{
    me.stat.apply_equipment(static_cast<equipment&>(item).model(), 1);

    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::item_update_slot<V>(me, parts));
    });
    me.sound(SOUND::EQUIPMENT_ON);

    std::stringstream sstream;
    switch (parts)
    {
    case EQUIPMENT_PARTS::WEAPON:
        sstream << _TEXT(MESSAGE_EQUIPMENT_PART_WEAPON);
        break;

    case EQUIPMENT_PARTS::ARMOR:
        sstream << _TEXT(MESSAGE_EQUIPMENT_PART_ARMOR);
        break;

    case EQUIPMENT_PARTS::SHIELD:
        sstream << _TEXT(MESSAGE_EQUIPMENT_PART_SHIELD);
        break;

    case EQUIPMENT_PARTS::HELMET:
        sstream << _TEXT(MESSAGE_EQUIPMENT_PART_HELMET);
        break;

    case EQUIPMENT_PARTS::LEFT_HAND:
        sstream << _TEXT(MESSAGE_EQUIPMENT_PART_LEFT_HAND);
        break;

    case EQUIPMENT_PARTS::RIGHT_HAND:
        sstream << _TEXT(MESSAGE_EQUIPMENT_PART_RIGHT_HAND);
        break;

    case EQUIPMENT_PARTS::LEFT_AUX:
        sstream << _TEXT(MESSAGE_EQUIPMENT_PART_LEFT_AUX);
        break;

    case EQUIPMENT_PARTS::RIGHT_AUX:
        sstream << _TEXT(MESSAGE_EQUIPMENT_PART_RIGHT_AUX);
        break;
    }

    sstream << item.name();
    me.message(sstream.str(), MESSAGE_TYPE::STATE);

    sstream.str(std::string());
    sstream << std::format(_TEXT(MESSAGE_EQUIPMENT_STAT_ARMOR),
                           me.stat.phydef(),
                           me.stat.regenerative(),
                           me.stat.magdef());
    me.message(sstream.str(), MESSAGE_TYPE::STATE);

    me.update(UPDATE_STATE_LEVEL::BASED | UPDATE_STATE_LEVEL::HP_MP);
}

void listener_impl::on_equipment_off(character& me, EQUIPMENT_PARTS parts, fb::game::equipment& equipment)
{
    me.stat.apply_equipment(equipment.model(), -1);

    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::item_unequip<V>(parts));
    });
    me.sound(SOUND::EQUIPMENT_OFF);

    me.update(UPDATE_STATE_LEVEL::BASED | UPDATE_STATE_LEVEL::HP_MP);
}

void listener_impl::on_durability_down(character& me, fb::game::equipment& equipment, uint32_t before, uint32_t after)
{
    auto& model          = equipment.model();
    auto  percent_before = (uint8_t)std::ceil((before * 100) / (double)model.durability);
    auto  percent_after  = (uint8_t)std::ceil((after * 100) / (double)model.durability);

    if (percent_before == percent_after)
        return;

    if (percent_before < percent_after)
        return;

    if (after == 0)
    {
        me.message(std::format(_TEXT(MESSAGE_EQUIPMENT_DURABILITY_EXHAUSTED), equipment.name()));
    }
    else if (percent_after % 5 == 0 || percent_after < 5 || percent_before - percent_after >= 5)
    {
        me.message(std::format(_TEXT(MESSAGE_EQUIPMENT_DURABILITY_REMAINING), equipment.name(), percent_after));
    }
}