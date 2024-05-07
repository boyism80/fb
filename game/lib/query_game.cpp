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
    auto item_set = std::vector<std::string>();
    for(auto i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = session.items[i];
        if (item == nullptr)
            continue;

        auto model = item->based<fb::game::item>();
        auto durability = item->durability();
        auto custom_name = (item->attr(fb::game::item::ATTRIBUTE::WEAPON)) ? static_cast<fb::game::weapon*>(item)->custom_name() : std::optional<std::string>();
        auto parameter = std::vector<std::string>
        {
            /* owner       */ std::to_string(session.id()),
            /* index       */ std::to_string(i),
            /* parts       */ std::to_string(static_cast<int>(fb::game::equipment::parts::UNKNOWN)),
            /* deposited   */ std::to_string(-1),
            /* model       */ std::to_string(model->id),
            /* count       */ std::to_string(item->count()),
            /* durability  */ durability.has_value() ? std::to_string(durability.value()) : "NULL",
            /* custom_name */ custom_name.has_value() ? custom_name.value() : "NULL"
        };
        item_set.push_back(fb::query::make_insert(parameter));
    }

    for(auto& [parts, equipment] : session.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        auto model = equipment->based<fb::game::equipment>();
        auto durability = equipment->durability();
        auto custom_name = (equipment->attr(fb::game::item::ATTRIBUTE::WEAPON)) ? static_cast<fb::game::weapon*>(equipment)->custom_name() : std::optional<std::string>();
        auto parameter = std::vector<std::string>
        {
            /* owner       */ std::to_string(session.id()),
            /* index       */ std::to_string(-1),
            /* parts       */ std::to_string(static_cast<int>(parts)),
            /* deposited   */ std::to_string(-1),
            /* model       */ std::to_string(model->id),
            /* count       */ std::to_string(equipment->count()),
            /* durability  */ durability.has_value() ? std::to_string(durability.value()) : "NULL",
            /* custom_name */ custom_name.has_value() ? custom_name.value() : "NULL"
        };

        item_set.push_back(fb::query::make_insert(parameter));
    }

    auto& deposited_items = session.deposited_items();
    for(int i = 0; i < deposited_items.size(); i++)
    {
        auto item = deposited_items.at(i);
        auto model = item->based<fb::game::item>();
        auto durability = item->durability();
        auto custom_name = (item->attr(fb::game::item::ATTRIBUTE::WEAPON)) ? static_cast<fb::game::weapon*>(item)->custom_name() : std::optional<std::string>();
        auto parameter = std::vector<std::string>
        {
            /* owner       */ std::to_string(session.id()),
            /* index       */ std::to_string(-1),
            /* parts       */ std::to_string(static_cast<int>(fb::game::equipment::parts::UNKNOWN)),
            /* deposited   */ std::to_string(i),
            /* model       */ std::to_string(model->id),
            /* count       */ std::to_string(item->count()),
            /* durability  */ durability.has_value() ? std::to_string(durability.value()) : "NULL",
            /* custom_name */ custom_name.has_value() ? custom_name.value() : "NULL"
        };
        item_set.push_back(fb::query::make_insert(parameter));
    }

    if (item_set.empty())
        return std::string();

    std::stringstream sstream;
    sstream << "INSERT INTO item (`owner`, `index`, `parts`, `deposited`, `model`, `count`, `durability`, `custom_name`) VALUES "
        << boost::algorithm::join(item_set, ", ")
        << " ON DUPLICATE KEY UPDATE model=VALUES(model), count=VALUES(count), durability=VALUES(durability), custom_name=VALUES(custom_name)";

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
    for (auto& [parts, equipment] : equipments)
    {
        if (equipment != nullptr)
            continue;

        gear_slots.push_back(std::to_string((uint8_t)parts));
    }

    if (item_slots.empty() && gear_slots.empty())
        return std::string();

    std::stringstream sstream;
    sstream << "DELETE FROM item WHERE `owner` = " << session.id() << " AND " 
            << "("
            << "`index` IN (" << boost::algorithm::join(item_slots, ", ") << ")" << " OR "
            << "`parts` IN (" << boost::algorithm::join(gear_slots, ", ") << ")"
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