#include <fb/game/server.h>

#include <fb/model/model.h>
#include <tuple>

using namespace fb::game;

namespace game_resp = fb::protocol::game::response;

async::task<void> listener_impl::on_equipment_on(character& me, item& item, EQUIPMENT_PARTS parts)

{

    std::ignore = co_await me.send(game_resp::item_update_slot(me, parts));

    co_await me.sound(SOUND::EQUIPMENT_ON);

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

    co_await me.message(sstream.str(), MESSAGE_TYPE::STATE);

    sstream.str(std::string());

    sstream << std::format(_TEXT(MESSAGE_EQUIPMENT_STAT_ARMOR),

                           me.stat.phydef(),

                           me.stat.regenerative(),

                           me.stat.magdef());

    co_await me.message(sstream.str(), MESSAGE_TYPE::STATE);
}

async::task<void> listener_impl::on_equipment_off(character& me, EQUIPMENT_PARTS parts, fb::game::equipment& equipment)

{

    co_await me.sound(SOUND::EQUIPMENT_OFF);
}

async::task<void>
listener_impl::on_durability_down(character& me, fb::game::equipment& equipment, uint32_t before, uint32_t after)

{

    auto& model = equipment.based<fb::model::equipment>();

    auto percent_before = (uint8_t)std::ceil((before * 100) / (double)model.durability);

    auto percent_after = (uint8_t)std::ceil((after * 100) / (double)model.durability);

    if (percent_before == percent_after)

        co_return;

    if (percent_before < percent_after)

        co_return;

    if (after == 0)

    {

        co_await me.message(std::format(_TEXT(MESSAGE_EQUIPMENT_DURABILITY_EXHAUSTED), equipment.name()));
    }

    else if (percent_after % 5 == 0 || percent_after < 5 || percent_before - percent_after >= 5)

    {

        co_await me.message(
            std::format(_TEXT(MESSAGE_EQUIPMENT_DURABILITY_REMAINING), equipment.name(), percent_after));
    }
}
