#include "builtin_function.h"
#include "fb_game.h"

using namespace fb::game;

int fb::game::acceptor::builtin_name2mob(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto name = lua_tostring(lua, 1);
    auto mob = db::name2mob(name);

    if(mob == nullptr)  { lua_pushnil(lua); }
    else                { mob->to_lua(lua); }
    return 1;
}

int fb::game::acceptor::builtin_name2npc(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto name = lua_tostring(lua, 1);
    auto npc = db::name2npc(name);
    
    if(npc == nullptr)  { lua_pushnil(lua); }
    else                { npc->to_lua(lua); }
    return 1;
}

int fb::game::acceptor::builtin_name2map(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto name = lua_tostring(lua, 1);
    auto map = db::name2map(name);

    if(map == nullptr)  { lua_pushnil(lua); }
    else                { map->to_lua(lua); }
    return 1;
}

int fb::game::acceptor::builtin_name2item(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto name = lua_tostring(lua, 1);
    auto item = db::name2item(name);

    if(item == nullptr) { lua_pushnil(lua); }
    else                { item->to_lua(lua); }
    return 1;
}

int fb::game::acceptor::builtin_timer(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto value = lua_tointeger(lua, 1);
    auto decrease = lua_toboolean(lua, 2);

    acceptor->send_stream(timer::make_stream(value, decrease ? timer::DECREASE : timer::INCREASE));
    return 0;
}

int fb::game::acceptor::builtin_weather(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto value = lua_tointeger(lua, 1);

    acceptor->send_stream(weather::make_stream(weather::type(value)));
    return 0;
}