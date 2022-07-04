#include <fb/game/session.h>
#include <fb/game/map.h>
#include <fb/game/query.h>

std::string nowstr()
{
    std::stringstream sstream;
    sstream << '\'' << std::put_time(fb::now(), "%Y-%m-%d %H:%M:%S") << '\'';

    auto now_s = sstream.str();
    std::cout << now_s;

    return sstream.str();
}


std::string fb::game::query::make_update_session(fb::game::session& session)
{
    auto map = session.map();
    if(map == nullptr)
        map = session.before().map;

    auto clan = session.clan();

    auto dict = std::map<std::string, std::string>
    {
        { "last_login",    nowstr() },
        { "admin",         std::to_string((uint8_t)session.admin()) },
        { "look",          std::to_string(session.look()) },
        { "color",         std::to_string(session.color()) },
        { "sex",           std::to_string(static_cast<int>(session.sex())) },
        { "nation",        std::to_string(static_cast<int>(session.nation())) },
        { "creature",      std::to_string(static_cast<int>(session.creature())) },
        { "map",           std::to_string(map != nullptr ? map->id() : 0) },
        { "position_x",    std::to_string(map != nullptr ? session.x() : 1) },
        { "position_y",    std::to_string(map != nullptr ? session.y() : 1) },
        { "direction",     std::to_string(static_cast<int>(session.direction())) },
        { "state",         std::to_string(static_cast<int>(session.state())) },
        { "class",         std::to_string(session.cls()) },
        { "promotion",     std::to_string(session.promotion()) },
        { "exp",           std::to_string(session.experience()) },
        { "money",         std::to_string(session.money()) },
        { "disguise",      session.disguise().has_value() ? std::to_string(session.disguise().value()).c_str() : "NULL" },
        { "hp",            std::to_string(session.hp()) },
        { "base_hp",       std::to_string(session.base_hp()) },
        { "additional_hp", std::to_string(0) },
        { "mp",            std::to_string(session.mp()) },
        { "base_mp",       std::to_string(session.base_mp()) },
        { "additional_mp", std::to_string(0) },
        { "weapon_color",  "NULL" },
        { "helmet_color",  "NULL" },
        { "armor_color",   session.armor_color().has_value() ? std::to_string(session.armor_color().value()).c_str() : "NULL" },
        { "shield_color",  "NULL" }
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
    std::vector<std::string> itemSet;
    for(auto i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = session.items[i];
        auto master = item != nullptr ? item->based<fb::game::item>() : nullptr;
        auto durability = (item != nullptr) ? item->durability() : std::nullopt;
        auto parameter = std::vector<std::string>
        {
            std::to_string(session.id()),
            std::to_string(i),
            std::to_string(static_cast<int>(fb::game::equipment::slot::UNKNOWN_SLOT)),
            (item == nullptr) ? "NULL" : std::to_string(master->id),
            (item == nullptr) ? "NULL" : std::to_string(item->count()),
            durability.has_value() ? std::to_string(durability.value()) : "NULL",
        };
        itemSet.push_back(fb::query::make_insert(parameter));
    }

    const auto& equipments = session.items.equipments();
    for(auto& [key, value] : equipments)
    {
        auto slot = key;
        auto equipment = value;
        auto master = equipment != nullptr ? equipment->based<fb::game::equipment>() : nullptr;

        auto durability = (equipment != nullptr) ? equipment->durability() : std::nullopt;
        auto parameter = std::vector<std::string>
        {
            std::to_string(session.id()),
            std::to_string(-1),
            std::to_string(static_cast<int>(slot)),
            (equipment == nullptr) ? "NULL" : std::to_string(master->id),
            (equipment == nullptr) ? "NULL" : std::to_string(equipment->count()),
            durability.has_value() ? std::to_string(durability.value()) : "NULL",
        };

        itemSet.push_back(fb::query::make_insert(parameter));
    }

    std::stringstream sstream;
    sstream << "INSERT INTO item (`owner`, `index`, `slot`, `master`, `count`, `durability`) VALUES "
        << boost::algorithm::join(itemSet, ", ")
        << " ON DUPLICATE KEY UPDATE master=VALUES(master), count=VALUES(count), durability=VALUES(durability)";

    return sstream.str();
}

std::string fb::game::query::make_update_spell(fb::game::session& me)
{
    std::vector<std::string> spellSet;
    for(int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto spell = me.spells[i];
        spellSet.push_back(fb::query::make_insert(std::vector<std::string>
        {
            std::to_string(me.id()),
            std::to_string(i),
            (spell == nullptr) ? "NULL" : std::to_string(spell->id()),
        }));
    }

    std::stringstream sstream;
    sstream << "INSERT INTO spell (owner, slot, id) VALUES "
        << boost::algorithm::join(spellSet, ", ")
        << " ON DUPLICATE KEY UPDATE id=VALUES(id)";
    
    return sstream.str();
}
