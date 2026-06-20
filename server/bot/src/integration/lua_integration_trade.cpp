#include <fb/bot/builtin/game_bot.h>
#include <fb/bot/game_bot.h>
#include <fb/bot/integration/lua_integration_protocol.h>
#include <fb/bot/integration/trade_bot.h>
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

const char* trade_type_name(trade_bot::trade_type type)
{
    switch (type)
    {
    case trade_bot::trade_type::dialog:
        return "dialog";
    case trade_bot::trade_type::bundle:
        return "bundle";
    case trade_bot::trade_type::upload:
        return "upload";
    case trade_bot::trade_type::money:
        return "money";
    case trade_bot::trade_type::close:
        return "close";
    case trade_bot::trade_type::lock:
        return "lock";
    default:
        return "unknown";
    }
}

void push_trade(fb::lua::context* lua, const trade_bot& resp)
{
    lua->new_table();
    lua->pushstring("type");
    lua->pushstring(trade_type_name(resp.type));
    lua->settable(-3);
    lua->pushstring("dialog_oid");
    lua->pushinteger(resp.dialog_oid);
    lua->settable(-3);
    lua->pushstring("dialog_name");
    lua->pushstring(resp.dialog_name);
    lua->settable(-3);
    lua->pushstring("upload_index");
    lua->pushinteger(resp.upload_index);
    lua->settable(-3);
    lua->pushstring("upload_look");
    lua->pushinteger(resp.upload_look);
    lua->settable(-3);
    lua->pushstring("upload_color");
    lua->pushinteger(resp.upload_color);
    lua->settable(-3);
    lua->pushstring("upload_name");
    lua->pushstring(resp.upload_name);
    lua->settable(-3);
    lua->pushstring("upload_mine");
    lua->pushboolean(resp.upload_mine);
    lua->settable(-3);
    lua->pushstring("money");
    lua->pushinteger(resp.money);
    lua->settable(-3);
    lua->pushstring("mine");
    lua->pushboolean(resp.mine);
    lua->settable(-3);
    lua->pushstring("close_message");
    lua->pushstring(resp.close_message);
    lua->settable(-3);
}

} // namespace

int builtin::game_bot::builtin_request_trade(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return luaL_error(L, "request_trade requires an active lua context");

    auto bot = lua->touserdata<fb::bot::game_bot>(1);
    if (bot == nullptr)
        return luaL_error(L, "request_trade requires a bot");

    fb::logger::debug("request_trade entered bot_id={}", bot->id);

    auto request = lua_protocol::to_request(L, 2);
    luaL_checktype(L, 3, LUA_TFUNCTION);

    lua_pushvalue(L, 3);
    auto validator_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    auto bot_ptr = bot;
    auto result  = std::make_shared<trade_bot>();

    auto builder  = lua->new_co_builder();
    builder.yield = [lua, bot_ptr, request, validator_ref, result]() -> async::task<void> {
        auto* L_state = static_cast<lua_State*>(*lua);

        auto cleanup = [L_state, validator_ref]() {
            luaL_unref(L_state, LUA_REGISTRYINDEX, validator_ref);
        };

        if (bot_ptr == nullptr)
        {
            cleanup();
            co_return;
        }

        fb::logger::debug("request_trade yield start bot_id={}", bot_ptr->id);

        auto condition = [lua, validator_ref](const trade_bot& resp) -> bool {
            auto* L_state = static_cast<lua_State*>(*lua);
            push_trade(lua, resp);

            lua_rawgeti(L_state, LUA_REGISTRYINDEX, validator_ref);
            lua_pushvalue(L_state, -2);
            if (lua_pcall(L_state, 1, 1, 0) != LUA_OK)
            {
                auto* err = lua_tostring(L_state, -1);
                fb::logger::fatal("trade validator failed: {}", err != nullptr ? err : "unknown");
                lua_pop(L_state, 2);
                return false;
            }

            auto accepted = lua_toboolean(L_state, -1) != 0;
            lua_pop(L_state, 2);
            fb::logger::debug("request_trade validator accepted={} type={}", accepted, trade_type_name(resp.type));
            return accepted;
        };

        try
        {
            *result = co_await bot_ptr->request<trade_bot>(*request, condition, INTEGRATION_DEFAULT_TIMEOUT);
            fb::logger::debug("request_trade response type={}", trade_type_name(result->type));
        }
        catch (const std::exception& e)
        {
            auto* what = e.what();
            fb::logger::fatal("request_trade failed: {}", (what != nullptr && what[0] != '\0') ? what : "unknown");
            throw;
        }

        cleanup();
    };
    builder.resume = [lua, result]() -> async::task<int> {
        push_trade(lua, *result);
        co_return 1;
    };
    return builder.run();
}
