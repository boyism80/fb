#include <fb/game/server.h>

using namespace fb::game;

void listener_impl::on_equipment_on(character& me, item& item, EQUIPMENT_PARTS parts)
{
    me.send(fb_resp::item_update_slot(me, parts));
    me.sound(SOUND::EQUIPMENT_ON);

    std::stringstream sstream;
    switch (parts)
    {
    case EQUIPMENT_PARTS::WEAPON:
        sstream << "w:무기  :";
        break;

    case EQUIPMENT_PARTS::ARMOR:
        sstream << "a:갑옷  :";
        break;

    case EQUIPMENT_PARTS::SHIELD:
        sstream << "s:방패  :";
        break;

    case EQUIPMENT_PARTS::HELMET:
        sstream << "h:머리  :";
        break;

    case EQUIPMENT_PARTS::LEFT_HAND:
        sstream << "l:왼손  :";
        break;

    case EQUIPMENT_PARTS::RIGHT_HAND:
        sstream << "r:오른손  :";
        break;

    case EQUIPMENT_PARTS::LEFT_AUX:
        sstream << "[:보조1  :";
        break;

    case EQUIPMENT_PARTS::RIGHT_AUX:
        sstream << "]:보조2  :";
        break;
    }

    sstream << item.name();
    me.message(sstream.str(), MESSAGE_TYPE::STATE);

    sstream.str(std::string());
    sstream << std::format("갑옷 강도  {}  {} S  {}", me.phydef(), me.regenerative(), me.magdef());
    me.message(sstream.str(), MESSAGE_TYPE::STATE);
}

void listener_impl::on_equipment_off(character& me, EQUIPMENT_PARTS parts, fb::game::equipment& equipment)
{
    me.sound(SOUND::EQUIPMENT_OFF);
}

void listener_impl::on_durability_down(character& me, fb::game::equipment& equipment, uint32_t before, uint32_t after)
{
    auto& model          = equipment.based<fb::model::equipment>();
    auto  percent_before = (uint8_t)std::ceil((before * 100) / (double)model.durability);
    auto  percent_after  = (uint8_t)std::ceil((after * 100) / (double)model.durability);

    if (percent_before == percent_after)
        return;

    if (percent_before < percent_after)
        return;

    if (after == 0)
    {
        me.message(std::format("{}의 내구도가 다 닳았습니다.", equipment.name()));
    }
    else if (percent_after % 5 == 0 || percent_after < 5 || percent_before - percent_after >= 5)
    {
        me.message(std::format("{}의 내구도가 {}% 남았습니다.", equipment.name(), percent_after));
    }
}