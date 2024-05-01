#include <fb/game/session.h>
#include <fb/game/map.h>
#include <fb/game/query.h>

std::string nowstr()
{
    std::stringstream sstream;
    sstream << '\'' << std::put_time(fb::now(), "%Y-%m-%d %H:%M:%S") << '\'';
    return sstream.str();
}


std::string fb::game::query::make_update_session(fb::game::session& session)
{
    auto map = session.map();
    auto clan = session.clan();
    auto dict = std::map<std::string, std::string>
    {
        { "last_login",      nowstr() },
        { "admin",           std::to_string((uint8_t)session.admin()) },
        { "look",            std::to_string(session.look()) },
        { "color",           std::to_string(session.color()) },
        { "sex",             std::to_string(static_cast<int>(session.sex())) },
        { "nation",          std::to_string(static_cast<int>(session.nation())) },
        { "creature",        std::to_string(static_cast<int>(session.creature())) },
        { "map",             std::to_string(map != nullptr ? map->id() : 0) },
        { "position_x",      std::to_string(map != nullptr ? session.x() : 1) },
        { "position_y",      std::to_string(map != nullptr ? session.y() : 1) },
        { "direction",       std::to_string(static_cast<int>(session.direction())) },
        { "state",           std::to_string(static_cast<int>(session.state())) },
        { "class",           std::to_string(session.cls()) },
        { "promotion",       std::to_string(session.promotion()) },
        { "exp",             std::to_string(session.experience()) },
        { "money",           std::to_string(session.money()) },
        { "deposited_money", std::to_string(session.deposited_money()) },
        { "disguise",        session.disguise().has_value() ? std::to_string(session.disguise().value()).c_str() : "NULL" },
        { "hp",              std::to_string(session.hp()) },
        { "base_hp",         std::to_string(session.base_hp()) },
        { "additional_hp",   std::to_string(0) },
        { "mp",              std::to_string(session.mp()) },
        { "base_mp",         std::to_string(session.base_mp()) },
        { "additional_mp",   std::to_string(0) },
        { "weapon_color",    "NULL" },
        { "helmet_color",    "NULL" },
        { "armor_color",     session.armor_color().has_value() ? std::to_string(session.armor_color().value()).c_str() : "NULL" },
        { "shield_color",    "NULL" }
    };

    std::stringstream sstream;
    sstream << "UPDATE user SET "
        << fb::query::make_update(dict)
        << " WHERE id="
        << session.id()
        << " LIMIT 1";

    return sstream.str();
}

std::string fb::game::query::make_update_item(fb::game::session& session)
{
    std::vector<std::string> item_set;
    for(auto i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = session.items[i];
        if (item == nullptr)
            continue;

        auto model = item->based<fb::game::item>();
        auto durability = item->durability();
        auto parameter = std::vector<std::string>
        {
            std::to_string(session.id()),
            std::to_string(i),
            std::to_string(static_cast<int>(fb::game::equipment::slot::UNKNOWN_SLOT)),
            std::to_string(model->id),
            std::to_string(item->count()),
            durability.has_value() ? std::to_string(durability.value()) : "NULL",
        };
        item_set.push_back(fb::query::make_insert(parameter));
    }

    const auto& equipments = session.items.equipments();
    for(auto& [slot, equipment] : equipments)
    {
        if (equipment == nullptr)
            continue;

        auto model = equipment->based<fb::game::equipment>();
        auto durability = equipment->durability();
        auto parameter = std::vector<std::string>
        {
            std::to_string(session.id()),
            std::to_string(-1),
            std::to_string(static_cast<int>(slot)),
            std::to_string(model->id),
            std::to_string(equipment->count()),
            durability.has_value() ? std::to_string(durability.value()) : "NULL",
        };

        item_set.push_back(fb::query::make_insert(parameter));
    }

    if (item_set.empty())
        return std::string();

    std::stringstream sstream;
    sstream << "INSERT INTO item (`owner`, `index`, `slot`, `model`, `count`, `durability`) VALUES "
        << boost::algorithm::join(item_set, ", ")
        << " ON DUPLICATE KEY UPDATE model=VALUES(model), count=VALUES(count), durability=VALUES(durability)";

    return sstream.str();
}

std::string fb::game::query::make_delete_item(fb::game::session& session)
{
    std::vector<std::string> item_slots;
    for(auto i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = session.items[i];
        if (item != nullptr)
            continue;

        item_slots.push_back(std::to_string(i));
    }

    std::vector<std::string> gear_slots;
    const auto& equipments = session.items.equipments();
    for (auto& [slot, equipment] : equipments)
    {
        if (equipment != nullptr)
            continue;

        gear_slots.push_back(std::to_string((uint8_t)slot));
    }

    if (item_slots.empty() && gear_slots.empty())
        return std::string();

    std::stringstream sstream;
    sstream << "DELETE FROM item WHERE `owner` = " << session.id() << " AND " 
            << "("
            << "`index` IN (" << boost::algorithm::join(item_slots, ", ") << ")" << " OR "
            << "`slot` IN (" << boost::algorithm::join(gear_slots, ", ") << ")"
            << ")";

    return sstream.str();
}

std::string fb::game::query::make_update_spell(fb::game::session& session)
{
    std::vector<std::string> spell_set;
    for(int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto spell = session.spells[i];
        if (spell == nullptr)
            continue;

        spell_set.push_back(fb::query::make_insert(std::vector<std::string>
        {
            std::to_string(session.id()),
            std::to_string(i),
            std::to_string(spell->id),
        }));
    }

    if (spell_set.empty())
        return std::string();

    std::stringstream sstream;
    sstream << "INSERT INTO spell (owner, slot, id) VALUES "
            << boost::algorithm::join(spell_set, ", ")
            << " ON DUPLICATE KEY UPDATE id=VALUES(id)";
    
    return sstream.str();
}

std::string fb::game::query::make_delete_spell(fb::game::session& session)
{
    std::vector<std::string> spell_slots;
    for(auto i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto spell = session.spells[i];
        if (spell != nullptr)
            continue;

        spell_slots.push_back(std::to_string(i));
    }

    if (spell_slots.empty())
        return std::string();

    std::stringstream sstream;
    sstream << "DELETE FROM spell WHERE `owner` = " << session.id() << " AND " 
            << "`slot` IN (" << boost::algorithm::join(spell_slots, ", ") << ")";

    return sstream.str();
}