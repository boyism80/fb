#include <fb/bot/builtin/game_bot.h>
#include <fb/bot/integration/dialog_bot.h>
#include <fb/bot/integration/dialog_ext_bot.h>
#include <fb/bot/game_bot.h>
#include <fb/bot/integration/lua_integration_protocol.h>
#include <fb/bot/integration/lua_integration_test.h>
#include <fb/game/protocol.h>
#include <fb/logger.h>

#include <fb/lua.h>

#include <memory>
#include <string>

using namespace std::chrono_literals;
using namespace fb::bot::integration;
using namespace fb::bot;

namespace {

#ifdef _DEBUG
constexpr auto INTEGRATION_DEFAULT_TIMEOUT = 30s;
#else
constexpr auto INTEGRATION_DEFAULT_TIMEOUT = 10s;
#endif

const char* dialog_ext_type_name(dialog_ext_type type)
{
    switch (type)
    {
    case dialog_ext_type::normal:
    case dialog_ext_type::normal_no_msg:
        return "normal";
    case dialog_ext_type::list:
    case dialog_ext_type::list_no_msg:
        return "list";
    case dialog_ext_type::input_ext:
    case dialog_ext_type::input_ext_no_msg:
    case dialog_ext_type::input_password:
    case dialog_ext_type::input_password_no_msg:
        return "input_ext";
    case dialog_ext_type::email:
        return "email";
    case dialog_ext_type::look:
        return "look";
    default:
        return "unknown";
    }
}

const char* dialog_type_name(dialog_type type)
{
    switch (type)
    {
    case dialog_type::menu:
    case dialog_type::menu_no_ext:
        return "menu";
    case dialog_type::input:
    case dialog_type::input_no_ext:
        return "input";
    case dialog_type::item:
        return "item";
    case dialog_type::slot:
        return "slot";
    case dialog_type::pursuit:
        return "pursuit";
    case dialog_type::spell:
        return "spell";
    case dialog_type::dual_field:
        return "dual_field";
    default:
        return "unknown";
    }
}

void push_dialog_ext(fb::lua::context* lua, const dialog_ext_bot& resp)
{
    lua->new_table();
    lua->pushstring("type");
    lua->pushstring(dialog_ext_type_name(resp.type));
    lua->settable(-3);
    lua->pushstring("subtype");
    lua->pushinteger(static_cast<lua_Integer>(resp.type));
    lua->settable(-3);
    lua->pushstring("look");
    lua->pushinteger(resp.look);
    lua->settable(-3);
    lua->pushstring("color");
    lua->pushinteger(resp.color);
    lua->settable(-3);
    lua->pushstring("message");
    lua->pushstring(resp.message);
    lua->settable(-3);
    lua->pushstring("oid");
    lua->pushinteger(resp.oid);
    lua->settable(-3);
    lua->pushstring("interaction");
    lua->pushinteger(resp.interaction);
    lua->settable(-3);
    lua->pushstring("normal_button_prev");
    lua->pushboolean(resp.normal_button_prev);
    lua->settable(-3);
    lua->pushstring("normal_button_next");
    lua->pushboolean(resp.normal_button_next);
    lua->settable(-3);
    lua->pushstring("list_button_prev");
    lua->pushboolean(resp.list_button_prev);
    lua->settable(-3);

    lua->pushstring("list_lists");
    lua->new_table();
    for (size_t i = 0; i < resp.list_lists.size(); ++i)
    {
        lua->pushinteger(static_cast<lua_Integer>(i + 1));
        lua->pushstring(resp.list_lists[i]);
        lua->settable(-3);
    }
    lua->settable(-3);

    lua->pushstring("input_ext_top");
    lua->pushstring(resp.input_ext_top);
    lua->settable(-3);
    lua->pushstring("input_ext_bottom");
    lua->pushstring(resp.input_ext_bottom);
    lua->settable(-3);
    lua->pushstring("input_ext_maxlen");
    lua->pushinteger(resp.input_ext_maxlen);
    lua->settable(-3);
    lua->pushstring("input_ext_button_prev");
    lua->pushboolean(resp.input_ext_button_prev);
    lua->settable(-3);
    lua->pushstring("input_ext_password");
    lua->pushboolean(resp.input_ext_password);
    lua->settable(-3);
    lua->pushstring("email_str1");
    lua->pushstring(resp.email_str1);
    lua->settable(-3);
    lua->pushstring("email_str2");
    lua->pushstring(resp.email_str2);
    lua->settable(-3);
}

void push_dialog(fb::lua::context* lua, const dialog_bot& resp)
{
    lua->new_table();
    lua->pushstring("type");
    lua->pushstring(dialog_type_name(resp.type));
    lua->settable(-3);
    lua->pushstring("look");
    lua->pushinteger(resp.look);
    lua->settable(-3);
    lua->pushstring("color");
    lua->pushinteger(resp.color);
    lua->settable(-3);
    lua->pushstring("message");
    lua->pushstring(resp.message);
    lua->settable(-3);
    lua->pushstring("oid");
    lua->pushinteger(resp.oid);
    lua->settable(-3);
    lua->pushstring("interaction");
    lua->pushinteger(resp.interaction);
    lua->settable(-3);
    lua->pushstring("item_pursuit");
    lua->pushinteger(resp.item_pursuit);
    lua->settable(-3);
    lua->pushstring("pursuit");
    lua->pushinteger(resp.pursuit);
    lua->settable(-3);

    lua->pushstring("item_items");
    lua->new_table();
    for (size_t i = 0; i < resp.item_items.size(); ++i)
    {
        lua->pushinteger(static_cast<lua_Integer>(i + 1));
        lua->new_table();
        lua->pushstring("look");
        lua->pushinteger(resp.item_items[i].look);
        lua->settable(-3);
        lua->pushstring("color");
        lua->pushinteger(resp.item_items[i].color);
        lua->settable(-3);
        lua->pushstring("value");
        lua->pushinteger(resp.item_items[i].value);
        lua->settable(-3);
        lua->pushstring("name");
        lua->pushstring(resp.item_items[i].name);
        lua->settable(-3);
        lua->pushstring("desc");
        lua->pushstring(resp.item_items[i].desc);
        lua->settable(-3);
        lua->settable(-3);
    }
    lua->settable(-3);

    lua->pushstring("menu_menus");
    lua->new_table();
    for (size_t i = 0; i < resp.menu_menus.size(); ++i)
    {
        lua->pushinteger(static_cast<lua_Integer>(i + 1));
        lua->pushstring(resp.menu_menus[i]);
        lua->settable(-3);
    }
    lua->settable(-3);

    lua->pushstring("slot_slots");
    lua->new_table();
    for (size_t i = 0; i < resp.slot_slots.size(); ++i)
    {
        lua->pushinteger(static_cast<lua_Integer>(i + 1));
        lua->pushinteger(resp.slot_slots[i]);
        lua->settable(-3);
    }
    lua->settable(-3);

    lua->pushstring("dual_pairs");
    lua->new_table();
    for (size_t i = 0; i < resp.dual_pairs.size(); ++i)
    {
        lua->pushinteger(static_cast<lua_Integer>(i + 1));
        lua->new_table();
        lua->pushstring("label");
        lua->pushstring(resp.dual_pairs[i].first);
        lua->settable(-3);
        lua->pushstring("value");
        lua->pushstring(resp.dual_pairs[i].second);
        lua->settable(-3);
        lua->settable(-3);
    }
    lua->settable(-3);

    lua->pushstring("subtype");
    lua->pushinteger(static_cast<lua_Integer>(resp.type));
    lua->settable(-3);
}

template <typename ResponseType, void (*PushFn)(fb::lua::context*, const ResponseType&)>
int bot_request_dialog_impl(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return 0;

    auto request = lua_protocol::to_request(L, 2);
    luaL_checktype(L, 3, LUA_TFUNCTION);

    lua_pushvalue(L, 3);
    auto validator_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    auto timeout = fb::model::timespan(INTEGRATION_DEFAULT_TIMEOUT);
    if (lua->argc() >= 4 && lua->is_nil(4) == false)
        timeout = fb::model::timespan(std::chrono::milliseconds(static_cast<int>(lua->tointeger(4))));

    auto bot_ptr = bot;
    auto result  = std::make_shared<std::optional<ResponseType>>();

    auto builder  = lua->new_co_builder();
    builder.yield = [lua, bot_ptr, request, validator_ref, timeout, result]() -> async::task<void> {
        auto* L_state = static_cast<lua_State*>(*lua);

        auto cleanup = [L_state, validator_ref]() {
            luaL_unref(L_state, LUA_REGISTRYINDEX, validator_ref);
        };

        if (bot_ptr == nullptr)
        {
            cleanup();
            co_return;
        }

        // condition: push response table on the suspended coroutine's stack, call validator
        auto condition = [lua, validator_ref](const ResponseType& resp) -> bool {
            PushFn(lua, resp); // push onto the coroutine stack (suspended, safe to do)

            auto* L_state = static_cast<lua_State*>(*lua);
            lua_rawgeti(L_state, LUA_REGISTRYINDEX, validator_ref);
            lua_pushvalue(L_state, -2);
            if (lua_pcall(L_state, 1, 1, 0) != LUA_OK)
            {
                auto* err = lua_tostring(L_state, -1);
                fb::logger::fatal("dialog validator failed: {}", err != nullptr ? err : "unknown");
                lua_pop(L_state, 2);
                return false;
            }
            auto accepted = lua_toboolean(L_state, -1) != 0;
            lua_pop(L_state, 2);

            return accepted;
        };

        try
        {
            result->emplace(co_await bot_ptr->request<ResponseType>(*request, condition, timeout));
        }
        catch (const std::exception& e)
        {
            cleanup();
            auto* what = e.what();
            fb::logger::fatal("request_dialog failed: {}", (what != nullptr && what[0] != '\0') ? what : "unknown");
            throw;
        }

        cleanup();
    };
    builder.resume = [lua, result]() -> async::task<int> {
        if (result->has_value() == false)
        {
            lua->pushnil();
            co_return 1;
        }
        PushFn(lua, result->value());
        co_return 1;
    };
    return builder.run();
}

} // namespace

int builtin::game_bot::builtin_request_dialog(lua_State* L)
{
    return bot_request_dialog_impl<dialog_bot, push_dialog>(L);
}

int builtin::game_bot::builtin_request_dialog_ext(lua_State* L)
{
    return bot_request_dialog_impl<dialog_ext_bot, push_dialog_ext>(L);
}
