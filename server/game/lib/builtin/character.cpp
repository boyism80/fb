#include <fb/game/builtin/object.h>
#include <fb/game/builtin/character.h>
#include <fb/game/server.h>
#include <fb/game/marriage.h>
#include <fb/model/datetime.h>
#include <string_view>
#include <unordered_map>

using namespace fb::game;
using table = fb::model::table;

// clang-format off
IMPLEMENT_LUA_EXTENSION(character, "fb.game.character")
{"__eq",                   builtin::object::builtin_eq},
{"uid",                    builtin::character::builtin_uid},
{"look",                   builtin::character::builtin_look},
{"color",                  builtin::character::builtin_color},
{"gender",                 builtin::character::builtin_gender},
{"money",                  builtin::character::builtin_money},
{"exp",                    builtin::character::builtin_exp},
{"item",                   builtin::character::builtin_item},
{"items",                  builtin::character::builtin_items},
{"has_items",              builtin::character::builtin_has_items},
{"equipments",             builtin::character::builtin_equipments},
{"equipment_off",          builtin::character::builtin_equipment_off},
{"dropitem",               builtin::character::builtin_item_drop},
{"mkitem",                 builtin::character::builtin_mkitem},
{"rmitem",                 builtin::character::builtin_rmitem},
{"exchange",                builtin::character::builtin_exchange},
{"state",                  builtin::character::builtin_state},
{"mimic",                  builtin::character::builtin_mimic},
{"class",                  builtin::character::builtin_class},
{"promotion",              builtin::character::builtin_promotion},
{"level",                  builtin::character::builtin_level},
{"assert",                 builtin::character::builtin_assert},
{"role",                   builtin::character::builtin_role},
{"deposited_money",        builtin::character::builtin_deposited_money},
{"stored_item",            builtin::character::builtin_stored_item},
{"store_item",             builtin::character::builtin_store_item},
{"retrieve_item",          builtin::character::builtin_retrieve_item},
{"group",                  builtin::character::builtin_group},
{"create_group",           builtin::character::builtin_create_group},
{"clan",                   builtin::character::builtin_clan},
{"create_clan",            builtin::character::builtin_create_clan},
{"destroy_clan",           builtin::character::builtin_destroy_clan},
{"achievements",           builtin::character::builtin_achievements},
{"achievement",            builtin::character::builtin_achievement},
{"push_achievement",       builtin::character::builtin_push_achievement},
{"erase_achievement",      builtin::character::builtin_erase_achievement},
{"whisper",                builtin::character::builtin_whisper},
{"send_mail",              builtin::character::builtin_send_mail},
{"nation",                 builtin::character::builtin_nation},
{"weapon",                 builtin::character::builtin_weapon},
{"title",                  builtin::character::builtin_title},
{"gain",                   builtin::character::builtin_gain},
{"weapon_damage",          builtin::character::builtin_weapon_damage},
{"detect",                 builtin::character::builtin_detect},
{"spawn_mob",              builtin::character::builtin_spawn_mob},
{"spawned_mobs",           builtin::character::builtin_spawned_mobs},
{"base_hp",                builtin::character::builtin_base_hp},
{"base_mp",                builtin::character::builtin_base_mp},
{"base_str",               builtin::character::builtin_base_str},
{"base_dex",               builtin::character::builtin_base_dex},
{"base_int",               builtin::character::builtin_base_int},
{"base_dam",               builtin::character::builtin_base_dam},
{"base_hit",               builtin::character::builtin_base_hit},
{"armor_color",            builtin::character::builtin_armor_color},
{"mkspell",                builtin::character::builtin_mkspell},
{"rmspell",                builtin::character::builtin_rmspell},
{"spell",                  builtin::character::builtin_spell},
{"world",                  builtin::character::builtin_world},
{"ad",                     builtin::character::builtin_ad},
{"web",                    builtin::character::builtin_web},
{"birthday",               builtin::character::builtin_birthday},
{"active",                 builtin::character::builtin_active},
{"super_hide",             builtin::character::builtin_super_hide},
{"creature",               builtin::character::builtin_creature},
{"teleport",               builtin::character::builtin_teleport},
{"dialog",                 builtin::character::builtin_dialog},
{"list",                   builtin::character::builtin_list},
{"input",                  builtin::character::builtin_input},
{"menu",                   builtin::character::builtin_menu},
{"slot",                   builtin::character::builtin_slot},
{"rezen_force",            builtin::character::builtin_rezen_force},
{"quest",                  builtin::character::builtin_quest},
{"start_quest",            builtin::character::builtin_start_quest},
{"remove_quest",           builtin::character::builtin_remove_quest},
{"reward",                 builtin::character::builtin_reward},
{"send_system_mail",       builtin::character::builtin_send_system_mail},
{"storage_entries",        builtin::character::builtin_storage_entries},
{"receive_storage_reward", builtin::character::builtin_receive_storage_reward},
{"marketplace_list",       builtin::character::builtin_marketplace_list},
{"marketplace_cancel",     builtin::character::builtin_marketplace_cancel},
{"marketplace_purchase",   builtin::character::builtin_marketplace_purchase},
{"marketplace_search",     builtin::character::builtin_marketplace_search},
{"marketplace_get_listings",     builtin::character::builtin_marketplace_get_listings},
{"marketplace_pending_listings", builtin::character::builtin_marketplace_pending_listings},
{"marriage",                     builtin::character::builtin_marriage},
{"marry",                        builtin::character::builtin_marry},
{"divorce",                      builtin::character::builtin_divorce},
END_LUA_EXTENSION; // clang-format on

int builtin::character::builtin_uid(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    ch->assert_thread();

    lua->pushinteger(ch->id);
    return 1;
}

int builtin::character::builtin_look(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto look = ch->look();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(look);
                return 1;
            });
        });
    }
    else
    {
        auto value = (uint16_t)lua->tointeger(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->look(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_color(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto color = ch->color();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(color);
                return 1;
            });
        });
    }
    else
    {
        auto value = (uint8_t)lua->tointeger(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->color(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_gender(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto gender = ch->gender();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(gender);
                return 1;
            });
        });
    }
    else
    {
        auto gender = static_cast<GENDER>(lua->tointeger(2));
        auto weak   = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->gender(gender);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_money(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto money = ch->money();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(money);
                return 1;
            });
        });
    }
    else
    {
        auto value = (uint32_t)lua->tointeger(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->money(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_exp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto exp = ch->exp();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(exp);
                return 1;
            });
        });
    }
    else
    {
        auto value = (uint32_t)lua->tointeger(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->exp(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_item(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (lua->is_number(2))
    {
        auto index = (uint8_t)lua->tointeger(2);
        auto item  = ch->items[index];
        if (item == nullptr)
            lua->pushnil();
        else
            lua->pushobject(item);
        return 1;
    }

    if (lua->is_string(2))
    {
        auto name = lua->tostring(2);
        auto item = ch->items.find(name);
        if (item == nullptr)
            lua->pushnil();
        else
            lua->pushobject(item);
        return 1;
    }

    if (lua->is_userdata<fb::model::item>(2))
    {
        auto model = lua->touserdata<fb::model::item>(2);
        auto item  = ch->items.find(*model);
        if (item == nullptr)
            lua->pushnil();
        else
            lua->pushobject(item);
        return 1;
    }

    // item dialog
    auto oid   = uint32_t{0xFFFFFFFD};
    auto model = (const fb::model::object*)nullptr;
    if (lua->is_userdata<fb::game::object>(2))
    {
        auto obj = lua->touserdata<fb::game::object>(2);
        oid      = obj->oid();
        model    = &obj->based();
    }
    else if (lua->is_userdata<fb::model::object>(2))
    {
        model = lua->touserdata<fb::model::object>(2);
    }
    else
    {
        return 0;
    }

    auto message = lua->tostring(3);
    auto items   = fb::game::dialog::item_pairs();
    if (lua->rawlen(4) > 0)
    {
        lua->pushnil();
        while (lua->next(4))
        {
            // auto i = lua->tointeger(-2);
            auto item  = static_cast<fb::model::item*>(nullptr);
            auto price = uint32_t(0);

            { // get 1st field
                lua->pushinteger(1);
                lua_gettable(L, -2);
                if (lua->is_string(-1))
                {
                    item = table::item.name2item(lua->tostring(-1));
                }
                else if (lua->is_userdata<fb::model::item>(-1))
                {
                    item = lua->touserdata<fb::model::item>(-1);
                }
                else
                {
                }
                lua->pop(1);
            }

            { // get 2nd field
                lua->pushinteger(2);
                lua_gettable(L, -2);
                if (lua->is_number(-1))
                {
                    price = lua->tointeger(-1);
                }
                else
                {
                }
            }
            lua->pop(1);

            if (item == nullptr)
                continue;

            items.push_back({*item, price});
            lua->pop(1);
        }
    }

    ch->listener.on_dialog(*ch, *model, message, items, oid);
    if (ch->dialog != nullptr)
        ch->dialog->release();

    ch->dialog = lua;
    return lua->yield(1);
}

int builtin::character::builtin_items(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    std::optional<std::string> filter_name;
    if (lua->argc() >= 2 && lua->is_string(2))
        filter_name = lua->tostring(2);

    auto weak = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto buffer = std::vector<std::pair<uint8_t, std::shared_ptr<fb::game::item>>>();
        for (int i = 0; i < CONTAINER_CAPACITY; i++)
        {
            if (ch->items[i] == nullptr)
                continue;

            if (filter_name.has_value() && ch->items[i]->based<fb::model::item>().name != *filter_name)
                continue;

            buffer.push_back({i, ch->items[i]->shared_from_this_as<fb::game::item>()});
        }

        return lua->ensure_resume(*server, weak, [=]() {
            lua->new_table();
            for (auto& [index, item] : buffer)
            {
                lua->pushinteger(index + 1);
                lua->pushobject(item);
                lua_settable(L, -3);
            }
            return 1;
        });
    });
}

/**
 * Returns true if the character has at least the required count of each item.
 * Accepts either:
 *   - me:has_items(item_name, count) for a single item, or
 *   - me:has_items({['name1'] = 1, ['name2'] = 5}) for multiple items.
 */
int builtin::character::builtin_has_items(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ch = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    ch->assert_thread();

    auto required = std::vector<std::pair<std::string, uint16_t>>();

    if (lua->is_string(2) && lua->is_number(3))
    {
        auto name = lua->tostring(2);
        auto cnt  = static_cast<uint16_t>(lua->tointeger(3));
        if (!name.empty() && cnt > 0)
            required.push_back({name, cnt});
    }
    else if (lua->is_table(2))
    {
        lua->pushnil();
        while (lua->next(2))
        {
            if (lua->is_string(-2) && lua->is_number(-1))
            {
                auto name = lua->tostring(-2);
                auto cnt  = static_cast<uint16_t>(lua->tointeger(-1));
                if (!name.empty() && cnt > 0)
                    required.push_back({name, cnt});
            }
            lua->pop(1);
        }
    }

    if (required.empty())
    {
        lua->pushboolean(false);
        return 1;
    }

    auto required_models = std::vector<std::pair<const fb::model::item*, uint16_t>>();
    for (const auto& [name, min_count] : required)
    {
        auto model = table::item.name2item(name);
        if (model == nullptr)
        {
            lua->pushboolean(false);
            return 1;
        }
        required_models.push_back({model, min_count});
    }

    lua->pushboolean(ch->items.has(required_models));
    return 1;
}

int builtin::character::builtin_equipments(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto weak = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto buffer = std::vector<std::pair<EQUIPMENT_PARTS, std::shared_ptr<fb::game::item>>>();
        for (auto& [parts, equipment] : ch->items.equipments())
        {
            if (equipment == nullptr)
                continue;

            buffer.push_back({parts, equipment});
        }

        return lua->ensure_resume(*server, weak, [=]() {
            lua->new_table();
            for (auto& [parts, equipment] : buffer)
            {
                lua->pushinteger(parts);
                lua->pushobject(equipment);
                lua_settable(L, -3);
            }
            return 1;
        });
    });
}

int builtin::character::builtin_equipment_off(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto parts = static_cast<fb::model::enum_value::EQUIPMENT_PARTS>(lua->tointeger(2, 0));

    auto weak = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto slot = ch->items.inactive(parts);
        return lua->ensure_resume(*server, weak, [=]() {
            if (slot == 0xFF)
            {
                lua->pushnil();
                return 1;
            }
            auto item = ch->items.at(slot);
            if (item != nullptr)
                lua->pushobject(item);
            else
                lua->pushnil();
            return 1;
        });
    });
}

int builtin::character::builtin_item_drop(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto index    = static_cast<uint8_t>(lua->tointeger(2));
    auto drop_all = lua->toboolean(3);

    auto weak = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto dropped = ch->items.drop(index - 1, drop_all ? 1 : -1);

        return lua->ensure_resume(*server, weak, [=]() {
            if (dropped != nullptr)
                lua->pushobject(dropped);
            else
                lua->pushnil();
            return 1;
        });
    });
}

int builtin::character::builtin_mkitem(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto store = lua->toboolean(4, true);
    if (store == false)
        return builtin::object::builtin_mkitem(L);

    auto weak        = ch->weak_from_this_as<fb::game::character>();
    auto id_to_count = std::unordered_map<uint32_t, uint16_t>();
    auto single_item = !lua->is_table(2);

    if (lua->is_table(2))
    {
        lua->pushnil();
        while (lua->next(2))
        {
            if (lua->is_string(-2) && lua->is_number(-1))
            {
                auto name  = lua->tostring(-2);
                auto count = static_cast<uint16_t>(lua->tointeger(-1));
                auto model = table::item.name2item(name);
                if (model != nullptr && count > 0)
                    id_to_count[model->id] += count;
            }
            lua->pop(1);
        }
    }
    else
    {
        auto name  = lua->tostring(2);
        auto count = static_cast<uint16_t>(lua->tointeger(3, 1));
        if (name.empty())
        {
            lua->pushnil();
            return 1;
        }
        auto model = table::item.name2item(name);
        if (model == nullptr)
        {
            lua->pushnil();
            return 1;
        }
        id_to_count[model->id] = count;
    }

    if (id_to_count.empty())
    {
        lua->pushnil();
        return 1;
    }

    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        if (ch->items.is_rewardable(id_to_count, 0) == false)
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushnil();
                return 1;
            });

        auto items = std::vector<std::shared_ptr<fb::game::item>>();
        for (auto& [id, count] : id_to_count)
        {
            auto& model = table::item[id];
            auto  item  = model.make(*server, count);
            if (item != nullptr)
                items.push_back(std::move(item));
        }
        auto slots = ch->items.add(items, true);

        return lua->ensure_resume(*server, weak, [=]() {
            if (single_item)
            {
                if (slots.empty())
                    lua->pushnil();
                else
                    lua->pushobject(ch->items[slots[0]]);
            }
            else
            {
                lua->new_table();
                for (size_t i = 0; i < slots.size(); i++)
                {
                    lua->pushobject(ch->items[slots[i]]);
                    lua->rawseti(-2, static_cast<int>(i + 1));
                }
            }
            return 1;
        });
    });
}

int builtin::character::builtin_rmitem(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
    {
        lua->pushboolean(false);
        return 1;
    }

    /* Table form: me:rmitem({['name1'] = count, ['name2'] = count, ...}, optional ITEM_DELETE_TYPE). All-or-nothing:
     * remove only if character has all required counts. Returns success (boolean). */
    if (lua->is_table(2))
    {
        auto delete_attr = lua->toenum(3, ITEM_DELETE_TYPE::REMOVED);
        auto to_remove   = std::vector<std::pair<const fb::model::item*, uint16_t>>();
        lua->pushnil();
        while (lua->next(2))
        {
            if (lua->is_string(-2) && lua->is_number(-1))
            {
                auto name  = lua->tostring(-2);
                auto cnt   = static_cast<uint16_t>(lua->tointeger(-1));
                auto model = table::item.name2item(name);
                if (model != nullptr && cnt > 0)
                    to_remove.push_back({model, cnt});
            }
            lua->pop(1);
        }
        if (to_remove.empty())
        {
            lua->pushboolean(false);
            return 1;
        }

        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            bool success = false;
            try
            {
                if (ch->items.has(to_remove))
                {
                    for (const auto& [model, cnt] : to_remove)
                    {
                        auto index   = ch->items.index(*model);
                        auto dropped = ch->items.remove(index, cnt, delete_attr);
                        if (dropped != nullptr)
                            std::ignore = dropped->destroy();
                    }
                    success = true;
                }
            }
            catch (...)
            { }

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushboolean(success);
                return 1;
            });
        });
    }

    auto count       = static_cast<uint8_t>(lua->tointeger(3, 1));
    auto delete_attr = lua->toenum(4, ITEM_DELETE_TYPE::REMOVED);

    if (lua->is_userdata<fb::game::item>(2))
    {
        auto item = lua->touserdata<fb::game::item>(2);
        if (item == nullptr)
        {
            lua->pushboolean(false);
            return 1;
        }

        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            bool success = false;
            try
            {
                auto found = ch->items.find(item->based<fb::model::item>());
                if (found != nullptr && found->count() >= count)
                {
                    auto dropped =
                        ch->items.remove(ch->items.index(item->based<fb::model::item>()), count, delete_attr);
                    if (dropped != nullptr)
                    {
                        std::ignore = dropped->destroy();
                        success     = true;
                    }
                }
            }
            catch (...)
            { }

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushboolean(success);
                return 1;
            });
        });
    }
    else if (lua->is_userdata<fb::model::item>(2))
    {
        auto model = lua->touserdata<fb::model::item>(2);
        if (model == nullptr)
        {
            lua->pushboolean(false);
            return 1;
        }

        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            bool success = false;
            try
            {
                auto found = ch->items.find(*model);
                if (found != nullptr && found->count() >= count)
                {
                    auto index   = ch->items.index(*model);
                    auto dropped = ch->items.remove(index, count, delete_attr);
                    if (dropped != nullptr)
                    {
                        std::ignore = dropped->destroy();
                        success     = true;
                    }
                }
            }
            catch (...)
            { }

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushboolean(success);
                return 1;
            });
        });
    }
    else if (lua->is_number(2))
    {
        auto raw_index = lua->tointeger(2);
        auto index     = static_cast<uint8_t>(raw_index) - 1;

        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            bool success = false;
            try
            {
                auto item = ch->items.at(index);
                if (item != nullptr && item->count() >= count)
                {
                    auto dropped = ch->items.remove(index, count, delete_attr);
                    if (dropped != nullptr)
                    {
                        std::ignore = dropped->destroy();
                        success     = true;
                    }
                }
            }
            catch (...)
            { }

            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushboolean(success);
                return 1;
            });
        });
    }
    else if (lua->is_string(2))
    {
        auto name = lua->tostring(2);
        if (name.empty())
        {
            lua->pushboolean(false);
            return 1;
        }

        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            bool success = false;
            try
            {
                auto model = table::item.name2item(name);
                if (model == nullptr)
                {
                    return lua->ensure_resume(*server, weak, [=]() {
                        lua->pushboolean(false);
                        return 1;
                    });
                }

                auto found = ch->items.find(name);
                if (found != nullptr && found->count() >= count)
                {
                    auto index   = ch->items.index(*model);
                    auto dropped = ch->items.remove(index, count, delete_attr);
                    if (dropped != nullptr)
                    {
                        std::ignore = dropped->destroy();
                        success     = true;
                    }
                }

                return lua->ensure_resume(*server, weak, [=]() {
                    lua->pushboolean(success);
                    return 1;
                });
            }
            catch (...)
            {
                return lua->ensure_resume(*server, weak, [=]() {
                    lua->pushboolean(false);
                    return 1;
                });
            }
        });
    }
    else
    {
        lua->pushboolean(false);
        return 1;
    }
}

int builtin::character::builtin_exchange(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
    {
        lua->pushboolean(false);
        return 1;
    }

    if (lua->is_nil(2) == false && lua->is_table(2) == false)
    {
        lua->pushboolean(false);
        return 1;
    }
    if (lua->is_nil(3) == false && lua->is_table(3) == false)
    {
        lua->pushboolean(false);
        return 1;
    }

    auto cost_items   = std::unordered_map<uint32_t, uint16_t>();
    auto cost_money   = uint32_t{0};
    auto cost_exp     = uint32_t{0};
    auto reward_items = std::unordered_map<uint32_t, uint16_t>();
    auto reward_money = uint32_t{0};
    auto reward_exp   = uint32_t{0};

    if (lua->is_table(2))
    {
        lua->pushstring("item");
        lua->rawget(2);
        if (lua->is_table(-1))
        {
            lua->pushnil();
            while (lua->next(-2))
            {
                if (lua->is_string(-2) && lua->is_number(-1))
                {
                    auto name  = lua->tostring(-2);
                    auto count = static_cast<uint16_t>(lua->tointeger(-1));
                    auto model = table::item.name2item(name);
                    if (model != nullptr && count > 0)
                        cost_items[model->id] += count;
                }
                lua->pop(1);
            }
        }
        lua->pop(1);

        lua->pushstring("money");
        lua->rawget(2);
        if (lua->is_number(-1))
            cost_money = static_cast<uint32_t>(lua->tointeger(-1));
        lua->pop(1);

        lua->pushstring("exp");
        lua->rawget(2);
        if (lua->is_number(-1))
            cost_exp = static_cast<uint32_t>(lua->tointeger(-1));
        lua->pop(1);
    }

    if (lua->is_table(3))
    {
        lua->pushstring("item");
        lua->rawget(3);
        if (lua->is_table(-1))
        {
            lua->pushnil();
            while (lua->next(-2))
            {
                if (lua->is_string(-2) && lua->is_number(-1))
                {
                    auto name  = lua->tostring(-2);
                    auto count = static_cast<uint16_t>(lua->tointeger(-1));
                    auto model = table::item.name2item(name);
                    if (model != nullptr && count > 0)
                        reward_items[model->id] += count;
                }
                lua->pop(1);
            }
        }
        lua->pop(1);

        lua->pushstring("money");
        lua->rawget(3);
        if (lua->is_number(-1))
            reward_money = static_cast<uint32_t>(lua->tointeger(-1));
        lua->pop(1);

        lua->pushstring("exp");
        lua->rawget(3);
        if (lua->is_number(-1))
            reward_exp = static_cast<uint32_t>(lua->tointeger(-1));
        lua->pop(1);
    }

    auto weak = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        exchange_result result = exchange_result::ok;

        if (cost_exp > 0 && ch->exp() < cost_exp)
        {
            result = exchange_result::lack_cost;
        }
        else
        {
            result = ch->items.exchange(cost_items, cost_money, reward_items, reward_money);
            if (result == exchange_result::ok)
            {
                if (cost_exp > 0)
                    ch->reduce_exp(cost_exp);
                if (reward_exp > 0)
                    ch->add_exp(reward_exp, false, true);
            }
        }

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(static_cast<int>(result));
            return 1;
        });
    });
}

int builtin::character::builtin_state(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto state = ch->state();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(state);
                return 1;
            });
        });
    }
    else
    {
        auto value = STATE(lua->tointeger(2));
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->state(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_mimic(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto const& opt = ch->mimicry();
            return lua->ensure_resume(*server, weak, [=]() {
                if (!opt.has_value())
                {
                    lua->pushnil();
                    return 1;
                }
                auto const& p = opt.value();
                lua->new_table();
                if (p.disguise.has_value())
                {
                    lua->pushstring("disguise");
                    lua->pushinteger(static_cast<lua_Integer>(p.disguise.value()));
                    lua->settable(-3);
                }
                lua->pushstring("hair");
                lua->pushinteger(static_cast<lua_Integer>(p.hair));
                lua->settable(-3);
                if (p.hair_color.has_value())
                {
                    lua->pushstring("hair_color");
                    lua->pushinteger(static_cast<lua_Integer>(p.hair_color.value()));
                    lua->settable(-3);
                }
                lua->pushstring("gender");
                lua->pushinteger(static_cast<lua_Integer>(static_cast<uint8_t>(p.gender)));
                lua->settable(-3);
                lua->pushstring("state");
                lua->pushinteger(static_cast<lua_Integer>(static_cast<uint8_t>(p.state)));
                lua->settable(-3);
                if (p.weapon.has_value())
                {
                    lua->pushstring("weapon");
                    lua->pushinteger(static_cast<lua_Integer>(p.weapon.value()));
                    lua->settable(-3);
                }
                if (p.weapon_color.has_value())
                {
                    lua->pushstring("weapon_color");
                    lua->pushinteger(static_cast<lua_Integer>(p.weapon_color.value()));
                    lua->settable(-3);
                }
                if (p.armor.has_value())
                {
                    lua->pushstring("armor");
                    lua->pushinteger(static_cast<lua_Integer>(p.armor.value()));
                    lua->settable(-3);
                }
                if (p.armor_color.has_value())
                {
                    lua->pushstring("armor_color");
                    lua->pushinteger(static_cast<lua_Integer>(p.armor_color.value()));
                    lua->settable(-3);
                }
                if (p.shield.has_value())
                {
                    lua->pushstring("shield");
                    lua->pushinteger(static_cast<lua_Integer>(p.shield.value()));
                    lua->settable(-3);
                }
                if (p.shield_color.has_value())
                {
                    lua->pushstring("shield_color");
                    lua->pushinteger(static_cast<lua_Integer>(p.shield_color.value()));
                    lua->settable(-3);
                }
                return 1;
            });
        });
    }
    else if (argc >= 2)
    {
        if (lua->is_nil(2))
        {
            auto weak = ch->weak_from_this_as<fb::game::character>();
            return lua->ensure_yield(*server, weak, [=](auto is_yield) {
                ch->mimicry(std::nullopt);
                return lua->ensure_resume(*server, weak, [=]() {
                    return 0;
                });
            });
        }
        if (!lua->is_table(2))
            return 0;

        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            fb::game::character_appearance appearance;
            lua->pushstring("disguise");
            lua->rawget(2);
            if (lua->is_number(-1))
                appearance.disguise = static_cast<uint16_t>(lua->tointeger(-1));
            lua->pop(1);

            lua->pushstring("hair");
            lua->rawget(2);
            if (lua->is_number(-1))
                appearance.hair = static_cast<uint16_t>(lua->tointeger(-1));
            lua->pop(1);

            lua->pushstring("hair_color");
            lua->rawget(2);
            if (lua->is_number(-1))
                appearance.hair_color = static_cast<uint8_t>(lua->tointeger(-1));
            lua->pop(1);

            lua->pushstring("gender");
            lua->rawget(2);
            if (lua->is_number(-1))
                appearance.gender = static_cast<GENDER>(lua->tointeger(-1));
            lua->pop(1);

            lua->pushstring("state");
            lua->rawget(2);
            if (lua->is_number(-1))
                appearance.state = static_cast<STATE>(lua->tointeger(-1));
            lua->pop(1);

            lua->pushstring("weapon");
            lua->rawget(2);
            if (lua->is_number(-1))
                appearance.weapon = static_cast<uint16_t>(lua->tointeger(-1));
            lua->pop(1);

            lua->pushstring("weapon_color");
            lua->rawget(2);
            if (lua->is_number(-1))
                appearance.weapon_color = static_cast<uint8_t>(lua->tointeger(-1));
            lua->pop(1);

            lua->pushstring("armor");
            lua->rawget(2);
            if (lua->is_number(-1))
                appearance.armor = static_cast<uint8_t>(lua->tointeger(-1));
            lua->pop(1);

            lua->pushstring("armor_color");
            lua->rawget(2);
            if (lua->is_number(-1))
                appearance.armor_color = static_cast<uint8_t>(lua->tointeger(-1));
            lua->pop(1);

            lua->pushstring("shield");
            lua->rawget(2);
            if (lua->is_number(-1))
                appearance.shield = static_cast<uint8_t>(lua->tointeger(-1));
            lua->pop(1);

            lua->pushstring("shield_color");
            lua->rawget(2);
            if (lua->is_number(-1))
                appearance.shield_color = static_cast<uint8_t>(lua->tointeger(-1));
            lua->pop(1);

            ch->mimicry(std::move(appearance));
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
    return 0;
}

int builtin::character::builtin_class(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto cls = ch->cls();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(cls);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->cls(static_cast<CLASS>(value));
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_promotion(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto promotion = ch->promotion();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(promotion);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->promotion(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_level(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto level = ch->level();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(level);
                return 1;
            });
        });
    }
    else
    {
        auto level = std::max(0, std::min((int)lua->tointeger(2), 255));
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->level(level);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_assert(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    auto size   = lua->rawlen(2);

    if (argc == 1)
    {
        lua->pushboolean(true);
        return 1;
    }

    auto values = std::vector<STATE>();
    if (lua->is_table(2))
    {
        for (int i = 0; i < size; i++)
        {
            lua->rawgeti(2, i + 1);
            if (lua->is_number(-1))
                values.push_back((STATE)lua->tointeger(-1));
        }
    }
    else
    {
        for (auto i = 1; i < argc; i++)
        {
            if (lua->is_number(i + 1))
                values.push_back(static_cast<STATE>(lua->tointeger(i + 1)));
        }
    }

    auto weak = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        try
        {
            ch->assert_state(values);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        }
        catch (std::runtime_error& e)
        {
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushstring(e.what());
                return 1;
            });
        }
    });

    try
    {
        ch->assert_state(values);
        return 0;
    }
    catch (std::runtime_error& e)
    {
        lua->pushstring(e.what());
        return 1;
    }
}

int builtin::character::builtin_role(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto role = ch->role();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(role);
                return 1;
            });
        });
    }
    else
    {
        auto value = static_cast<ROLE>(lua->tointeger(2));
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->role(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_deposited_money(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto deposited = ch->items.deposited();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(deposited);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->items.deposited(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_stored_item(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        // Get all stored items
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            const auto& stored_items = ch->items.stored();

            return lua->ensure_resume(*server, weak, [=]() {
                lua->new_table();
                for (int i = 0; i < stored_items.size(); i++)
                {
                    lua->pushinteger(i + 1);
                    lua->pushobject(stored_items.at(i));
                    lua_settable(L, -3);
                }
                return 1;
            });
        });
    }
    else if (lua->is_number(2))
    {
        // Search by index
        auto index = lua->tointeger(2);

        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            const auto& stored_items = ch->items.stored();
            auto        stored_item  = std::shared_ptr<fb::game::item>();

            if (index <= stored_items.size() - 1)
                stored_item = stored_items.at(index);

            return lua->ensure_resume(*server, weak, [=]() {
                if (stored_item == nullptr)
                    lua->pushnil();
                else
                    lua->pushobject(stored_item);
                return 1;
            });
        });
    }
    else if (lua->is_string(2))
    {
        // Search by name
        auto name = lua->tostring(2);

        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            const auto& stored_items = ch->items.stored();
            auto        found        = std::find_if(stored_items.cbegin(),
                                      stored_items.cend(),
                                      [&name](const std::shared_ptr<fb::game::item>& stored_item) {
                                          return stored_item->based<fb::model::item>().name == name;
                                      });

            auto stored_item = (found != stored_items.cend()) ? *found : nullptr;

            return lua->ensure_resume(*server, weak, [=]() {
                if (stored_item == nullptr)
                    lua->pushnil();
                else
                    lua->pushobject(stored_item);
                return 1;
            });
        });
    }
    else if (lua->is_userdata<fb::game::item>(2))
    {
        // Search by game item
        auto item = lua->touserdata<fb::game::item>(2);

        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            const auto& stored_items = ch->items.stored();
            auto&       model        = item->based<fb::model::item>();
            auto        found        = std::find_if(stored_items.cbegin(),
                                      stored_items.cend(),
                                      [&model](const std::shared_ptr<fb::game::item>& stored_item) {
                                          return stored_item->based<fb::model::item>() == model;
                                      });

            auto stored_item = (found != stored_items.cend()) ? *found : nullptr;

            return lua->ensure_resume(*server, weak, [=]() {
                if (stored_item == nullptr)
                    lua->pushnil();
                else
                    lua->pushobject(stored_item);
                return 1;
            });
        });
    }
    else if (lua->is_userdata<fb::model::item>(2))
    {
        // Search by model item
        auto model = lua->touserdata<fb::model::item>(2);

        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            const auto& stored_items = ch->items.stored();
            auto        found        = std::find_if(stored_items.cbegin(),
                                      stored_items.cend(),
                                      [model](const std::shared_ptr<fb::game::item>& stored_item) {
                                          return stored_item->based<fb::model::item>() == *model;
                                      });

            auto stored_item = (found != stored_items.cend()) ? *found : nullptr;

            return lua->ensure_resume(*server, weak, [=]() {
                if (stored_item == nullptr)
                    lua->pushnil();
                else
                    lua->pushobject(stored_item);
                return 1;
            });
        });
    }
    else
    {
        return 0;
    }
}

int builtin::character::builtin_store_item(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto item  = lua->touserdata<fb::game::item>(2);
    auto count = lua->tointeger(3, 1);

    // Find the shared_ptr that contains this item
    auto weak = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto index   = ch->items.index(item);
        auto success = false;
        if (index != 0xFF)
            success = ch->items.store(index, count);

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushboolean(success);
            return 1;
        });
    });
}

int builtin::character::builtin_retrieve_item(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto item  = lua->touserdata<fb::game::item>(2);
    auto count = lua->tointeger(3, 1);

    auto weak = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto& stored_items = ch->items.stored();
        auto  returned     = std::shared_ptr<fb::game::item>();

        for (size_t i = 0; i < stored_items.size(); ++i)
        {
            if (stored_items[i] == item)
            {
                returned = ch->items.retrieve(static_cast<uint8_t>(i), count);
                break;
            }
        }

        return lua->ensure_resume(*server, weak, [=]() {
            if (returned == nullptr)
                lua->pushnil();
            else
                lua->pushobject(returned);
            return 1;
        });
    });
}

int builtin::character::builtin_group(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc < 2)
    {
        static auto fn = [](fb::game::server*                  server,
                            std::weak_ptr<fb::game::character> ch,
                            fb::lua::context*                  lua) -> async::task<void> {
            auto shared = ch.lock();
            if (shared == nullptr)
            {
                co_await lua->switching();
                lua->pushnil();
                lua->resume(1);
                co_return;
            }

            auto group_id = shared->group_id();
            if (group_id.has_value() == false)
            {
                co_await lua->switching();
                lua->pushnil();
                lua->resume(1);
            }
            else
            {
                server->groups.read(group_id.value(), [=](const auto& group) {
                    lua->pushobject(group);
                    lua->resume(1);
                });
            }
            co_return;
        };

        auto weak = ch->weak_from_this_as<fb::game::character>();
        server->threads.enqueue(weak, [=](auto&) -> async::task<void> {
            co_await fn(server, weak, lua);
        });
        return lua->yield(1);
    }
    else if (lua->is_function(2))
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto group_id = ch->group_id();

            return lua->ensure_resume(*server, weak, [=]() {
                static auto static_func = [](fb::lua::context* server) {
                    lua_call(*server, 2, LUA_MULTRET);
                    server->remove(-server->argc());
                    return server->argc();
                };

                lua->pushobject(ch);
                if (group_id.has_value() == false)
                {
                    lua->pushnil();
                    return static_func(lua);
                }
                else
                {
                    return server->groups.read(group_id.value(), [=](const auto& group) {
                        lua->pushobject(group);
                        return static_func(lua);
                    });
                }
            });
        });
    }
    else
    {
        return 0;
    }
}

int builtin::character::builtin_create_group(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto name = lua->tostring(2);

    static auto fn = [](fb::game::server*                  server,
                        std::weak_ptr<fb::game::character> ch,
                        fb::lua::context*                  lua,
                        std::string                        target) -> async::task<void> {
        auto shared = ch.lock();
        if (shared == nullptr)
        {
            co_await lua->switching();
            lua->pushnil();
            lua->resume(1);
            co_return;
        }

        try
        {
            co_await server->create_group(*shared, target);
            co_await lua->switching();
            lua->pushboolean(true);
            lua->resume(1);
        }
        catch (std::exception& e)
        {
            async::awaitable_then(lua->switching(), [lua](auto result) {
                result();
                lua->pushboolean(false);
                lua->resume(1);
            });
        }
    };

    auto weak = ch->weak_from_this_as<fb::game::character>();
    server->threads.enqueue(weak, [=](auto&) -> async::task<void> {
        co_await fn(server, ch, lua, name);
    });

    return lua->yield(1);
}

int builtin::character::builtin_clan(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc < 2)
    {
        static auto fn = [](fb::game::server*                  server,
                            std::weak_ptr<fb::game::character> ch,
                            fb::lua::context*                  lua) -> async::task<void> {
            auto shared = ch.lock();
            if (shared == nullptr)
            {
                co_await lua->switching();
                lua->pushnil();
                lua->resume(1);
                co_return;
            }

            auto& clan_id = shared->clan_id();
            if (clan_id.has_value() == false)
            {
                co_await lua->switching();
                lua->pushnil();
                lua->resume(1);
            }
            else
            {
                async::awaitable_then(lua->switching(), [lua, server, cid = clan_id.value()](auto result) {
                    result();
                    server->clans.read(cid, [=](const auto& clan) {
                        lua->pushobject(clan);
                        lua->resume(1);
                    });
                });
            }
            co_return;
        };

        auto weak = ch->weak_from_this_as<fb::game::character>();
        server->threads.enqueue(weak, [=](auto&) -> async::task<void> {
            co_await fn(server, ch, lua);
        });
        return lua->yield(1);
    }
    else if (lua->is_function(2))
    {
        static auto fn = [](fb::game::server*                  server,
                            std::weak_ptr<fb::game::character> ch,
                            fb::lua::context*                  lua) -> async::task<void> {
            auto shared = ch.lock();
            if (shared == nullptr)
            {
                co_await lua->switching();
                lua->pushnil();
                lua->resume(1);
                co_return;
            }

            auto& clan_id = shared->clan_id();
            if (clan_id.has_value() == false)
            {
                co_await lua->switching();
                lua->pushnil();
                lua->resume(1);
            }
            else
            {
                async::awaitable_then(lua->switching(), [lua, server, cid = clan_id.value()](auto result) {
                    result();
                    server->clans.read(cid, [=](const auto& clan) {
                        lua->pushobject(clan);
                        lua->resume(1);
                    });
                });
            }
        };

        auto weak = ch->weak_from_this_as<fb::game::character>();
        server->threads.enqueue(weak, [=](auto&) -> async::task<void> {
            co_await fn(server, ch, lua);
        });

        return lua->yield(1);
    }
    else
    {
        return 0;
    }
}

int builtin::character::builtin_create_clan(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto name = lua->tostring(2);

    static auto fn = [](fb::game::server*                  server,
                        std::weak_ptr<fb::game::character> ch,
                        fb::lua::context*                  lua,
                        const std::string&                 name) -> async::task<void> {
        auto shared = ch.lock();
        if (shared == nullptr)
        {
            co_await lua->switching();
            lua->pushnil();
            lua->resume(1);
            co_return;
        }
        try
        {
            co_await server->create_clan(*shared, name);
            co_await lua->switching();
            lua->pushnil();
            lua->resume(1);
        }
        catch (std::exception& e)
        {
            auto what = std::string(e.what());
            async::awaitable_then(lua->switching(), [lua, what](auto result) {
                result();
                lua->pushstring(what.c_str());
                lua->resume(1);
            });
        }
        co_return;
    };

    auto weak = ch->weak_from_this_as<fb::game::character>();
    server->threads.enqueue(weak, [=](auto&) -> async::task<void> {
        co_await fn(server, ch, lua, name);
        co_return;
    });

    return lua->yield(1);
}

int builtin::character::builtin_destroy_clan(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    // Static function to isolate async operation and ensure parameter lifetime safety
    static auto static_func = [](fb::game::server*                  server,
                                 std::weak_ptr<fb::game::character> ch,
                                 fb::lua::context*                  lua) -> async::task<void> {
        auto shared = ch.lock();
        if (shared == nullptr)
        {
            co_await lua->switching();
            lua->pushnil();
            lua->resume(1);
            co_return;
        }
        try
        {
            co_await server->destroy_clan(*shared);
            co_await lua->switching();
            lua->pushnil();
            lua->resume(1);
            co_return;
        }
        catch (std::exception& e)
        {
            auto what = std::string(e.what());
            async::awaitable_then(lua->switching(), [lua, what](auto result) {
                result();
                lua->pushstring(what.c_str());
                lua->resume(1);
            });
            co_return;
        }
    };

    auto weak = ch->weak_from_this_as<fb::game::character>();
    server->threads.enqueue(weak, [server, weak, lua](auto&) -> async::task<void> {
        co_await static_func(server, weak, lua);
    });

    return lua->yield(1);
}

int builtin::character::builtin_achievements(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto weak = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto buffer = std::vector<std::pair<uint32_t, fb::game::achievement*>>();
        for (auto& [id, achievement] : ch->achievements)
        {
            buffer.push_back({id, achievement.get()});
        }

        return lua->ensure_resume(*server, weak, [=]() {
            lua->new_table();
            auto i = 0;
            for (auto& [id, achievement] : buffer)
            {
                lua->pushinteger(i + 1);
                lua->pushobject(*achievement);
                lua_settable(L, -3);
                i++;
            }
            return 1;
        });
    });

    return 1;
}

int builtin::character::builtin_achievement(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto i    = lua->tointeger(2);
    auto weak = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto achievement = static_cast<fb::game::achievement*>(nullptr);
        if (ch->achievements.contains(i))
            achievement = ch->achievements.at(i).get();

        return lua->ensure_resume(*server, weak, [=]() {
            if (achievement == nullptr)
                lua->pushnil();
            else
                lua->pushobject(*achievement);

            return 1;
        });
    });
}

int builtin::character::builtin_push_achievement(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (!lua->is_number(2) || !lua->is_string(3) || !lua->is_number(4) || !lua->is_number(5))
        return 0;

    auto id    = static_cast<uint32_t>(lua->tointeger(2));
    auto text  = lua->tostring(3);
    auto icon  = static_cast<uint8_t>(lua->tointeger(4));
    auto color = static_cast<uint16_t>(lua->tointeger(5));

    auto weak = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        ch->achievements[id] = std::make_unique<fb::game::achievement>(id, text, icon, color);

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushobject(*ch->achievements[id]);
            return 1;
        });
    });
}

int builtin::character::builtin_erase_achievement(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto i    = lua->tointeger(2);
    auto weak = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto success = false;
        if (ch->achievements.contains(i))
        {
            ch->achievements.erase(i);
            success = true;
        }

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushboolean(success);
            return 1;
        });
    });
}

int builtin::character::builtin_whisper(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto to      = lua->tostring(2);
    auto message = lua->tostring(3);

    static auto fn = [](fb::game::server*                  server,
                        fb::lua::context*                  lua,
                        std::weak_ptr<fb::game::character> ch,
                        const std::string&                 to,
                        const std::string&                 message) -> async::task<void> {
        auto shared = ch.lock();
        if (shared == nullptr)
        {
            co_await lua->switching();
            lua->pushnil();
            lua->resume(1);
            co_return;
        }

        try
        {
            co_await server->whisper(*shared, to, message);
            lua->pushnil();
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
        }

        lua->resume(1);
    };

    auto weak = ch->weak_from_this_as<fb::game::character>();
    server->threads.enqueue(weak, [=](auto&) -> async::task<void> {
        co_await fn(server, lua, ch, to, message);
    });

    return lua->yield(1);
}

int builtin::character::builtin_nation(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto nation = ch->nation();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(static_cast<uint8_t>(nation));
                return 1;
            });
        });
    }
    else
    {
        auto value = static_cast<NATION>(lua->tointeger(2));
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto success = ch->nation(value);
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushboolean(success);
                return 1;
            });
        });
    }
}

int builtin::character::builtin_weapon(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto weapon = ch->items.weapon();
            return lua->ensure_resume(*server, weak, [=]() {
                if (weapon == nullptr)
                    lua->pushnil();
                else
                    lua->pushobject(weapon);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->touserdata<fb::game::weapon>(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->items.weapon(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_title(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto title = ch->title();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushstring(title);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tostring(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->title(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_gain(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto items = std::vector<std::shared_ptr<fb::game::item>>{};
    for (int i = 2; i <= argc; i++)
    {
        auto item = lua->touserdata<fb::game::item>(i);
        items.push_back(item);
    }

    auto weak = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        ch->items.add(items, true);
        return lua->ensure_resume(*server, weak, [=]() {
            return 0;
        });
    });
}

int builtin::character::builtin_weapon_damage(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto weapon_damage = ch->weapon_damage();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(weapon_damage);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->weapon_damage(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_detect(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto detect = ch->detect();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushboolean(detect);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->toboolean(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->detect(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_spawn_mob(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto map = ch->map();
    if (map == nullptr)
    {
        lua->pushnil();
        return 1;
    }

    auto name  = lua->tostring(2);
    auto owned = true;
    auto model = table::mob.name2mob(name);
    if (model == nullptr)
    {
        lua->pushnil();
        return 1;
    }

    uint16_t x, y;
    uint16_t offset = 3;
    if (argc > 3)
    {
        if (lua->is_table(3))
        {
            if (argc >= 4)
            {
                owned   = lua->toboolean(4);
                offset += 1;
            }

            lua->rawgeti(3, 1);
            x = (uint16_t)lua->tointeger(-1);
            lua->remove(-1);

            lua->rawgeti(3, 2);
            y = (uint16_t)lua->tointeger(-1);
            lua->remove(-1);

            offset += 1;
        }
        else
        {
            if (argc >= 5)
            {
                owned   = lua->toboolean(5);
                offset += 1;
            }

            x = (uint16_t)lua->tointeger(3);
            y = (uint16_t)lua->tointeger(4);

            offset += 2;
        }
    }

    auto notify = lua->toboolean(offset, true);
    auto weak   = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) mutable {
        if (argc < 3)
        {
            x = ch->x();
            y = ch->y();
        }

        auto mob = ch->spawn_mob(*model, fb::model::point16_t(x, y), owned, notify);
        return lua->ensure_resume(*server, weak, [=]() {
            if (mob == nullptr)
                lua->pushnil();
            else
                lua->pushobject(mob);
            return 1;
        });
    });
}

int builtin::character::builtin_spawned_mobs(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto weak = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto& spawned_mobs = ch->spawned_mobs();

        return lua->ensure_resume(*server, weak, [=]() {
            lua->new_table();
            for (int i = 0; i < spawned_mobs.size(); i++)
            {
                lua->pushinteger(i + 1);
                lua->pushobject(spawned_mobs[i]);
                lua_settable(L, -3);
            }

            return 1;
        });
    });
}

int builtin::character::builtin_base_hp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto base_hp = ch->stat.base_hp();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(base_hp);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->stat.base_hp(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_base_mp(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto base_mp = ch->stat.base_mp();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(base_mp);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->stat.base_mp(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_base_str(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto base_str = ch->stat.base_str();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(base_str);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->stat.base_str(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_base_dex(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto base_dex = ch->stat.base_dex();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(base_dex);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->stat.base_dex(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_base_int(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto base_int = ch->stat.base_int();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(base_int);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->stat.base_int(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_base_dam(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto base_dam = ch->stat.base_dam();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(base_dam);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->stat.base_dam(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_base_hit(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto base_hit = ch->stat.base_hit();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(base_hit);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->stat.base_hit(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_armor_color(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto result = std::optional<uint16_t>{};
            auto color  = ch->armor_color();
            if (color.has_value())
                result = color.value();

            return lua->ensure_resume(*server, weak, [=]() {
                if (result.has_value())
                    lua->pushinteger(result.value());
                else
                    lua->pushnil();
                return 1;
            });
        });
    }
    else
    {
        auto value = std::optional<uint16_t>{};
        if (lua->is_number(2))
            value = lua->tointeger(2);

        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->armor_color(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_mkspell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto name  = lua->tostring(2);
    auto model = table::spell.name2spell(name);
    if (model == nullptr)
        return 0;

    auto weak = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto slot = ch->spells.add(*model);
        if (slot == 0xFF)
            return 0;

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushinteger(slot);
            return 1;
        });
    });
}

int builtin::character::builtin_rmspell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            for (int i = 0; i < CONTAINER_CAPACITY; i++)
            {
                auto spell = ch->spells[i];
                if (spell == nullptr)
                    continue;

                ch->spells.remove(i);
            }

            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
    else if (lua->is_number(2))
    {
        auto slot = lua->tointeger(2);
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->spells.remove(slot);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
    else if (lua->is_string(2))
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto name = lua->tostring(2);
            auto slot = uint8_t{0xFF};
            for (int i = 0; i < CONTAINER_CAPACITY; i++)
            {
                auto spell = ch->spells[i];
                if (spell == nullptr)
                    continue;

                if (spell->model.name == name)
                {
                    slot = i;
                    break;
                }
            }

            if (slot != 0xFF)
                ch->spells.remove(slot);

            auto weak = ch->weak_from_this_as<fb::game::character>();
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
    else
    {
        return 0;
    }
}

int builtin::character::builtin_spell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ch = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto name = lua->tostring(2);
    if (name.empty())
    {
        lua->pushnil();
        return 1;
    }

    auto model = table::spell.name2spell(name);
    if (model == nullptr)
    {
        lua->pushnil();
        return 1;
    }

    auto spell = ch->spells.find(*model);
    if (spell == nullptr)
        lua->pushnil();
    else
        lua->pushobject(spell);

    return 1;
}

int builtin::character::builtin_world(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)

        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto name = lua->tostring(2);
    for (auto& [id, world] : table::world)
    {
        for (auto& [index, point] : world)
        {
            if (point.name == name)
            {
                auto weak = ch->weak_from_this_as<fb::game::character>();
                return lua->ensure_yield(*server, weak, [=](auto is_yield) {
                    ch->show_world_map(id, index);
                    return lua->ensure_resume(*server, weak, [=]() {
                        lua->pushboolean(true);
                        return 1;
                    });
                });
            }
        }
    }

    lua->pushboolean(false);
    return 1;
}

int builtin::character::builtin_ad(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto width  = lua->tointeger(2, 300);
    auto height = lua->tointeger(3, 120);
    auto url    = lua->tostring(4, "http://www.google.com");
    auto time   = lua->tointeger(5, 60);
    ch->send(fb::protocol::game::response::ad(width, height, url, time));
    return 0;
}

int builtin::character::builtin_web(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto type    = lua->tointeger(2, 0);
    auto url     = lua->tostring(3, "http://www.google.com");
    auto message = lua->tostring(4, "default message");
    ch->send(fb::protocol::game::response::web(type, url, message));
    return 0;
}

int builtin::character::builtin_delay(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto model = (const fb::model::spell*)nullptr;
    if (lua->is_userdata<fb::model::spell>(2))
    {
        model = lua->touserdata<fb::model::spell>(2);
    }
    else if (lua->is_userdata<fb::game::spell>(2))
    {
        auto spell = lua->touserdata<fb::game::spell>(2);
        if (spell != nullptr)
            model = &spell->model;
    }
    else if (lua->is_string(2))
    {
        model = table::spell.name2spell(lua->tostring(2));
    }

    if (model == nullptr)
        return 0;

    auto value = lua->tointeger(3);
    auto spell = ch->spells.find(*model);
    if (spell == nullptr)
        return 0;

    spell->delay(value);
    return 0;
}

int builtin::character::builtin_birthday(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto result = std::optional<std::uint32_t>{};
            if (ch->birthday().has_value())
                result = ch->birthday().value();

            return lua->ensure_resume(*server, weak, [=]() {
                if (result.has_value())
                    lua->pushinteger(result.value());
                else
                    lua->pushnil();
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->tointeger(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->birthday(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_active(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto item = lua->touserdata<fb::game::item>(2);
    if (item == nullptr)
        return 0;

    auto slot = ch->items.index(item);
    if (slot == 0xFF)
        return 0;

    ch->items.active(slot);
    return 0;
}

int builtin::character::builtin_super_hide(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto result = ch->super_hide();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushboolean(result);
                return 1;
            });
        });
    }
    else
    {
        auto value = lua->toboolean(2);
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            ch->super_hide(value);
            return lua->ensure_resume(*server, weak, [=]() {
                return 0;
            });
        });
    }
}

int builtin::character::builtin_send_mail(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto to    = lua->tostring(2);
    auto title = std::string{"mail title"};
    if (argc >= 3 && lua->is_nil(3) == false)
        title = lua->tostring(3);

    auto contents = std::string{"mail contents"};
    if (argc >= 4 && lua->is_nil(4) == false)
        contents = lua->tostring(4);

    auto weak = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        std::ignore = server->send_mail(*ch, to, title, contents);
        return lua->ensure_resume(*server, weak, [=]() {
            return 0;
        });
    });
}

int builtin::character::builtin_creature(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (argc == 1)
    {
        auto weak = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto creature = ch->creature();
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushinteger(static_cast<uint8_t>(creature));
                return 1;
            });
        });
    }
    else
    {
        auto value = static_cast<CREATURE>(lua->tointeger(2));
        auto weak  = ch->weak_from_this_as<fb::game::character>();
        return lua->ensure_yield(*server, weak, [=](auto is_yield) {
            auto success = ch->creature(value);
            return lua->ensure_resume(*server, weak, [=]() {
                lua->pushboolean(success);
                return 1;
            });
        });
    }
}

int fb::game::builtin::character::builtin_teleport(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;
    auto ch_weak = ch->weak_from_this_as<fb::game::character>();

    auto target = lua->touserdata<fb::game::character>(2);
    if (target == nullptr)
        return 0;
    auto target_weak = target->weak_from_this_as<fb::game::character>();

    if (ch->thread() == target->thread())
    {
        auto map    = target->map();
        std::ignore = ch->map(map, target->position());
        lua->pushboolean(true);
        return 1;
    }
    else
    {
        server->threads.enqueue(target_weak, [=](auto& thread) -> async::task<void> {
            auto map = target->map();
            if (map == nullptr)
            {
                lua->release();
                co_return;
            }
            auto position = target->position();

            server->threads.enqueue(ch_weak, [=](auto& thread) -> async::task<void> {
                async::awaitable_then(ch->map(map, position), [=](auto result) {
                    auto success = result();
                    auto weak    = ch->weak_from_this_as<fb::game::character>();
                    lua->ensure_resume(*server, weak, [=]() {
                        lua->pushboolean(success);
                        return 1;
                    });
                });
                co_return;
            });
        });

        lua->yield(0);
        return 0;
    }
}

int fb::game::builtin::character::builtin_dialog(lua_State* L)
{
    // Ex) ch:dialog(obj, "hello", true, true);

    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto argc = lua->argc();
    if (argc < 3)
        throw std::runtime_error("not enough parameters");

    auto oid   = uint32_t{0xFFFFFFFD};
    auto obj   = std::shared_ptr<fb::game::object>(nullptr);
    auto model = static_cast<const fb::model::object*>(nullptr);
    if (lua->is_userdata<fb::game::object>(2))
    {
        obj   = lua->touserdata<fb::game::object>(2);
        oid   = obj->oid();
        model = &obj->based();
    }
    else if (lua->is_userdata<fb::model::object>(2))
    {
        model = lua->touserdata<fb::model::object>(2);
    }
    else if (lua->is_nil(2))
    {
        model = nullptr;
    }
    else
    {
        return 0;
    }

    auto message     = lua->tostring(3);
    auto button_prev = lua->toboolean(4, false);
    auto button_next = lua->toboolean(5, false);

    std::ignore =
        server->threads.dispatch(ch->weak_from_this_as<fb::game::character>(), [=](auto& thread) -> async::task<void> {
            if (obj != nullptr)
                ch->listener.on_dialog(*ch, *obj, message, button_prev, button_next, oid);
            else if (model != nullptr)
                ch->listener.on_dialog(*ch, *model, message, button_prev, button_next, oid);
            else
                ch->listener.on_dialog(*ch, message, button_prev, button_next, oid);

            if (ch->dialog != nullptr)
                ch->dialog->release();

            ch->dialog = lua;
            co_return;
        });

    return lua->yield(1);
}

int fb::game::builtin::character::builtin_list(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto oid        = uint32_t{0xFFFFFFFD};
    auto obj        = std::shared_ptr<fb::game::object>(nullptr);
    auto model      = static_cast<const fb::model::object*>(nullptr);
    auto appearance = static_cast<fb::game::character_appearance*>(nullptr);
    if (lua->is_userdata<fb::game::object>(2))
    {
        obj   = lua->touserdata<fb::game::object>(2);
        oid   = obj->oid();
        model = &obj->based();
    }
    else if (lua->is_userdata<fb::model::object>(2))
    {
        model = lua->touserdata<fb::model::object>(2);
    }
    else if (lua->is_table(2))
    {
        appearance = new fb::game::character_appearance();
        lua->pushstring("gender");
        if (lua_rawget(L, 2) == LUA_TNUMBER)
            appearance->gender = static_cast<GENDER>(lua->tointeger(-1));

        lua->pushstring("state");
        if (lua_rawget(L, 2) == LUA_TNUMBER)
            appearance->state = static_cast<STATE>(lua->tointeger(-1));

        lua->pushstring("hair");
        if (lua_rawget(L, 2) == LUA_TNUMBER)
            appearance->hair = lua->tointeger(-1);

        lua->pushstring("hair_color");
        if (lua_rawget(L, 2) == LUA_TNUMBER)
            appearance->hair_color = lua->tointeger(-1);

        lua->pushstring("weapon");
        if (lua_rawget(L, 2) == LUA_TNUMBER)
            appearance->weapon = lua->tointeger(-1);

        lua->pushstring("weapon_color");
        if (lua_rawget(L, 2) == LUA_TNUMBER)
            appearance->weapon_color = lua->tointeger(-1);

        lua->pushstring("armor");
        if (lua_rawget(L, 2) == LUA_TNUMBER)
            appearance->armor = lua->tointeger(-1);

        lua->pushstring("armor_color");
        if (lua_rawget(L, 2) == LUA_TNUMBER)
            appearance->armor_color = lua->tointeger(-1);

        lua->pushstring("shield");
        if (lua_rawget(L, 2) == LUA_TNUMBER)
            appearance->shield = lua->tointeger(-1);

        lua->pushstring("shield_color");
        if (lua_rawget(L, 2) == LUA_TNUMBER)
            appearance->shield_color = lua->tointeger(-1);
    }
    else
    {
        return 0;
    }

    auto message     = lua->tostring(3);
    auto size        = lua->rawlen(4);
    auto button_prev = lua->toboolean(5);

    auto menus = std::vector<std::string>();
    for (int i = 0; i < size; i++)
    {
        lua->rawgeti(4, i + 1);
        menus.push_back(lua->tostring(-1));
    }

    std::ignore =
        server->threads.dispatch(ch->weak_from_this_as<fb::game::character>(), [=](auto& thread) -> async::task<void> {
            if (appearance != nullptr)
                ch->listener.on_dialog(*ch,
                                       std::unique_ptr<fb::game::appearance>(appearance),
                                       message,
                                       menus,
                                       button_prev,
                                       oid);
            else if (obj != nullptr)
                ch->listener.on_dialog(*ch, *obj, message, menus, button_prev, oid);
            else
                ch->listener.on_dialog(*ch, *model, message, menus, button_prev, oid);

            if (ch->dialog != nullptr)
                ch->dialog->release();

            ch->dialog = lua;
            co_return;
        });

    return lua->yield(1);
}

int fb::game::builtin::character::builtin_input(lua_State* L)
{
    // Ex) obj::input(ch, "message")
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto oid   = uint32_t{0xFFFFFFFD};
    auto model = static_cast<const fb::model::object*>(nullptr);
    auto obj   = std::shared_ptr<fb::game::object>(nullptr);
    if (lua->is_userdata<fb::game::object>(2))
    {
        obj   = lua->touserdata<fb::game::object>(2);
        oid   = obj->oid();
        model = &obj->based();
    }
    else if (lua->is_userdata<fb::model::object>(2))
    {
        model = lua->touserdata<fb::model::object>(2);
    }
    else
    {
        return 0;
    }

    auto message = lua->tostring(3);
    auto argc    = lua->argc();
    if (argc > 3)
    {
        auto message_top = lua->tostring(4);
        auto message_bot = lua->tostring(5);
        auto maxlen      = (uint8_t)lua->tointeger(6, 0xFF);
        auto prev        = lua->toboolean(7, false);

        std::ignore = server->threads.dispatch(
            ch->weak_from_this_as<fb::game::character>(),
            [=](auto& thread) -> async::task<void> {
                ch->listener.on_dialog(*ch, *model, message, message_top, message_bot, maxlen, prev, oid);
                if (ch->dialog != nullptr)
                    ch->dialog->release();

                ch->dialog = lua;
                co_return;
            });
    }
    else
    {
        std::ignore = server->threads.dispatch(ch->weak_from_this_as<fb::game::character>(),
                                               [=](auto& thread) -> async::task<void> {
                                                   if (obj != nullptr)
                                                       ch->listener.on_dialog(*ch, *obj, message, oid);
                                                   else
                                                       ch->listener.on_dialog(*ch, *model, message, oid);

                                                   if (ch->dialog != nullptr)
                                                       ch->dialog->release();

                                                   ch->dialog = lua;
                                                   co_return;
                                               });
    }

    return lua->yield(1);
}

int fb::game::builtin::character::builtin_menu(lua_State* L)
{
    // Ex) obj::menu(ch, "hello", {"hello 1", "hello 2", "hello 3"})
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto oid   = uint32_t{0xFFFFFFFD};
    auto model = static_cast<const fb::model::object*>(nullptr);
    auto obj   = std::shared_ptr<fb::game::object>(nullptr);
    if (lua->is_userdata<fb::game::object>(2))
    {
        obj   = lua->touserdata<fb::game::object>(2);
        oid   = obj->oid();
        model = &obj->based();
    }
    else if (lua->is_userdata<fb::model::object>(2))
    {
        model = lua->touserdata<fb::model::object>(2);
    }
    else
    {
        return 0;
    }

    auto message = lua->tostring(3);

    // Read menu list
    auto size  = lua->rawlen(4);
    auto menus = std::vector<std::string>();
    for (int i = 0; i < size; i++)
    {
        lua->rawgeti(4, i + 1);
        menus.push_back(lua->tostring(-1));
    }

    std::ignore =
        server->threads.dispatch(ch->weak_from_this_as<fb::game::character>(), [=](auto& thread) -> async::task<void> {
            if (obj != nullptr)
                ch->listener.on_dialog(*ch, *obj, message, menus, oid);
            else
                ch->listener.on_dialog(*ch, *model, message, menus, oid);

            if (ch->dialog != nullptr)
                ch->dialog->release();

            ch->dialog = lua;
            co_return;
        });

    return lua->yield(1);
}

int fb::game::builtin::character::builtin_slot(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto oid   = uint32_t{0xFFFFFFFD};
    auto model = static_cast<const fb::model::object*>(nullptr);
    auto obj   = std::shared_ptr<fb::game::object>(nullptr);
    if (lua->is_userdata<fb::game::object>(2))
    {
        obj   = lua->touserdata<fb::game::object>(2);
        oid   = obj->oid();
        model = &obj->based();
    }
    else if (lua->is_userdata<fb::model::object>(2))
    {
        model = lua->touserdata<fb::model::object>(2);
    }
    else
    {
        return 0;
    }

    auto message = lua->tostring(3);
    auto slots   = std::vector<uint8_t>();
    auto size    = lua_rawlen(L, 4);
    for (int i = 0; i < size; i++)
    {
        lua->pushinteger(i + 1);
        lua_gettable(L, 4);
        slots.push_back(lua->tointeger(-1));
        lua->pop(1);
    }

    std::ignore =
        server->threads.dispatch(ch->weak_from_this_as<fb::game::character>(), [=](auto& thread) -> async::task<void> {
            if (obj != nullptr)
                ch->listener.on_dialog(*ch, *obj, message, slots, oid);
            else
                ch->listener.on_dialog(*ch, *model, message, slots, oid);

            if (ch->dialog != nullptr)
                ch->dialog->release();

            ch->dialog = lua;
            co_return;
        });

    return lua->yield(1);
}

int fb::game::builtin::character::builtin_rezen_force(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto map = ch->map();
    if (map == nullptr)
        return 0;

    auto is_global = lua->toboolean(2, false);
    if (is_global)
        server->rezen_force();
    else
        server->rezen_force(*map);

    return 0;
}

int fb::game::builtin::character::builtin_quest(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (lua->argc() < 2)
    {
        lua->pushnil();
        return 1;
    }

    auto id = lua->tointeger(2);
    if (ch->quests.contains(id) == false)
        lua->pushnil();
    else
        lua->pushobject(ch->quests[id]);

    return 1;
}

int fb::game::builtin::character::builtin_start_quest(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto id = lua->tointeger(2);

    if (ch->quests.start(id) == false)
    {
        lua->pushnil();
        return 1;
    }

    if (ch->quests.contains(id) == false)
        lua->pushnil();
    else
        lua->pushobject(ch->quests[id]);

    return 1;
}

int fb::game::builtin::character::builtin_remove_quest(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (lua->argc() < 2)
    {
        lua->pushboolean(false);
        return 1;
    }

    if (lua->is_number(2))
    {
        auto id = lua->tointeger(2);
        lua->pushboolean(ch->quests.remove(id));
        return 1;
    }
    else if (lua->is_userdata<fb::game::quest>(2))
    {
        auto quest = lua->touserdata<fb::game::quest>(2);
        lua->pushboolean(ch->quests.remove(quest->id));
        return 1;
    }
    else
    {
        lua->pushboolean(false);
        return 1;
    }
}

int fb::game::builtin::character::builtin_reward(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ch = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    if (lua->is_string(2) == false)
    {
        lua->pushboolean(false);
        return 1;
    }

    auto reward_id = lua->tostring(2);
    if (table::reward.contains(reward_id) == false)
    {
        lua->pushboolean(false);
        return 1;
    }

    auto& reward = table::reward[reward_id];
    lua->pushboolean(ch->reward(reward.dsl));
    return 1;
}

int builtin::character::builtin_send_system_mail(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto argc   = lua->argc();
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto title = lua->tostring(2);
    if (title.empty())
    {
        lua->pushboolean(false);
        return 1;
    }

    auto contents = lua->tostring(3);
    if (contents.empty())
    {
        lua->pushboolean(false);
        return 1;
    }

    auto expire_date = std::optional<std::string>{std::nullopt};
    if (argc >= 3 && lua->is_nil(3) == false && lua->is_string(3))
        expire_date = lua->tostring(3);

    static auto fn = [](fb::game::server*                 server,
                        fb::lua::context*                 lua,
                        uint32_t                          sender,
                        const std::string&                title,
                        const std::string&                contents,
                        const std::optional<std::string>& expire_date) -> async::task<void> {
        auto   success = false;
        auto   world   = fb::config<uint32_t>("world");
        auto&& resp    = co_await server->http.post(
            "internal",
            "/mail/system",
            internal_reqs::WriteSystemMail{world,
                                           sender,
                                           title,
                                           contents,
                                           expire_date.has_value() ? expire_date.value() : std::string{}});
        success = resp.error == 0;

        co_await lua->switching();
        lua->pushboolean(success);
        lua->resume(1);
    };

    async::awaitable_then(fn(server, lua, ch->id, title, contents, expire_date), [lua](auto result) {
        result();
    });

    return lua->yield(1);
}

int builtin::character::builtin_storage_entries(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto weak = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto buffer = std::vector<fb::game::storage_box::entry>();
        buffer.reserve(ch->storage_box.entries().size());
        auto now = fb::model::datetime();
        for (const auto& [id, entry] : ch->storage_box.entries())
        {
            // Exclude entries with no rewards, already received, or expired
            if (entry.attachments.empty())
                continue;
            if (entry.received)
                continue;
            if (entry.expire_date.has_value() && entry.expire_date.value() < now)
                continue;
            buffer.push_back(entry);
        }

        std::sort(buffer.begin(), buffer.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.id > rhs.id;
        });

        return lua->ensure_resume(*server, weak, [=]() {
            lua->new_table();
            auto i = 0;
            for (const auto& entry : buffer)
            {
                lua->pushinteger(i + 1);
                lua->new_table();

                lua->pushstring("id");
                lua->pushinteger(entry.id);
                lua_settable(L, -3);

                lua->pushstring("title");
                lua->pushstring(entry.title);
                lua_settable(L, -3);

                lua->pushstring("message");
                lua->pushstring(entry.message);
                lua_settable(L, -3);

                lua->pushstring("received");
                lua->pushboolean(entry.received);
                lua_settable(L, -3);

                lua->pushstring("expired_date");
                if (entry.expire_date.has_value())
                {
                    lua->pushstring(entry.expire_date.value().to_string());
                }
                else
                {
                    lua->pushnil();
                }
                lua_settable(L, -3);

                lua->pushstring("attachments");
                lua->new_table();
                auto j = 0;
                for (const auto& attachment : entry.attachments)
                {
                    lua->pushinteger(j + 1);
                    auto json = attachment.to_json();
                    lua->pushjson(json);
                    lua_settable(L, -3);
                    j++;
                }
                lua_settable(L, -3);

                lua_settable(L, -3);
                i++;
            }
            return 1;
        });
    });
}

int builtin::character::builtin_receive_storage_reward(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto entry_id = static_cast<uint32_t>(lua->tointeger(2));
    if (entry_id == 0)
    {
        lua->pushboolean(false);
        return 1;
    }

    auto weak = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        auto success = ch->storage_box.receive_reward(entry_id);

        return lua->ensure_resume(*server, weak, [=]() {
            lua->pushboolean(success);
            return 1;
        });
    });
}

int builtin::character::builtin_marketplace_list(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto argc = lua->argc();
    if (argc < 4)
    {
        lua->pushstring("Invalid arguments: marketplace_list(item_index, count, price, [expire_hours])");
        return 1;
    }

    auto item_index   = static_cast<uint8_t>(lua->tointeger(2));
    auto count        = static_cast<uint16_t>(lua->tointeger(3));
    auto price        = static_cast<uint32_t>(lua->tointeger(4));
    auto expire_hours = static_cast<uint16_t>(lua->tointeger(5, 72));

    static auto fn = [](fb::lua::context*                  lua,
                        fb::game::server*                  server,
                        std::weak_ptr<fb::game::character> weak,
                        uint8_t                            item_index,
                        uint16_t                           count,
                        uint32_t                           price,
                        uint16_t                           expire_hours) -> async::task<void> {
        try
        {
            auto shared = weak.lock();
            if (shared == nullptr)
                throw std::runtime_error("Character is not alive");

            auto listing = co_await shared->marketplace.list(item_index, count, price, expire_hours);
            lua->pushnil(); // No error
            listing.to_lua(lua);
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
            lua->pushnil();
        }

        lua->resume(2);
    };

    auto weak = ch->weak_from_this_as<fb::game::character>();
    server->threads.enqueue(weak, [=](auto&) -> async::task<void> {
        co_await fn(lua, server, weak, item_index, count, price, expire_hours);
    });

    return lua->yield(2);
}

int builtin::character::builtin_marketplace_cancel(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto argc = lua->argc();
    if (argc < 2)
    {
        lua->pushstring("Invalid arguments: marketplace_cancel(listing_id)");
        return 1;
    }

    auto listing_id = lua->tostring(2);
    if (listing_id.empty())
    {
        lua->pushstring("Invalid listing_id");
        return 1;
    }

    static auto fn = [](fb::lua::context*                  lua,
                        fb::game::server*                  server,
                        std::weak_ptr<fb::game::character> weak,
                        const std::string&                 listing_id) -> async::task<void> {
        try
        {
            auto shared = weak.lock();
            if (shared == nullptr)
                throw std::runtime_error("Character is not alive");

            auto success = co_await shared->marketplace.cancel(listing_id);
            lua->pushnil(); // No error
            lua->pushboolean(success);
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
            lua->pushnil();
        }

        lua->resume(2);
    };

    auto weak = ch->weak_from_this_as<fb::game::character>();
    server->threads.enqueue(weak, [=](auto&) -> async::task<void> {
        co_await fn(lua, server, weak, listing_id);
    });

    return lua->yield(2);
}

int builtin::character::builtin_marketplace_purchase(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto argc = lua->argc();
    if (argc < 3)
    {
        lua->pushstring("Invalid arguments: marketplace_purchase(listing_id, purchase_count)");
        return 1;
    }

    auto listing_id = lua->tostring(2);
    if (listing_id.empty())
    {
        lua->pushstring("Invalid listing_id");
        return 1;
    }

    auto purchase_count = lua->tointeger(3);
    if (purchase_count <= 0 || purchase_count > 65535)
    {
        lua->pushstring("Invalid purchase_count (must be between 1 and 65535)");
        return 1;
    }

    static auto fn = [](fb::lua::context*                  lua,
                        fb::game::server*                  server,
                        std::weak_ptr<fb::game::character> weak,
                        const std::string&                 listing_id,
                        uint16_t                           purchase_count) -> async::task<void> {
        try
        {
            auto shared = weak.lock();
            if (shared == nullptr)
                throw std::runtime_error("Character is not alive");

            auto listing = co_await shared->marketplace.purchase(listing_id, purchase_count);
            lua->pushnil(); // No error
            listing.to_lua(lua);
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
            lua->pushnil();
        }

        lua->resume(2);
    };

    auto weak = ch->weak_from_this_as<fb::game::character>();
    server->threads.enqueue(weak, [=](auto&) -> async::task<void> {
        co_await fn(lua, server, weak, listing_id, static_cast<uint16_t>(purchase_count));
    });

    return lua->yield(2);
}

int builtin::character::builtin_marketplace_search(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto argc = lua->argc();
    if (argc < 2 || !lua->is_table(2))
    {
        lua->pushstring("Invalid arguments: marketplace_search(option_table)");
        return 1;
    }

    marketplace::search_option option;

    // Read option table
    lua_pushstring(*lua, "item_name");
    lua_rawget(*lua, 2);
    if (!lua->is_nil(-1))
    {
        option.item_name = lua->tostring(-1);
    }
    lua->pop(1);

    lua_pushstring(*lua, "min_price");
    lua_rawget(*lua, 2);
    if (!lua->is_nil(-1))
    {
        option.min_price = static_cast<uint32_t>(lua->tointeger(-1));
    }
    lua->pop(1);

    lua_pushstring(*lua, "max_price");
    lua_rawget(*lua, 2);
    if (!lua->is_nil(-1))
    {
        option.max_price = static_cast<uint32_t>(lua->tointeger(-1));
    }
    lua->pop(1);

    lua_pushstring(*lua, "seller_id");
    lua_rawget(*lua, 2);
    if (!lua->is_nil(-1))
    {
        option.seller_id = static_cast<uint32_t>(lua->tointeger(-1));
    }
    lua->pop(1);

    lua_pushstring(*lua, "sort_by");
    lua_rawget(*lua, 2);
    if (!lua->is_nil(-1))
    {
        option.sort_by = lua->tostring(-1);
    }
    lua->pop(1);

    lua_pushstring(*lua, "page");
    lua_rawget(*lua, 2);
    if (!lua->is_nil(-1))
    {
        option.page = static_cast<uint32_t>(lua->tointeger(-1));
    }
    lua->pop(1);

    static auto fn = [](fb::lua::context*                  lua,
                        fb::game::server*                  server,
                        std::weak_ptr<fb::game::character> weak,
                        marketplace::search_option         option) -> async::task<void> {
        try
        {
            auto shared = weak.lock();
            if (shared == nullptr)
                throw std::runtime_error("Character is not alive");

            auto result = co_await shared->marketplace.search(option);
            lua->pushnil(); // No error

            // Create result table
            lua->new_table();

            // Push listings array
            lua_pushstring(*lua, "listings");
            lua->new_table();
            for (size_t i = 0; i < result.listings.size(); i++)
            {
                lua->pushinteger(i + 1);
                result.listings[i].to_lua(lua);
                lua_settable(*lua, -3);
            }
            lua_settable(*lua, -3);

            // Push total_count
            lua_pushstring(*lua, "total_count");
            lua->pushinteger(result.total_count);
            lua_settable(*lua, -3);

            // Push page
            lua_pushstring(*lua, "page");
            lua->pushinteger(result.page);
            lua_settable(*lua, -3);
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
            lua->pushnil();
        }

        lua->resume(2);
    };

    auto weak = ch->weak_from_this_as<fb::game::character>();
    server->threads.enqueue(weak, [=](auto&) -> async::task<void> {
        co_await fn(lua, server, weak, option);
    });

    return lua->yield(2);
}

int builtin::character::builtin_marketplace_get_listings(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto argc = lua->argc();
    if (argc < 2 || !lua->is_table(2))
    {
        lua->pushstring("Invalid arguments: marketplace_get_listings(listing_ids_table)");
        return 1;
    }

    std::vector<std::string> listing_ids;
    auto                     table_size = lua->rawlen(2);
    listing_ids.reserve(table_size);
    for (int i = 1; i <= table_size; i++)
    {
        lua->rawgeti(2, i);
        if (lua->is_string(-1))
        {
            listing_ids.push_back(lua->tostring(-1));
        }
        lua->pop(1);
    }

    if (listing_ids.empty())
    {
        lua->pushstring("listing_ids table is empty");
        return 1;
    }

    static auto fn = [](fb::lua::context*                  lua,
                        fb::game::server*                  server,
                        std::weak_ptr<fb::game::character> weak,
                        std::vector<std::string>           listing_ids) -> async::task<void> {
        try
        {
            auto shared = weak.lock();
            if (shared == nullptr)
                throw std::runtime_error("Character is not alive");

            auto listings = co_await shared->marketplace.get_listings(listing_ids);
            lua->pushnil(); // No error

            // Create listings array
            lua->new_table();
            for (size_t i = 0; i < listings.size(); i++)
            {
                lua->pushinteger(i + 1);
                listings[i].to_lua(lua);
                lua_settable(*lua, -3);
            }
        }
        catch (std::exception& e)
        {
            lua->pushstring(e.what());
            lua->pushnil();
        }

        lua->resume(2);
    };

    auto weak = ch->weak_from_this_as<fb::game::character>();
    server->threads.enqueue(weak, [=](auto&) -> async::task<void> {
        co_await fn(lua, server, weak, listing_ids);
    });

    return lua->yield(2);
}

int builtin::character::builtin_marketplace_pending_listings(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto weak = ch->weak_from_this_as<fb::game::character>();
    return lua->ensure_yield(*server, weak, [=](auto is_yield) {
        const auto& pending_listings = ch->marketplace.pending_listings();

        return lua->ensure_resume(*server, weak, [=]() {
            lua->new_table();
            auto i = 0;
            for (const auto& [listing_id, pending_info] : pending_listings)
            {
                lua->pushinteger(i + 1);
                lua->new_table();

                lua_pushstring(*lua, "listing_id");
                lua->pushstring(listing_id);
                lua_settable(*lua, -3);

                lua_pushstring(*lua, "type");
                lua->pushinteger(static_cast<int>(pending_info.type));
                lua_settable(*lua, -3);

                lua_pushstring(*lua, "character_id");
                lua->pushinteger(pending_info.character_id);
                lua_settable(*lua, -3);

                lua_pushstring(*lua, "dsls");
                lua->new_table();
                auto j = 0;
                for (const auto& dsl : pending_info.dsls)
                {
                    lua->pushinteger(j + 1);
                    auto json = dsl.to_json();
                    lua->pushjson(json);
                    lua_settable(*lua, -3);
                    j++;
                }
                lua_settable(*lua, -3);

                lua_settable(*lua, -3);
                i++;
            }
            return 1;
        });
    });
}

int builtin::character::builtin_marriage(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ch = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    ch->assert_thread();

    const auto& m           = ch->marriage();
    bool        married     = m.spouse_id.has_value();
    bool        can_remarry = !married && (fb::model::datetime() >= m.remarriage_after);

    lua->new_table();
    lua_pushstring(*lua, "married");
    lua->pushboolean(married);
    lua_settable(*lua, -3);
    lua_pushstring(*lua, "spouse_id");
    if (m.spouse_id.has_value())
        lua->pushinteger(static_cast<int64_t>(m.spouse_id.value()));
    else
        lua->pushnil();
    lua_settable(*lua, -3);
    lua_pushstring(*lua, "spouse_name");
    lua->pushstring(m.spouse_name);
    lua_settable(*lua, -3);
    lua_pushstring(*lua, "divorce_count");
    lua->pushinteger(static_cast<int64_t>(m.divorce_count));
    lua_settable(*lua, -3);
    lua_pushstring(*lua, "can_remarry");
    lua->pushboolean(can_remarry);
    lua_settable(*lua, -3);
    return 1;
}

int builtin::character::builtin_marry(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto me     = lua->touserdata<fb::game::character>(1);
    auto target = lua->touserdata<fb::game::character>(2);
    if (me == nullptr || target == nullptr)
    {
        lua->pushstring("invalid character");
        return 1;
    }

    me->assert_thread();

    const auto& my_m  = me->marriage();
    const auto& tar_m = target->marriage();

    if (my_m.spouse_id.has_value())
    {
        lua->pushstring("already married");
        return 1;
    }
    if (tar_m.spouse_id.has_value())
    {
        lua->pushstring("target already married");
        return 1;
    }
    if (fb::model::datetime() < my_m.remarriage_after)
    {
        lua->pushstring("remarriage cooldown");
        return 1;
    }
    if (fb::model::datetime() < tar_m.remarriage_after)
    {
        lua->pushstring("target remarriage cooldown");
        return 1;
    }

    fb::game::marriage new_me;
    new_me.spouse_id        = target->id;
    new_me.spouse_name      = target->name();
    new_me.remarriage_after = fb::model::datetime();
    new_me.divorce_count    = my_m.divorce_count;

    fb::game::marriage new_tar;
    new_tar.spouse_id        = me->id;
    new_tar.spouse_name      = me->name();
    new_tar.remarriage_after = fb::model::datetime();
    new_tar.divorce_count    = tar_m.divorce_count;

    me->marriage(new_me);
    target->marriage(new_tar);

    lua->pushnil();
    return 1;
}

int builtin::character::builtin_divorce(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto me     = lua->touserdata<fb::game::character>(1);
    if (me == nullptr)
    {
        lua->pushstring("invalid character");
        return 1;
    }

    me->assert_thread();

    const auto& m = me->marriage();
    if (!m.spouse_id.has_value())
    {
        lua->pushstring("not married");
        return 1;
    }

    auto spouse = server->characters.read([id = m.spouse_id.value()](auto& container) {
        return container.find(id);
    });

    if (spouse == nullptr)
    {
        lua->pushstring("spouse not online");
        return 1;
    }

    auto now = fb::model::datetime();
    now.add_days(7);

    fb::game::marriage new_me;
    new_me.spouse_id        = std::nullopt;
    new_me.spouse_name      = "";
    new_me.remarriage_after = now;
    new_me.divorce_count    = m.divorce_count + 1;

    fb::game::marriage new_tar;
    new_tar.spouse_id        = std::nullopt;
    new_tar.spouse_name      = "";
    new_tar.remarriage_after = now;
    new_tar.divorce_count    = spouse->marriage().divorce_count + 1;

    me->marriage(new_me);
    spouse->marriage(new_tar);

    lua->pushnil();
    return 1;
}