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
{"__eq",                         builtin::object::builtin_eq},
{"uid",                          builtin::character::builtin_uid},
{"look",                         builtin::character::builtin_look},
{"color",                        builtin::character::builtin_color},
{"gender",                       builtin::character::builtin_gender},
{"money",                        builtin::character::builtin_money},
{"exp",                          builtin::character::builtin_exp},
{"item",                         builtin::character::builtin_item},
{"items",                        builtin::character::builtin_items},
{"has_items",                    builtin::character::builtin_has_items},
{"equipments",                   builtin::character::builtin_equipments},
{"equipment_off",                builtin::character::builtin_equipment_off},
{"dropitem",                     builtin::character::builtin_item_drop},
{"mkitem",                       builtin::character::builtin_mkitem},
{"rmitem",                       builtin::character::builtin_rmitem},
{"exchange",                     builtin::character::builtin_exchange},
{"state",                        builtin::character::builtin_state},
{"mimic",                        builtin::character::builtin_mimic},
{"class",                        builtin::character::builtin_class},
{"promotion",                    builtin::character::builtin_promotion},
{"level",                        builtin::character::builtin_level},
{"assert",                       builtin::character::builtin_assert},
{"role",                         builtin::character::builtin_role},
{"deposited_money",              builtin::character::builtin_deposited_money},
{"stored_item",                  builtin::character::builtin_stored_item},
{"store_item",                   builtin::character::builtin_store_item},
{"retrieve_item",                builtin::character::builtin_retrieve_item},
{"group",                        builtin::character::builtin_group},
{"create_group",                 builtin::character::builtin_create_group},
{"clan",                         builtin::character::builtin_clan},
{"create_clan",                  builtin::character::builtin_create_clan},
{"destroy_clan",                 builtin::character::builtin_destroy_clan},
{"achievements",                 builtin::character::builtin_achievements},
{"achievement",                  builtin::character::builtin_achievement},
{"push_achievement",             builtin::character::builtin_push_achievement},
{"erase_achievement",            builtin::character::builtin_erase_achievement},
{"whisper",                      builtin::character::builtin_whisper},
{"send_mail",                    builtin::character::builtin_send_mail},
{"nation",                       builtin::character::builtin_nation},
{"weapon",                       builtin::character::builtin_weapon},
{"title",                        builtin::character::builtin_title},
{"gain",                         builtin::character::builtin_gain},
{"weapon_damage",                builtin::character::builtin_weapon_damage},
{"detect",                       builtin::character::builtin_detect},
{"spawn_mob",                    builtin::character::builtin_spawn_mob},
{"spawned_mobs",                 builtin::character::builtin_spawned_mobs},
{"base_hp",                      builtin::character::builtin_base_hp},
{"base_mp",                      builtin::character::builtin_base_mp},
{"base_str",                     builtin::character::builtin_base_str},
{"base_dex",                     builtin::character::builtin_base_dex},
{"base_int",                     builtin::character::builtin_base_int},
{"base_dam",                     builtin::character::builtin_base_dam},
{"base_hit",                     builtin::character::builtin_base_hit},
{"armor_color",                  builtin::character::builtin_armor_color},
{"weapon_color",                 builtin::character::builtin_weapon_color},
{"shield_color",                 builtin::character::builtin_shield_color},
{"mkspell",                      builtin::character::builtin_mkspell},
{"rmspell",                      builtin::character::builtin_rmspell},
{"world",                        builtin::character::builtin_world},
{"ad",                           builtin::character::builtin_ad},
{"web",                          builtin::character::builtin_web},
{"birthday",                     builtin::character::builtin_birthday},
{"active",                       builtin::character::builtin_active},
{"super_hide",                   builtin::character::builtin_super_hide},
{"creature",                     builtin::character::builtin_creature},
{"teleport",                     builtin::character::builtin_teleport},
{"dialog",                       builtin::character::builtin_dialog},
{"list",                         builtin::character::builtin_list},
{"input",                        builtin::character::builtin_input},
{"menu",                         builtin::character::builtin_menu},
{"slot",                         builtin::character::builtin_slot},
{"rezen_force",                  builtin::character::builtin_rezen_force},
{"quest",                        builtin::character::builtin_quest},
{"start_quest",                  builtin::character::builtin_start_quest},
{"remove_quest",                 builtin::character::builtin_remove_quest},
{"reward",                       builtin::character::builtin_reward},
{"send_system_mail",             builtin::character::builtin_send_system_mail},
{"storage_entries",              builtin::character::builtin_storage_entries},
{"receive_storage_reward",       builtin::character::builtin_receive_storage_reward},
{"marketplace_list",             builtin::character::builtin_marketplace_list},
{"marketplace_cancel",           builtin::character::builtin_marketplace_cancel},
{"marketplace_purchase",         builtin::character::builtin_marketplace_purchase},
{"marketplace_search",           builtin::character::builtin_marketplace_search},
{"marketplace_get_listings",     builtin::character::builtin_marketplace_get_listings},
{"marketplace_pending_listings", builtin::character::builtin_marketplace_pending_listings},
{"marriage",                     builtin::character::builtin_marriage},
{"marry",                        builtin::character::builtin_marry},
{"divorce",                      builtin::character::builtin_divorce},
{"unknown_12",                   builtin::character::builtin_unknown_12},
{"unknown_26",                   builtin::character::builtin_unknown_26},
{"ui",                           builtin::character::builtin_ui},
{"item_throw_confirm",           builtin::character::builtin_item_throw_confirm},
{"freeze",                       builtin::character::builtin_freeze},
{"friends_sync",                 builtin::character::builtin_friends_sync},
{"unknown_4B",                   builtin::character::builtin_unknown_4B},
{"unknown_4D",                   builtin::character::builtin_unknown_4D},
{"unknown_35",                   builtin::character::builtin_unknown_35},
{"holyday_screen",               builtin::character::builtin_holyday_screen},
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto uid      = std::make_shared<uint32_t>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *uid = ch->id;
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(*uid);
        co_return 1;
    };
    return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto look     = std::make_shared<uint16_t>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *look = ch->look();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*look);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = (uint16_t)lua->tointeger(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->look(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto color    = std::make_shared<uint8_t>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *color = ch->color();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*color);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = (uint8_t)lua->tointeger(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->color(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto gender   = std::make_shared<GENDER>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *gender = ch->gender();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*gender);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto gender   = static_cast<GENDER>(lua->tointeger(2));
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->gender(gender);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto money    = std::make_shared<uint32_t>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *money = ch->money();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*money);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = (uint32_t)lua->tointeger(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->money(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto exp      = std::make_shared<uint32_t>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *exp = ch->exp();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*exp);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = (uint32_t)lua->tointeger(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->exp(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        ch->listener.on_dialog(*ch, *model, message, items, oid);
        if (ch->dialog != nullptr)
            ch->dialog->release();

        ch->dialog = lua;
        co_return;
    };
    return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto buffer   = std::make_shared<std::vector<std::pair<uint8_t, std::shared_ptr<fb::game::item>>>>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        for (int i = 0; i < CONTAINER_CAPACITY; i++)
        {
            if (ch->items[i] == nullptr)
                continue;

            if (filter_name.has_value() && ch->items[i]->based<fb::model::item>().name != *filter_name)
                continue;

            buffer->push_back({i, ch->items[i]->shared_from_this_as<fb::game::item>()});
        }
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->new_table();
        for (auto& [index, item] : *buffer)
        {
            lua->pushinteger(index + 1);
            lua->pushobject(item);
            lua_settable(L, -3);
        }
        co_return 1;
    };
    return builder.run();
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

    auto server   = lua->env<fb::game::server>("server");
    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto result   = std::make_shared<bool>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *result = ch->items.has(required_models);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto buffer   = std::make_shared<std::vector<std::pair<EQUIPMENT_PARTS, std::shared_ptr<fb::game::item>>>>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        for (auto& [parts, equipment] : ch->items.equipments())
        {
            if (equipment == nullptr)
                continue;

            buffer->push_back({parts, equipment});
        }
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->new_table();
        for (auto& [parts, equipment] : *buffer)
        {
            lua->pushinteger(parts);
            lua->pushobject(equipment);
            lua_settable(L, -3);
        }
        co_return 1;
    };
    return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto slot     = std::make_shared<uint8_t>(0xFF);
    auto item     = std::make_shared<std::shared_ptr<fb::game::item>>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *slot = ch->items.inactive(parts);
        if (*slot != 0xFF)
            *item = ch->items.at(*slot);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*slot == 0xFF)
        {
            lua->pushnil();
            co_return 1;
        }
        if (*item != nullptr)
            lua->pushobject(*item);
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto dropped  = std::make_shared<std::shared_ptr<fb::game::item>>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *dropped = ch->items.drop(index - 1, drop_all ? 1 : -1);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*dropped != nullptr)
            lua->pushobject(*dropped);
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
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

    auto failed       = std::make_shared<bool>(false);
    auto result_items = std::make_shared<std::vector<std::shared_ptr<fb::game::item>>>();
    auto builder      = lua->new_co_builder(*server);
    builder.weak      = weak;
    builder.yield     = [=]() -> async::task<void> {
        if (ch->items.is_rewardable(id_to_count, 0) == false)
        {
            *failed = true;
            co_return;
        }

        auto items = std::vector<std::shared_ptr<fb::game::item>>();
        for (auto& [id, count] : id_to_count)
        {
            auto& model = table::item[id];
            auto  item  = model.make(*server, count);
            if (item != nullptr)
                items.push_back(std::move(item));
        }
        auto slots = ch->items.add(items, true);
        for (auto slot : slots)
            result_items->push_back(ch->items[slot]);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*failed)
        {
            lua->pushnil();
            co_return 1;
        }
        if (single_item)
        {
            if (result_items->empty())
                lua->pushnil();
            else
                lua->pushobject((*result_items)[0]);
        }
        else
        {
            lua->new_table();
            for (size_t i = 0; i < result_items->size(); i++)
            {
                lua->pushobject((*result_items)[i]);
                lua->rawseti(-2, static_cast<int>(i + 1));
            }
        }
        co_return 1;
    };
    return builder.run();
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

        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto success  = std::make_shared<bool>(false);
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
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
                    *success = true;
                }
            }
            catch (...)
            { }
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushboolean(*success);
            co_return 1;
        };
        return builder.run();
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

        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto success  = std::make_shared<bool>(false);
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
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
                        *success    = true;
                    }
                }
            }
            catch (...)
            { }
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushboolean(*success);
            co_return 1;
        };
        return builder.run();
    }
    else if (lua->is_userdata<fb::model::item>(2))
    {
        auto model = lua->touserdata<fb::model::item>(2);
        if (model == nullptr)
        {
            lua->pushboolean(false);
            return 1;
        }

        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto success  = std::make_shared<bool>(false);
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
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
                        *success    = true;
                    }
                }
            }
            catch (...)
            { }
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushboolean(*success);
            co_return 1;
        };
        return builder.run();
    }
    else if (lua->is_number(2))
    {
        auto raw_index = lua->tointeger(2);
        auto index     = static_cast<uint8_t>(raw_index) - 1;

        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto success  = std::make_shared<bool>(false);
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            try
            {
                auto item = ch->items.at(index);
                if (item != nullptr && item->count() >= count)
                {
                    auto dropped = ch->items.remove(index, count, delete_attr);
                    if (dropped != nullptr)
                    {
                        std::ignore = dropped->destroy();
                        *success    = true;
                    }
                }
            }
            catch (...)
            { }
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushboolean(*success);
            co_return 1;
        };
        return builder.run();
    }
    else if (lua->is_string(2))
    {
        auto name = lua->tostring(2);
        if (name.empty())
        {
            lua->pushboolean(false);
            return 1;
        }

        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto success  = std::make_shared<bool>(false);
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            try
            {
                auto model = table::item.name2item(name);
                if (model == nullptr)
                    co_return;

                auto found = ch->items.find(name);
                if (found != nullptr && found->count() >= count)
                {
                    auto index   = ch->items.index(*model);
                    auto dropped = ch->items.remove(index, count, delete_attr);
                    if (dropped != nullptr)
                    {
                        std::ignore = dropped->destroy();
                        *success    = true;
                    }
                }
            }
            catch (...)
            { }
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushboolean(*success);
            co_return 1;
        };
        return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto result   = std::make_shared<exchange_result>(exchange_result::ok);
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        if (cost_exp > 0 && ch->exp() < cost_exp)
        {
            *result = exchange_result::lack_cost;
        }
        else
        {
            *result = ch->items.exchange(cost_items, cost_money, reward_items, reward_money);
            if (*result == exchange_result::ok)
            {
                if (cost_exp > 0)
                    ch->reduce_exp(cost_exp);
                if (reward_exp > 0)
                    ch->add_exp(reward_exp, false, true);
            }
        }
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushinteger(static_cast<int>(*result));
        co_return 1;
    };
    return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto state    = std::make_shared<STATE>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *state = ch->state();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*state);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = STATE(lua->tointeger(2));
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->state(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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
        auto weak       = ch->weak_from_this_as<fb::game::character>();
        auto appearance = std::make_shared<std::optional<fb::game::character_appearance>>();
        auto builder    = lua->new_co_builder(*server);
        builder.weak    = weak;
        builder.yield   = [=]() -> async::task<void> {
            *appearance = ch->mimicry();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            if (!appearance->has_value())
            {
                lua->pushnil();
                co_return 1;
            }
            auto const& p = appearance->value();
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
            if (p.state.has_value())
            {
                lua->pushstring("state");
                lua->pushinteger(static_cast<lua_Integer>(static_cast<uint8_t>(p.state.value())));
                lua->settable(-3);
            }
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
            co_return 1;
        };
        return builder.run();
    }
    else if (argc >= 2)
    {
        if (lua->is_nil(2))
        {
            auto weak     = ch->weak_from_this_as<fb::game::character>();
            auto builder  = lua->new_co_builder(*server);
            builder.weak  = weak;
            builder.yield = [=]() -> async::task<void> {
                ch->mimicry(std::nullopt);
                co_return;
            };
            builder.resume = []() -> async::task<int> {
                co_return 0;
            };
            return builder.run();
        }
        if (!lua->is_table(2))
            return 0;

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
        else
            appearance.state = std::nullopt;
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

        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->mimicry(std::move(appearance));
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto cls      = std::make_shared<CLASS>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *cls = ch->cls();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*cls);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->cls(static_cast<CLASS>(value));
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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
        auto weak      = ch->weak_from_this_as<fb::game::character>();
        auto promotion = std::make_shared<uint8_t>();
        auto builder   = lua->new_co_builder(*server);
        builder.weak   = weak;
        builder.yield  = [=]() -> async::task<void> {
            *promotion = ch->promotion();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*promotion);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->promotion(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto level    = std::make_shared<uint8_t>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *level = ch->level();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*level);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto level    = std::max(0, std::min((int)lua->tointeger(2), 255));
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->level(level);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto error    = std::make_shared<std::optional<std::string>>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        try
        {
            ch->assert_state(values);
        }
        catch (std::runtime_error& e)
        {
            *error = e.what();
        }
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
        {
            lua->pushstring(error->value().c_str());
            co_return 1;
        }
        co_return 0;
    };
    return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto role     = std::make_shared<ROLE>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *role = ch->role();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*role);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = static_cast<ROLE>(lua->tointeger(2));
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->role(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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
        auto weak      = ch->weak_from_this_as<fb::game::character>();
        auto deposited = std::make_shared<uint32_t>();
        auto builder   = lua->new_co_builder(*server);
        builder.weak   = weak;
        builder.yield  = [=]() -> async::task<void> {
            *deposited = ch->items.deposited();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*deposited);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->items.deposited(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto buffer   = std::make_shared<std::vector<std::shared_ptr<fb::game::item>>>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            const auto& stored_items = ch->items.stored();
            buffer->reserve(stored_items.size());
            for (const auto& stored_item : stored_items)
                buffer->push_back(stored_item);
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->new_table();
            for (int i = 0; i < buffer->size(); i++)
            {
                lua->pushinteger(i + 1);
                lua->pushobject(buffer->at(i));
                lua_settable(L, -3);
            }
            co_return 1;
        };
        return builder.run();
    }
    else if (lua->is_number(2))
    {
        // Search by index
        auto index = lua->tointeger(2);

        auto weak        = ch->weak_from_this_as<fb::game::character>();
        auto stored_item = std::make_shared<std::shared_ptr<fb::game::item>>();
        auto builder     = lua->new_co_builder(*server);
        builder.weak     = weak;
        builder.yield    = [=]() -> async::task<void> {
            const auto& stored_items = ch->items.stored();
            if (index <= stored_items.size() - 1)
                *stored_item = stored_items.at(index);
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            if (*stored_item == nullptr)
                lua->pushnil();
            else
                lua->pushobject(*stored_item);
            co_return 1;
        };
        return builder.run();
    }
    else if (lua->is_string(2))
    {
        // Search by name
        auto name = lua->tostring(2);

        auto weak        = ch->weak_from_this_as<fb::game::character>();
        auto stored_item = std::make_shared<std::shared_ptr<fb::game::item>>();
        auto builder     = lua->new_co_builder(*server);
        builder.weak     = weak;
        builder.yield    = [=]() -> async::task<void> {
            const auto& stored_items = ch->items.stored();
            auto        found        = std::find_if(stored_items.cbegin(),
                                      stored_items.cend(),
                                      [&name](const std::shared_ptr<fb::game::item>& item) {
                                          return item->based<fb::model::item>().name == name;
                                      });
            if (found != stored_items.cend())
                *stored_item = *found;
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            if (*stored_item == nullptr)
                lua->pushnil();
            else
                lua->pushobject(*stored_item);
            co_return 1;
        };
        return builder.run();
    }
    else if (lua->is_userdata<fb::game::item>(2))
    {
        // Search by game item
        auto item = lua->touserdata<fb::game::item>(2);

        auto weak        = ch->weak_from_this_as<fb::game::character>();
        auto stored_item = std::make_shared<std::shared_ptr<fb::game::item>>();
        auto builder     = lua->new_co_builder(*server);
        builder.weak     = weak;
        builder.yield    = [=]() -> async::task<void> {
            const auto& stored_items = ch->items.stored();
            auto&       item_model   = item->based<fb::model::item>();
            auto        found        = std::find_if(stored_items.cbegin(),
                                      stored_items.cend(),
                                      [&item_model](const std::shared_ptr<fb::game::item>& stored) {
                                          return stored->based<fb::model::item>() == item_model;
                                      });
            if (found != stored_items.cend())
                *stored_item = *found;
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            if (*stored_item == nullptr)
                lua->pushnil();
            else
                lua->pushobject(*stored_item);
            co_return 1;
        };
        return builder.run();
    }
    else if (lua->is_userdata<fb::model::item>(2))
    {
        // Search by model item
        auto model = lua->touserdata<fb::model::item>(2);

        auto weak        = ch->weak_from_this_as<fb::game::character>();
        auto stored_item = std::make_shared<std::shared_ptr<fb::game::item>>();
        auto builder     = lua->new_co_builder(*server);
        builder.weak     = weak;
        builder.yield    = [=]() -> async::task<void> {
            const auto& stored_items = ch->items.stored();
            auto        found        = std::find_if(stored_items.cbegin(),
                                      stored_items.cend(),
                                      [model](const std::shared_ptr<fb::game::item>& stored) {
                                          return stored->based<fb::model::item>() == *model;
                                      });
            if (found != stored_items.cend())
                *stored_item = *found;
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            if (*stored_item == nullptr)
                lua->pushnil();
            else
                lua->pushobject(*stored_item);
            co_return 1;
        };
        return builder.run();
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
    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto success  = std::make_shared<bool>(false);
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto index = ch->items.index(item);
        if (index != 0xFF)
            *success = ch->items.store(index, count);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*success);
        co_return 1;
    };
    return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto returned = std::make_shared<std::shared_ptr<fb::game::item>>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto& stored_items = ch->items.stored();
        for (size_t i = 0; i < stored_items.size(); ++i)
        {
            if (stored_items[i] == item)
            {
                *returned = ch->items.retrieve(static_cast<uint8_t>(i), count);
                break;
            }
        }
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*returned == nullptr)
            lua->pushnil();
        else
            lua->pushobject(*returned);
        co_return 1;
    };
    return builder.run();
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
        auto weak         = ch->weak_from_this_as<fb::game::character>();
        auto group_holder = std::make_shared<std::shared_ptr<fb::game::group>>();
        auto builder      = lua->new_co_builder(*server);
        builder.weak      = weak;
        builder.yield     = [=]() -> async::task<void> {
            auto shared = weak.lock();
            if (shared == nullptr)
                co_return;

            auto group_id = shared->group_id();
            if (group_id.has_value())
            {
                auto guard    = server->groups.enter_read(group_id.value());
                *group_holder = guard.value();
            }
        };
        builder.resume = [=]() -> async::task<int> {
            if (*group_holder == nullptr)
                lua->pushnil();
            else
                lua->pushobject(*group_holder);
            co_return 1;
        };
        return builder.run();
    }
    else if (lua->is_function(2))
    {
        auto weak         = ch->weak_from_this_as<fb::game::character>();
        auto ch_holder    = std::make_shared<std::shared_ptr<fb::game::character>>();
        auto group_holder = std::make_shared<std::shared_ptr<fb::game::group>>();
        auto group_id     = std::make_shared<std::optional<uint32_t>>();
        auto builder      = lua->new_co_builder(*server);
        builder.weak      = weak;
        builder.yield     = [=]() -> async::task<void> {
            auto locked = weak.lock();
            if (locked == nullptr)
                co_return;

            *ch_holder = locked;
            auto gid   = locked->group_id();
            *group_id  = gid;
            if (gid.has_value())
            {
                auto guard    = server->groups.enter_read(gid.value());
                *group_holder = guard.value();
            }
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            static auto static_func = [](fb::lua::context* server) {
                lua_call(*server, 2, LUA_MULTRET);
                server->remove(-server->argc());
                return server->argc();
            };

            if (*ch_holder == nullptr)
            {
                lua->pushnil();
                lua->pushnil();
                co_return static_func(lua);
            }

            lua->pushobject(*ch_holder);
            if (*group_holder == nullptr)
                lua->pushnil();
            else
                lua->pushobject(*group_holder);
            co_return static_func(lua);
        };
        return builder.run();
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

    auto name     = lua->tostring(2);
    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto dead     = std::make_shared<bool>(false);
    auto success  = std::make_shared<bool>(false);
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto shared = weak.lock();
        if (shared == nullptr)
        {
            *dead = true;
            co_return;
        }

        try
        {
            co_await server->groups.create(*shared, name);
            *success = true;
        }
        catch (std::exception&)
        {
            *success = false;
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (*dead)
            lua->pushnil();
        else
            lua->pushboolean(*success);
        co_return 1;
    };
    return builder.run();
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
        auto weak        = ch->weak_from_this_as<fb::game::character>();
        auto clan_holder = std::make_shared<std::shared_ptr<fb::game::clan>>();
        auto builder     = lua->new_co_builder(*server);
        builder.weak     = weak;
        builder.yield    = [=]() -> async::task<void> {
            auto shared = weak.lock();
            if (shared == nullptr)
                co_return;

            auto& clan_id = shared->clan_id();
            if (clan_id.has_value())
            {
                auto guard = server->clans.try_enter_read(clan_id.value());
                if (guard.has_value())
                    *clan_holder = guard->value();
            }
        };
        builder.resume = [=]() -> async::task<int> {
            if (*clan_holder == nullptr)
                lua->pushnil();
            else
                lua->pushobject(*clan_holder);
            co_return 1;
        };
        return builder.run();
    }
    else if (lua->is_function(2))
    {
        auto weak        = ch->weak_from_this_as<fb::game::character>();
        auto ch_holder   = std::make_shared<std::shared_ptr<fb::game::character>>();
        auto clan_holder = std::make_shared<std::shared_ptr<fb::game::clan>>();
        auto builder     = lua->new_co_builder(*server);
        builder.weak     = weak;
        builder.yield    = [=]() -> async::task<void> {
            auto locked = weak.lock();
            if (locked == nullptr)
                co_return;

            *ch_holder    = locked;
            auto& clan_id = locked->clan_id();
            if (clan_id.has_value())
            {
                auto guard = server->clans.try_enter_read(clan_id.value());
                if (guard.has_value())
                    *clan_holder = guard->value();
            }
        };
        builder.resume = [=]() -> async::task<int> {
            static auto static_func = [](fb::lua::context* ctx) {
                lua_call(*ctx, 2, LUA_MULTRET);
                ctx->remove(-ctx->argc());
                return ctx->argc();
            };

            if (*ch_holder == nullptr)
            {
                lua->pushnil();
                lua->pushnil();
                co_return static_func(lua);
            }

            lua->pushobject(*ch_holder);
            if (*clan_holder == nullptr)
                lua->pushnil();
            else
                lua->pushobject(*clan_holder);
            co_return static_func(lua);
        };
        return builder.run();
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

    auto name     = lua->tostring(2);
    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto dead     = std::make_shared<bool>(false);
    auto error    = std::make_shared<std::optional<std::string>>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto shared = weak.lock();
        if (shared == nullptr)
        {
            *dead = true;
            co_return;
        }

        try
        {
            co_await server->clans.create(*shared, name);
        }
        catch (std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (*dead)
            lua->pushnil();
        else if (error->has_value())
            lua->pushstring(error->value().c_str());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto dead     = std::make_shared<bool>(false);
    auto error    = std::make_shared<std::optional<std::string>>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto shared = weak.lock();
        if (shared == nullptr)
        {
            *dead = true;
            co_return;
        }

        try
        {
            co_await server->clans.destroy(*shared);
        }
        catch (std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (*dead)
            lua->pushnil();
        else if (error->has_value())
            lua->pushstring(error->value().c_str());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto buffer   = std::make_shared<std::vector<fb::game::achievement*>>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        for (auto& [id, achievement] : ch->achievements)
            buffer->push_back(achievement.get());
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->new_table();
        auto i = 0;
        for (auto* achievement : *buffer)
        {
            lua->pushinteger(i + 1);
            lua->pushobject(*achievement);
            lua_settable(L, -3);
            i++;
        }
        co_return 1;
    };
    return builder.run();
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

    auto i           = lua->tointeger(2);
    auto weak        = ch->weak_from_this_as<fb::game::character>();
    auto achievement = std::make_shared<fb::game::achievement*>();
    auto builder     = lua->new_co_builder(*server);
    builder.weak     = weak;
    builder.yield    = [=]() -> async::task<void> {
        if (ch->achievements.contains(i))
            *achievement = ch->achievements.at(i).get();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*achievement == nullptr)
            lua->pushnil();
        else
            lua->pushobject(**achievement);
        co_return 1;
    };
    return builder.run();
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

    auto weak        = ch->weak_from_this_as<fb::game::character>();
    auto achievement = std::make_shared<fb::game::achievement*>();
    auto builder     = lua->new_co_builder(*server);
    builder.weak     = weak;
    builder.yield    = [=]() -> async::task<void> {
        ch->achievements[id] = std::make_unique<fb::game::achievement>(id, text, icon, color);
        *achievement         = ch->achievements[id].get();
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushobject(**achievement);
        co_return 1;
    };
    return builder.run();
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

    auto i        = lua->tointeger(2);
    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto success  = std::make_shared<bool>(false);
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        if (ch->achievements.contains(i))
        {
            ch->achievements.erase(i);
            *success = true;
        }
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*success);
        co_return 1;
    };
    return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto dead     = std::make_shared<bool>(false);
    auto error    = std::make_shared<std::optional<std::string>>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto shared = weak.lock();
        if (shared == nullptr)
        {
            *dead = true;
            co_return;
        }

        try
        {
            co_await shared->whisper(to, message);
        }
        catch (std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (*dead)
            lua->pushnil();
        else if (error->has_value())
            lua->pushstring(error->value().c_str());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto nation   = std::make_shared<NATION>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *nation = ch->nation();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(static_cast<uint8_t>(*nation));
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = static_cast<NATION>(lua->tointeger(2));
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto success  = std::make_shared<bool>(false);
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *success = ch->nation(value);
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushboolean(*success);
            co_return 1;
        };
        return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto weapon   = std::make_shared<std::shared_ptr<fb::game::weapon>>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *weapon = ch->items.weapon();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            if (*weapon == nullptr)
                lua->pushnil();
            else
                lua->pushobject(*weapon);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->touserdata<fb::game::weapon>(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->items.weapon(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto title    = std::make_shared<std::string>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *title = ch->title();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushstring(title->c_str());
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tostring(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->title(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        ch->items.add(items, true);
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
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
        auto weak          = ch->weak_from_this_as<fb::game::character>();
        auto weapon_damage = std::make_shared<uint16_t>();
        auto builder       = lua->new_co_builder(*server);
        builder.weak       = weak;
        builder.yield      = [=]() -> async::task<void> {
            *weapon_damage = ch->weapon_damage();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*weapon_damage);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->weapon_damage(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto detect   = std::make_shared<bool>(false);
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *detect = ch->detect();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushboolean(*detect);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->toboolean(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->detect(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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

    auto notify   = lua->toboolean(offset, true);
    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto coords   = std::make_shared<std::pair<uint16_t, uint16_t>>(x, y);
    auto mob_ptr  = std::make_shared<std::shared_ptr<fb::game::mob>>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        if (argc < 3)
        {
            coords->first  = ch->x();
            coords->second = ch->y();
        }
        *mob_ptr = ch->spawn_mob(*model, fb::model::point16_t(coords->first, coords->second), owned, notify);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*mob_ptr == nullptr)
            lua->pushnil();
        else
            lua->pushobject(*mob_ptr);
        co_return 1;
    };
    return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto buffer   = std::make_shared<std::vector<std::shared_ptr<fb::game::mob>>>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto& spawned_mobs = ch->spawned_mobs();
        buffer->reserve(spawned_mobs.size());
        for (const auto& mob : spawned_mobs)
            buffer->push_back(mob);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->new_table();
        for (int i = 0; i < buffer->size(); i++)
        {
            lua->pushinteger(i + 1);
            lua->pushobject(buffer->at(i));
            lua_settable(L, -3);
        }
        co_return 1;
    };
    return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto base_hp  = std::make_shared<uint32_t>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *base_hp = ch->stat.base_hp();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*base_hp);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->stat.base_hp(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto base_mp  = std::make_shared<uint32_t>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *base_mp = ch->stat.base_mp();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*base_mp);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->stat.base_mp(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto base_str = std::make_shared<uint8_t>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *base_str = ch->stat.base_str();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*base_str);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->stat.base_str(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto base_dex = std::make_shared<uint8_t>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *base_dex = ch->stat.base_dex();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*base_dex);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->stat.base_dex(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto base_int = std::make_shared<uint8_t>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *base_int = ch->stat.base_int();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*base_int);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->stat.base_int(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto base_dam = std::make_shared<uint8_t>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *base_dam = ch->stat.base_dam();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*base_dam);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->stat.base_dam(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto base_hit = std::make_shared<uint8_t>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *base_hit = ch->stat.base_hit();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*base_hit);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->stat.base_hit(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto result   = std::make_shared<std::optional<uint8_t>>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *result = ch->armor_color();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            if (result->has_value())
                lua->pushinteger(result->value());
            else
                lua->pushnil();
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value = std::optional<uint8_t>{};
        if (lua->is_number(2))
            value = lua->tointeger(2);

        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->armor_color(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::character::builtin_weapon_color(lua_State* L)
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto result   = std::make_shared<std::optional<uint8_t>>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *result = ch->weapon_color();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            if (result->has_value())
                lua->pushinteger(static_cast<lua_Integer>(result->value()));
            else
                lua->pushnil();
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value = std::optional<uint8_t>{};
        if (lua->is_number(2))
            value = static_cast<uint8_t>(lua->tointeger(2));

        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->weapon_color(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::character::builtin_shield_color(lua_State* L)
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto result   = std::make_shared<std::optional<uint8_t>>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *result = ch->shield_color();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            if (result->has_value())
                lua->pushinteger(static_cast<lua_Integer>(result->value()));
            else
                lua->pushnil();
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value = std::optional<uint8_t>{};
        if (lua->is_number(2))
            value = static_cast<uint8_t>(lua->tointeger(2));

        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->shield_color(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto slot     = std::make_shared<uint8_t>(0xFF);
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *slot = ch->spells.add(*model);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*slot == 0xFF)
            co_return 0;
        lua->pushinteger(*slot);
        co_return 1;
    };
    return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            for (int i = 0; i < CONTAINER_CAPACITY; i++)
            {
                auto spell = ch->spells[i];
                if (spell == nullptr)
                    continue;

                ch->spells.remove(i);
            }
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
    else if (lua->is_number(2))
    {
        auto slot     = lua->tointeger(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->spells.remove(slot);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
    else if (lua->is_string(2))
    {
        auto spell_name = lua->tostring(2);
        auto weak       = ch->weak_from_this_as<fb::game::character>();
        auto builder    = lua->new_co_builder(*server);
        builder.weak    = weak;
        builder.yield   = [=]() -> async::task<void> {
            auto slot = uint8_t{0xFF};
            for (int i = 0; i < CONTAINER_CAPACITY; i++)
            {
                auto spell = ch->spells[i];
                if (spell == nullptr)
                    continue;

                if (spell->model.name == spell_name)
                {
                    slot = i;
                    break;
                }
            }

            if (slot != 0xFF)
                ch->spells.remove(slot);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
    else
    {
        return 0;
    }
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
                auto weak     = ch->weak_from_this_as<fb::game::character>();
                auto builder  = lua->new_co_builder(*server);
                builder.weak  = weak;
                builder.yield = [=]() -> async::task<void> {
                    ch->show_world_map(id, index);
                    co_return;
                };
                builder.resume = [=]() -> async::task<int> {
                    lua->pushboolean(true);
                    co_return 1;
                };
                return builder.run();
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
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto width  = lua->tointeger(2, 300);
    auto height = lua->tointeger(3, 120);
    auto url    = lua->tostring(4, "http://www.google.com");
    auto time   = lua->tointeger(5, 60);

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        ch->send(fb::protocol::game::response::ad(width, height, url, time));
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::character::builtin_web(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto type    = lua->tointeger(2, 0);
    auto url     = lua->tostring(3, "http://www.google.com");
    auto message = lua->tostring(4, "default message");

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        ch->send(fb::protocol::game::response::web(type, url, message));
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::character::builtin_unknown_12(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto oid           = static_cast<uint32_t>(lua->tointeger(2, 0));
    auto party_slot    = static_cast<uint8_t>(lua->tointeger(3, 0));
    auto level_encoded = static_cast<uint8_t>(lua->tointeger(4, 1));

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        ch->send(fb::protocol::game::response::unknown_12(oid, party_slot, level_encoded));
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::character::builtin_unknown_26(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto flags     = static_cast<uint8_t>(lua->tointeger(2, 0));
    auto pos_x     = static_cast<int16_t>(lua->tointeger(3, 0));
    auto pos_y     = static_cast<int16_t>(lua->tointeger(4, 0));
    auto map_rel_x = static_cast<int16_t>(lua->tointeger(5, 0));
    auto map_rel_y = static_cast<int16_t>(lua->tointeger(6, 0));
    auto zone_slot = static_cast<uint8_t>(lua->tointeger(7, 0));

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        ch->send(fb::protocol::game::response::unknown_26(flags, pos_x, pos_y, map_rel_x, map_rel_y, zone_slot));
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::character::builtin_ui(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    using UI_SCREEN = fb::protocol::game::response::UI_SCREEN;
    auto screen     = static_cast<UI_SCREEN>(lua->tointeger(2, 0));

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        ch->send(fb::protocol::game::response::ui_screen(screen));
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::character::builtin_item_throw_confirm(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto slot = static_cast<uint8_t>(lua->tointeger(2, 0));

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        ch->send(fb::protocol::game::response::item_throw_confirm(slot));
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::character::builtin_freeze(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto freeze = lua->toboolean(2, false);

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        ch->send(fb::protocol::game::response::freeze(freeze));
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::character::builtin_friends_sync(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto enabled = static_cast<uint8_t>(lua->tointeger(2, 0));

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        ch->send(fb::protocol::game::response::friends_sync(enabled));
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::character::builtin_unknown_4B(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto payload = lua->tostring(2, "");

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        ch->send(fb::protocol::game::response::unknown_4B(payload));
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::character::builtin_unknown_4D(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto                       type = static_cast<uint8_t>(lua->tointeger(2, 0));
    std::array<std::string, 8> strings;
    const char*                default_str = "-";
    for (size_t i = 0; i < strings.size(); ++i)
    {
        strings[i] = lua->tostring(static_cast<int>(3 + i), default_str);
    }

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        ch->send(fb::protocol::game::response::unknown_4D(type, strings));
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::character::builtin_holyday_screen(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto screen    = static_cast<uint8_t>(lua->tointeger(2, 0));
    auto direction = static_cast<fb::model::enum_value::DIRECTION>(lua->tointeger(3, 0));
    auto x         = uint8_t(0);
    auto y         = uint8_t(0);
    if (lua->is_table(4))
    {
        lua->rawgeti(4, 1);
        if (lua->is_number(-1))
            x = static_cast<uint8_t>(lua->tointeger(-1));
        lua->pop(1);
        lua->rawgeti(4, 2);
        if (lua->is_number(-1))
            y = static_cast<uint8_t>(lua->tointeger(-1));
        lua->pop(1);
    }

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto hair = ch->look();
        ch->send(
            fb::protocol::game::response::holyday_screen(screen, hair, direction, fb::model::point<uint8_t>(x, y)));
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

int builtin::character::builtin_unknown_35(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        ch->send(fb::protocol::game::response::unknown_35());
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
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

    auto value    = lua->tointeger(3);
    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto spell = ch->spells.find(*model);
        if (spell != nullptr)
            spell->delay(value);
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto result   = std::make_shared<std::optional<std::uint32_t>>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *result = ch->birthday();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            if (result->has_value())
                lua->pushinteger(result->value());
            else
                lua->pushnil();
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->tointeger(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->birthday(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
    }
}

int builtin::character::builtin_active(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    auto item = lua->touserdata<fb::game::item>(2);
    if (item == nullptr)
        return 0;

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto slot = ch->items.index(item);
        if (slot != 0xFF)
            ch->items.active(slot);
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto result   = std::make_shared<bool>(false);
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *result = ch->super_hide();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushboolean(*result);
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = lua->toboolean(2);
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            ch->super_hide(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
        return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        std::ignore = server->mail.send(*ch, to, title, contents);
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
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
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto creature = std::make_shared<CREATURE>();
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *creature = ch->creature();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(static_cast<uint8_t>(*creature));
            co_return 1;
        };
        return builder.run();
    }
    else
    {
        auto value    = static_cast<CREATURE>(lua->tointeger(2));
        auto weak     = ch->weak_from_this_as<fb::game::character>();
        auto success  = std::make_shared<bool>(false);
        auto builder  = lua->new_co_builder(*server);
        builder.weak  = weak;
        builder.yield = [=]() -> async::task<void> {
            *success = ch->creature(value);
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushboolean(*success);
            co_return 1;
        };
        return builder.run();
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
        auto success_holder = std::make_shared<bool>(false);
        auto builder        = lua->new_co_builder(*server);
        builder.weak        = target_weak;
        builder.yield       = [=]() -> async::task<void> {
            auto target_locked = target_weak.lock();
            if (target_locked == nullptr)
                co_return;

            auto map = target_locked->map();
            if (map == nullptr)
                throw std::runtime_error("teleport target has no map");

            auto position = target_locked->position();

            co_await server->threads.switching(ch_weak);

            auto ch_locked = ch_weak.lock();
            if (ch_locked == nullptr)
                co_return;

            *success_holder = co_await ch_locked->map(map, position);
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushboolean(*success_holder);
            co_return 1;
        };
        return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
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
    };
    return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        if (appearance != nullptr)
            ch->listener
                .on_dialog(*ch, std::unique_ptr<fb::game::appearance>(appearance), message, menus, button_prev, oid);
        else if (obj != nullptr)
            ch->listener.on_dialog(*ch, *obj, message, menus, button_prev, oid);
        else
            ch->listener.on_dialog(*ch, *model, message, menus, button_prev, oid);

        if (ch->dialog != nullptr)
            ch->dialog->release();

        ch->dialog = lua;
        co_return;
    };
    return builder.run();
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
    auto weak    = ch->weak_from_this_as<fb::game::character>();
    auto builder = lua->new_co_builder(*server);
    builder.weak = weak;
    if (argc > 3)
    {
        auto message_top = lua->tostring(4);
        auto message_bot = lua->tostring(5);
        auto maxlen      = (uint8_t)lua->tointeger(6, 0xFF);
        auto prev        = lua->toboolean(7, false);

        builder.yield = [=]() -> async::task<void> {
            ch->listener.on_dialog(*ch, *model, message, message_top, message_bot, maxlen, prev, oid);
            if (ch->dialog != nullptr)
                ch->dialog->release();

            ch->dialog = lua;
            co_return;
        };
    }
    else
    {
        builder.yield = [=]() -> async::task<void> {
            if (obj != nullptr)
                ch->listener.on_dialog(*ch, *obj, message, oid);
            else
                ch->listener.on_dialog(*ch, *model, message, oid);

            if (ch->dialog != nullptr)
                ch->dialog->release();

            ch->dialog = lua;
            co_return;
        };
    }
    return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        if (obj != nullptr)
            ch->listener.on_dialog(*ch, *obj, message, menus, oid);
        else
            ch->listener.on_dialog(*ch, *model, message, menus, oid);

        if (ch->dialog != nullptr)
            ch->dialog->release();

        ch->dialog = lua;
        co_return;
    };
    return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        if (obj != nullptr)
            ch->listener.on_dialog(*ch, *obj, message, slots, oid);
        else
            ch->listener.on_dialog(*ch, *model, message, slots, oid);

        if (ch->dialog != nullptr)
            ch->dialog->release();

        ch->dialog = lua;
        co_return;
    };
    return builder.run();
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

    auto is_global = lua->toboolean(2, false);

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        if (is_global)
        {
            server->maps.rezen_force();
        }
        else
        {
            auto map = ch->map();
            if (map != nullptr)
                map->rezen_force();
        }
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
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

    auto id       = lua->tointeger(2);
    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto result   = std::make_shared<std::shared_ptr<fb::game::quest>>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        if (ch->quests.contains(id))
            *result = ch->quests[id];
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*result == nullptr)
            lua->pushnil();
        else
            lua->pushobject(*result);
        co_return 1;
    };
    return builder.run();
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

    auto id       = lua->tointeger(2);
    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto result   = std::make_shared<std::shared_ptr<fb::game::quest>>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        if (ch->quests.start(id) && ch->quests.contains(id))
            *result = ch->quests[id];
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (*result == nullptr)
            lua->pushnil();
        else
            lua->pushobject(*result);
        co_return 1;
    };
    return builder.run();
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

    std::optional<uint32_t> quest_id;
    if (lua->is_number(2))
    {
        quest_id = static_cast<uint32_t>(lua->tointeger(2));
    }
    else if (lua->is_userdata<fb::game::quest>(2))
    {
        auto quest = lua->touserdata<fb::game::quest>(2);
        if (quest != nullptr)
            quest_id = quest->id;
    }

    if (quest_id.has_value() == false)
    {
        lua->pushboolean(false);
        return 1;
    }

    auto id       = quest_id.value();
    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto result   = std::make_shared<bool>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *result = ch->quests.remove(id);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
}

int fb::game::builtin::character::builtin_reward(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
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

    auto reward_dsl = table::reward[reward_id].dsl;
    auto weak       = ch->weak_from_this_as<fb::game::character>();
    auto result     = std::make_shared<bool>();
    auto builder    = lua->new_co_builder(*server);
    builder.weak    = weak;
    builder.yield   = [=]() -> async::task<void> {
        *result = ch->reward(reward_dsl);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*result);
        co_return 1;
    };
    return builder.run();
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

    auto success  = std::make_shared<bool>(false);
    auto builder  = lua->new_co_builder(*server);
    builder.yield = [=]() -> async::task<void> {
        *success = co_await server->system_mail.create(ch->id, title, contents, expire_date);
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*success);
        co_return 1;
    };
    return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto buffer   = std::make_shared<std::vector<fb::game::storage_box::entry>>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        buffer->reserve(ch->storage_box.entries().size());
        auto now = server->now();
        for (const auto& [id, entry] : ch->storage_box.entries())
        {
            if (entry.attachments.empty())
                continue;
            if (entry.received)
                continue;
            if (entry.expire_date.has_value() && entry.expire_date.value() < now)
                continue;
            buffer->push_back(entry);
        }

        std::sort(buffer->begin(), buffer->end(), [](const auto& lhs, const auto& rhs) {
            return lhs.id > rhs.id;
        });
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->new_table();
        auto i = 0;
        for (const auto& entry : *buffer)
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
                lua->pushstring(entry.expire_date.value().to_string());
            else
                lua->pushnil();
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
        co_return 1;
    };
    return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto success  = std::make_shared<bool>(false);
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        *success = ch->storage_box.receive_reward(entry_id);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushboolean(*success);
        co_return 1;
    };
    return builder.run();
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

    auto weak           = ch->weak_from_this_as<fb::game::character>();
    auto error          = std::make_shared<std::optional<std::string>>();
    auto listing_holder = std::make_shared<std::optional<fb::game::marketplace::listing>>();
    auto builder        = lua->new_co_builder(*server);
    builder.weak        = weak;
    builder.yield       = [=]() -> async::task<void> {
        try
        {
            auto shared = weak.lock();
            if (shared == nullptr)
                throw std::runtime_error("Character is not alive");

            listing_holder->emplace(co_await shared->marketplace.list(item_index, count, price, expire_hours));
        }
        catch (std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
        {
            lua->pushstring(error->value().c_str());
            lua->pushnil();
        }
        else
        {
            lua->pushnil();
            listing_holder->value().to_lua(lua);
        }
        co_return 2;
    };
    return builder.run();
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

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto error    = std::make_shared<std::optional<std::string>>();
    auto success  = std::make_shared<std::optional<bool>>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        try
        {
            auto shared = weak.lock();
            if (shared == nullptr)
                throw std::runtime_error("Character is not alive");

            *success = co_await shared->marketplace.cancel(listing_id);
        }
        catch (std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
        {
            lua->pushstring(error->value().c_str());
            lua->pushnil();
        }
        else
        {
            lua->pushnil();
            lua->pushboolean(success->value_or(false));
        }
        co_return 2;
    };
    return builder.run();
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

    auto weak           = ch->weak_from_this_as<fb::game::character>();
    auto error          = std::make_shared<std::optional<std::string>>();
    auto listing_holder = std::make_shared<std::optional<fb::game::marketplace::listing>>();
    auto builder        = lua->new_co_builder(*server);
    builder.weak        = weak;
    builder.yield       = [=]() -> async::task<void> {
        try
        {
            auto shared = weak.lock();
            if (shared == nullptr)
                throw std::runtime_error("Character is not alive");

            listing_holder->emplace(
                co_await shared->marketplace.purchase(listing_id, static_cast<uint16_t>(purchase_count)));
        }
        catch (std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
        {
            lua->pushstring(error->value().c_str());
            lua->pushnil();
        }
        else
        {
            lua->pushnil();
            listing_holder->value().to_lua(lua);
        }
        co_return 2;
    };
    return builder.run();
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

    auto weak          = ch->weak_from_this_as<fb::game::character>();
    auto error         = std::make_shared<std::optional<std::string>>();
    auto result_holder = std::make_shared<std::optional<fb::game::marketplace::search_result>>();
    auto builder       = lua->new_co_builder(*server);
    builder.weak       = weak;
    builder.yield      = [=]() -> async::task<void> {
        try
        {
            auto shared = weak.lock();
            if (shared == nullptr)
                throw std::runtime_error("Character is not alive");

            result_holder->emplace(co_await shared->marketplace.search(option));
        }
        catch (std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
        {
            lua->pushstring(error->value().c_str());
            lua->pushnil();
            co_return 2;
        }

        lua->pushnil();
        lua->new_table();

        const auto& result = result_holder->value();

        lua_pushstring(*lua, "listings");
        lua->new_table();
        for (size_t i = 0; i < result.listings.size(); i++)
        {
            lua->pushinteger(i + 1);
            result.listings[i].to_lua(lua);
            lua_settable(*lua, -3);
        }
        lua_settable(*lua, -3);

        lua_pushstring(*lua, "total_count");
        lua->pushinteger(result.total_count);
        lua_settable(*lua, -3);

        lua_pushstring(*lua, "page");
        lua->pushinteger(result.page);
        lua_settable(*lua, -3);

        co_return 2;
    };
    return builder.run();
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

    auto weak            = ch->weak_from_this_as<fb::game::character>();
    auto error           = std::make_shared<std::optional<std::string>>();
    auto listings_holder = std::make_shared<std::vector<fb::game::marketplace::listing>>();
    auto builder         = lua->new_co_builder(*server);
    builder.weak         = weak;
    builder.yield        = [=]() -> async::task<void> {
        try
        {
            auto shared = weak.lock();
            if (shared == nullptr)
                throw std::runtime_error("Character is not alive");

            *listings_holder = co_await shared->marketplace.get_listings(listing_ids);
        }
        catch (std::exception& e)
        {
            *error = e.what();
        }
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
        {
            lua->pushstring(error->value().c_str());
            lua->pushnil();
            co_return 2;
        }

        lua->pushnil();
        lua->new_table();
        for (size_t i = 0; i < listings_holder->size(); i++)
        {
            lua->pushinteger(i + 1);
            (*listings_holder)[i].to_lua(lua);
            lua_settable(*lua, -3);
        }
        co_return 2;
    };
    return builder.run();
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

    auto weak    = ch->weak_from_this_as<fb::game::character>();
    auto buffer  = std::make_shared<std::vector<std::pair<std::string, fb::game::marketplace::pending_listing_info>>>();
    auto builder = lua->new_co_builder(*server);
    builder.weak = weak;
    builder.yield = [=]() -> async::task<void> {
        const auto& pending_listings = ch->marketplace.pending_listings();
        buffer->reserve(pending_listings.size());
        for (const auto& [listing_id, pending_info] : pending_listings)
            buffer->push_back({listing_id, pending_info});
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->new_table();
        auto i = 0;
        for (const auto& [listing_id, pending_info] : *buffer)
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
        co_return 1;
    };
    return builder.run();
}

int builtin::character::builtin_marriage(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto server = lua->env<fb::game::server>("server");
    auto ch     = lua->touserdata<fb::game::character>(1);
    if (ch == nullptr)
        return 0;

    struct marriage_snapshot
    {
        bool                    married     = false;
        bool                    can_remarry = false;
        std::optional<uint32_t> spouse_id;
        std::string             spouse_name;
        uint32_t                divorce_count = 0;
    };

    auto weak     = ch->weak_from_this_as<fb::game::character>();
    auto snapshot = std::make_shared<marriage_snapshot>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        const auto& m           = ch->marriage();
        snapshot->married       = m.spouse_id.has_value();
        snapshot->can_remarry   = !snapshot->married && (ch->server.now() >= m.remarriage_after);
        snapshot->spouse_id     = m.spouse_id;
        snapshot->spouse_name   = m.spouse_name;
        snapshot->divorce_count = m.divorce_count;
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->new_table();
        lua_pushstring(*lua, "married");
        lua->pushboolean(snapshot->married);
        lua_settable(*lua, -3);
        lua_pushstring(*lua, "spouse_id");
        if (snapshot->spouse_id.has_value())
            lua->pushinteger(static_cast<int64_t>(snapshot->spouse_id.value()));
        else
            lua->pushnil();
        lua_settable(*lua, -3);
        lua_pushstring(*lua, "spouse_name");
        lua->pushstring(snapshot->spouse_name);
        lua_settable(*lua, -3);
        lua_pushstring(*lua, "divorce_count");
        lua->pushinteger(static_cast<int64_t>(snapshot->divorce_count));
        lua_settable(*lua, -3);
        lua_pushstring(*lua, "can_remarry");
        lua->pushboolean(snapshot->can_remarry);
        lua_settable(*lua, -3);
        co_return 1;
    };
    return builder.run();
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

    auto weak     = me->weak_from_this_as<fb::game::character>();
    auto error    = std::make_shared<std::optional<std::string>>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        const auto& my_m  = me->marriage();
        const auto& tar_m = target->marriage();

        if (my_m.spouse_id.has_value())
        {
            *error = "already married";
            co_return;
        }
        if (tar_m.spouse_id.has_value())
        {
            *error = "target already married";
            co_return;
        }
        if (server->now() < my_m.remarriage_after)
        {
            *error = "remarriage cooldown";
            co_return;
        }
        if (server->now() < tar_m.remarriage_after)
        {
            *error = "target remarriage cooldown";
            co_return;
        }

        fb::game::marriage new_me(server->now(), target->id, target->name(), my_m.divorce_count);
        fb::game::marriage new_tar(server->now(), me->id, me->name(), tar_m.divorce_count);

        me->marriage(new_me);
        target->marriage(new_tar);
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
            lua->pushstring(error->value());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
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

    auto weak     = me->weak_from_this_as<fb::game::character>();
    auto error    = std::make_shared<std::optional<std::string>>();
    auto builder  = lua->new_co_builder(*server);
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        const auto& m = me->marriage();
        if (!m.spouse_id.has_value())
        {
            *error = "not married";
            co_return;
        }

        fb::game::character::container::character_ptr_t spouse;
        {
            auto guard = server->characters.enter_read();
            spouse     = guard.value().find(m.spouse_id.value());
        }

        if (spouse == nullptr)
        {
            *error = "spouse not online";
            co_return;
        }

        auto now = server->now();
        now.add_days(7);

        fb::game::marriage new_me(now, std::nullopt, "", m.divorce_count + 1);
        fb::game::marriage new_tar(now, std::nullopt, "", spouse->marriage().divorce_count + 1);

        me->marriage(new_me);
        if (spouse->thread() == me->thread())
        {
            spouse->marriage(new_tar);
        }
        else
        {
            auto spouse_weak = spouse->weak_from_this_as<fb::game::character>();
            auto cross       = server->threads.new_builder(spouse_weak);
            cross.func       = [spouse_weak, new_tar](auto&) -> async::task<void> {
                auto spouse_shared = spouse_weak.lock();
                if (spouse_shared != nullptr)
                    spouse_shared->marriage(new_tar);
                co_return;
            };
            cross.enqueue();
        }
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        if (error->has_value())
            lua->pushstring(error->value());
        else
            lua->pushnil();
        co_return 1;
    };
    return builder.run();
}