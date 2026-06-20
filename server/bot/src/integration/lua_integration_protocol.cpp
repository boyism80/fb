#include <fb/bot/integration/lua_integration_protocol.h>
#include <fb/bot/integration/protocol_registry.h>
#include <fb/bot/integration/protocol_lua.generated.h>
#include <fb/game/protocol.h>
#include <fb/logger.h>
#include <fb/model/model.h>
#include <fb/protocol/header.h>

#include <fb/lua.h>

#include <cstring>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fb::bot::integration::lua_protocol {

namespace {

fb::lua::context& require_lua(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        luaL_error(L, "integration protocol requires active lua context");

    return *lua;
}

struct request_holder
{
    std::shared_ptr<fb::protocol::header> value;
};

struct response_token
{
    const protocol_entry* entry;
};

int request_holder_gc(lua_State* L)
{
    auto* holder = static_cast<request_holder*>(luaL_checkudata(L, 1, REQUEST_METATABLE));
    holder->~request_holder();
    return 0;
}

void push_recipe_dsl_items(fb::lua::context& lua, const std::vector<fb::model::dsl>& dsls)
{
    lua.new_table();
    auto index = 1;
    for (auto& dsl : dsls)
    {
        if (dsl.header != fb::model::enum_value::DSL::item)
            continue;

        auto  params = fb::model::dsl::item(dsl.params);
        auto& item   = fb::model::table::item[params.id];

        lua.pushinteger(index++);
        lua.new_table();
        lua.pushstring("name");
        lua.pushstring(item.name);
        lua.settable(-3);
        lua.pushstring("count");
        lua.pushinteger(params.count);
        lua.settable(-3);
        lua.pushstring("capacity");
        lua.pushinteger(item.capacity);
        lua.settable(-3);
        lua.settable(-3);
    }
}

static int lua_model_promotions(lua_State* L)
{
    auto& lua = require_lua(L);

    lua.new_table();
    auto index = 1;
    for (auto& [cls, promotions] : fb::model::table::promotion)
    {
        for (auto& [_, promotion] : promotions)
        {
            lua.pushinteger(index++);
            lua.new_table();
            lua.pushstring("name");
            lua.pushstring(promotion.name);
            lua.settable(-3);
            lua.pushstring("cls");
            lua.pushinteger(static_cast<lua_Integer>(static_cast<uint8_t>(cls)));
            lua.settable(-3);
            lua.settable(-3);
        }
    }
    return 1;
}

static int lua_model_world_destination(lua_State* L)
{
    auto& lua    = require_lua(L);
    auto  parent = static_cast<uint16_t>(lua.tointeger(1));
    auto  id     = static_cast<uint16_t>(lua.tointeger(2));
    auto& cell   = fb::model::table::world[parent][id];

    lua.new_table();
    lua.pushstring("map");
    lua.pushinteger(cell.map);
    lua.settable(-3);

    lua.pushstring("position");
    lua.new_table();
    lua.pushinteger(1);
    lua.pushinteger(cell.position.x);
    lua.settable(-3);
    lua.pushinteger(2);
    lua.pushinteger(cell.position.y);
    lua.settable(-3);
    lua.settable(-3);
    return 1;
}

static int lua_model_equipment_items(lua_State* L)
{
    using namespace fb::model::enum_value;

    auto& lua = require_lua(L);

    static const std::unordered_set<ITEM_TYPE> equipment_types = {ITEM_TYPE::WEAPON,
                                                                  ITEM_TYPE::ARMOR,
                                                                  ITEM_TYPE::HELMET,
                                                                  ITEM_TYPE::RING,
                                                                  ITEM_TYPE::SHIELD,
                                                                  ITEM_TYPE::AUXILIARY};

    lua.new_table();
    auto index = 1;
    for (auto& [_, item] : fb::model::table::item)
    {
        if (equipment_types.contains(item.type) == false)
            continue;

        lua.pushinteger(index++);
        lua.new_table();
        lua.pushstring("name");
        lua.pushstring(item.name);
        lua.settable(-3);
        lua.pushstring("type");
        lua.pushstring(enum_tostring(item.type));
        lua.settable(-3);
        lua.pushstring("has_condition");
        lua.pushboolean(item.condition.empty() == false);
        lua.settable(-3);
        lua.settable(-3);
    }
    return 1;
}

static int lua_model_recipes(lua_State* L)
{
    auto& lua = require_lua(L);

    lua.new_table();
    auto index = 1;
    for (auto& recipe : fb::model::table::recipe)
    {
        lua.pushinteger(index++);
        lua.new_table();

        lua.pushstring("source");
        push_recipe_dsl_items(lua, recipe.source);
        lua.settable(-3);

        lua.pushstring("success");
        push_recipe_dsl_items(lua, recipe.success);
        lua.settable(-3);

        lua.pushstring("failed");
        push_recipe_dsl_items(lua, recipe.failed);
        lua.settable(-3);

        lua.settable(-3);
    }
    return 1;
}

} // namespace

void register_metatables(lua_State* L)
{
    luaL_newmetatable(L, REQUEST_METATABLE);
    lua_pushcfunction(L, request_holder_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    luaL_newmetatable(L, RESPONSE_METATABLE);
    lua_pop(L, 1);
}

static int luaopen_integration_response(lua_State* L)
{
    lua_newtable(L);
    for (size_t i = 0; i < protocol_registry::entry_count(); ++i)
    {
        auto& entry = protocol_registry::entries()[i];
        if (entry.direction != protocol_direction::response)
            continue;

        push_response_token(L, &entry);
        lua_setfield(L, -2, entry.name);
    }
    return 1;
}

static int luaopen_integration_protocol(lua_State* L)
{
    register_builders(L);
    register_manual_builders(L);
    return 1;
}

static int luaopen_integration_model(lua_State* L)
{
    lua_newtable(L);

    lua_pushcfunction(L, lua_model_promotions);
    lua_setfield(L, -2, "promotions");

    lua_pushcfunction(L, lua_model_world_destination);
    lua_setfield(L, -2, "world_destination");

    lua_pushcfunction(L, lua_model_equipment_items);
    lua_setfield(L, -2, "equipment_items");

    lua_pushcfunction(L, lua_model_recipes);
    lua_setfield(L, -2, "recipes");

    return 1;
}

void register_modules(lua_State* L)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");

    lua_pushcfunction(L, luaopen_integration_response);
    lua_setfield(L, -2, "integration.response");

    lua_pushcfunction(L, luaopen_integration_protocol);
    lua_setfield(L, -2, "integration.protocol");

    lua_pushcfunction(L, luaopen_integration_model);
    lua_setfield(L, -2, "integration.model");

    lua_pop(L, 2);
}

void register_modules(fb::lua::root& root)
{
    root.preload("integration.response", luaopen_integration_response);
    root.preload("integration.protocol", luaopen_integration_protocol);
    root.preload("integration.model", luaopen_integration_model);
}

const protocol_entry* to_response_token(lua_State* L, int index)
{
    auto* token = static_cast<response_token*>(luaL_checkudata(L, index, RESPONSE_METATABLE));
    return token->entry;
}

std::shared_ptr<fb::protocol::header> to_request(lua_State* L, int index)
{
    auto* holder = static_cast<request_holder*>(luaL_checkudata(L, index, REQUEST_METATABLE));
    return holder->value;
}

void push_request(lua_State* L, std::shared_ptr<fb::protocol::header> request)
{
    auto* storage = lua_newuserdata(L, sizeof(request_holder));
    new (storage) request_holder{std::move(request)};
    luaL_getmetatable(L, REQUEST_METATABLE);
    lua_setmetatable(L, -2);
}

void push_response_token(lua_State* L, const protocol_entry* entry)
{
    auto* token  = static_cast<response_token*>(lua_newuserdata(L, sizeof(response_token)));
    token->entry = entry;
    luaL_getmetatable(L, RESPONSE_METATABLE);
    lua_setmetatable(L, -2);
}

int lua_builder_dialog(lua_State* L)
{
    auto& lua = require_lua(L);

    namespace game_reqs = fb::protocol::game::request;
    using namespace fb::model::enum_value;

    auto parse_interaction = [](std::string_view value) -> game_reqs::dialog::INTERACTION {
        static const std::unordered_map<std::string_view, game_reqs::dialog::INTERACTION> values = {
            {"NORMAL",   game_reqs::dialog::INTERACTION::NORMAL  },
            {"INPUT",    game_reqs::dialog::INTERACTION::INPUT   },
            {"INPUT_EX", game_reqs::dialog::INTERACTION::INPUT_EX},
            {"MENU",     game_reqs::dialog::INTERACTION::MENU    },
            {"LIST",     game_reqs::dialog::INTERACTION::LIST    },
            {"SLOT",     game_reqs::dialog::INTERACTION::SLOT    },
            {"ITEM",     game_reqs::dialog::INTERACTION::ITEM    },
        };

        auto i = values.find(value);
        if (i == values.end())
            throw std::runtime_error(std::format("{} is not a valid dialog interaction", value));

        return i->second;
    };

    auto interaction = game_reqs::dialog::INTERACTION::NORMAL;
    if (lua.argc() >= 1 && lua.is_nil(1) == false)
        interaction = parse_interaction(lua.tostring(1));

    auto action  = static_cast<uint8_t>(lua.tointeger(2, 0));
    auto message = lua.argc() >= 3 && lua.is_nil(3) == false ? lua.tostring(3) : std::string{};
    auto index   = static_cast<uint16_t>(lua.tointeger(4, 0));
    auto pursuit = static_cast<uint16_t>(lua.tointeger(5, 0));
    auto name    = lua.argc() >= 6 && lua.is_nil(6) == false ? lua.tostring(6) : std::string{};
    auto button  = DIALOG_RESULT::NEXT;
    if (lua.argc() >= 7 && lua.is_nil(7) == false)
        button = enum_parse<DIALOG_RESULT>(lua.tostring(7));

    push_request(L, std::make_shared<game_reqs::dialog>(interaction, action, message, index, pursuit, name, button));
    return 1;
}

int lua_builder_trade(lua_State* L)
{
    try
    {
        auto& lua = require_lua(L);

        namespace game_reqs = fb::protocol::game::request;
        using trade_state   = game_reqs::trade::state;

        static const std::unordered_map<std::string_view, trade_state> states = {
            {"REQUEST",    trade_state::REQUEST   },
            {"UP_ITEM",    trade_state::UP_ITEM   },
            {"ITEM_COUNT", trade_state::ITEM_COUNT},
            {"UP_MONEY",   trade_state::UP_MONEY  },
            {"CANCEL",     trade_state::CANCEL    },
            {"LOCK",       trade_state::LOCK      },
        };

        auto action_name = std::string_view(luaL_checkstring(L, 1));
        auto i           = states.find(action_name);
        if (i == states.end())
            return luaL_error(L,
                              "%.*s is not a valid trade action",
                              static_cast<int>(action_name.size()),
                              action_name.data());

        auto oid = static_cast<uint32_t>(luaL_checkinteger(L, 2));

        fb::logger::debug("protocol.trade action={} oid={}", action_name, oid);

        game_reqs::trade::params parameter{};
        if (lua.argc() >= 3 && lua.is_nil(3) == false)
        {
            if (lua.is_table(3) == false)
                return luaL_error(L, "trade parameter must be a table");

            lua_getfield(L, 3, "index");
            if (lua.is_nil(-1) == false)
                parameter.index = static_cast<uint8_t>(lua.tointeger(-1));
            lua.pop(1);

            lua_getfield(L, 3, "count");
            if (lua.is_nil(-1) == false)
                parameter.count = static_cast<uint16_t>(lua.tointeger(-1));
            lua.pop(1);

            lua_getfield(L, 3, "money");
            if (lua.is_nil(-1) == false)
                parameter.money = static_cast<uint32_t>(lua.tointeger(-1));
            lua.pop(1);
        }

        push_request(L, std::make_shared<game_reqs::trade>(i->second, oid, parameter));
        return 1;
    }
    catch (const std::exception& e)
    {
        auto* what = e.what();
        return luaL_error(L, "protocol.trade failed: %s", (what != nullptr && what[0] != '\0') ? what : "unknown");
    }
    catch (...)
    {
        return luaL_error(L, "protocol.trade failed: non-std exception");
    }
}

int lua_builder_move(lua_State* L)
{
    auto& lua = require_lua(L);

    namespace game_reqs = fb::protocol::game::request;
    using namespace fb::model::enum_value;

    auto direction = enum_parse<DIRECTION>(lua.tostring(1));
    auto oid       = static_cast<uint32_t>(lua.tointeger(2));

    fb::model::point<uint16_t> position{};
    if (lua.is_table(3))
    {
        lua.rawgeti(3, 1);
        position.x = static_cast<uint16_t>(lua.tointeger(-1));
        lua.pop(1);
        lua.rawgeti(3, 2);
        position.y = static_cast<uint16_t>(lua.tointeger(-1));
        lua.pop(1);
    }
    else
    {
        position.x = static_cast<uint16_t>(lua.tointeger(3));
        position.y = static_cast<uint16_t>(lua.tointeger(4));
    }

    push_request(L, std::make_shared<game_reqs::move>(direction, oid, position));
    return 1;
}

void register_manual_builders(lua_State* L)
{
    namespace game_reqs = fb::protocol::game::request;
    using namespace fb::model::enum_value;

    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto& lua   = require_lua(L);
        auto  value = static_cast<uint8_t>(lua.tointeger(1));
        push_request(L, std::make_shared<game_reqs::emotion>(value));
        return 1;
    });
    lua_setfield(L, -2, "emotion");

    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto& lua     = require_lua(L);
        auto  shout   = lua.toboolean(1);
        auto  message = lua.tostring(2);
        push_request(L, std::make_shared<game_reqs::chat>(shout, message));
        return 1;
    });
    lua_setfield(L, -2, "chat");

    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto& lua      = require_lua(L);
        auto  action   = enum_parse<BULLETIN_ACTION>(lua.tostring(1));
        auto  section  = static_cast<uint16_t>(lua.tointeger(2, 0));
        auto  article  = static_cast<uint16_t>(lua.tointeger(3, 0));
        auto  offset   = static_cast<uint16_t>(lua.tointeger(4, 0));
        auto  title    = lua.argc() >= 5 && lua.is_nil(5) == false ? lua.tostring(5) : std::string{};
        auto  contents = lua.argc() >= 6 && lua.is_nil(6) == false ? lua.tostring(6) : std::string{};
        auto  user     = lua.argc() >= 7 && lua.is_nil(7) == false ? lua.tostring(7) : std::string{};
        push_request(L, std::make_shared<game_reqs::bulletin>(action, section, article, offset, title, contents, user));
        return 1;
    });
    lua_setfield(L, -2, "bulletin");

    lua_pushcfunction(L, lua_builder_dialog);
    lua_setfield(L, -2, "dialog");

    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto&      lua   = require_lua(L);
        const auto boost = lua.toboolean(1);
        push_request(L, std::make_shared<game_reqs::loot>(boost));
        return 1;
    });
    lua_setfield(L, -2, "loot");

    lua_pushcfunction(L, lua_builder_trade);
    lua_setfield(L, -2, "trade");

    lua_pushcfunction(L, lua_builder_move);
    lua_setfield(L, -2, "move");

    lua_pushcfunction(L, [](lua_State* L) -> int {
        auto& lua = require_lua(L);

        if (lua.is_table(1) == false)
            luaL_error(L, "item_combine expects a table of slot indices");

        std::vector<uint8_t> indices;
        for (int i = 1;; ++i)
        {
            lua.rawgeti(1, i);
            if (lua.is_nil(-1))
            {
                lua.pop(1);
                break;
            }

            indices.push_back(static_cast<uint8_t>(lua.tointeger(-1)));
            lua.pop(1);
        }

        push_request(L, std::make_shared<game_reqs::item_combine>(indices));
        return 1;
    });
    lua_setfield(L, -2, "item_combine");
}

} // namespace fb::bot::integration::lua_protocol
