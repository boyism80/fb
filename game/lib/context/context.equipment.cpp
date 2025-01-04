#include <fb/game/context.h>

using namespace fb::game;

void context::on_equipment_on(character& me, item& item, EQUIPMENT_PARTS parts)
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
    sstream << "갑옷 강도  " << me.defensive_physical() << "  " << me.regenerative() << " S  "
            << me.defensive_magical();
    me.message(sstream.str(), MESSAGE_TYPE::STATE);
}

void context::on_equipment_off(character& me, EQUIPMENT_PARTS parts, uint8_t index)
{
    me.sound(SOUND::EQUIPMENT_OFF);
}