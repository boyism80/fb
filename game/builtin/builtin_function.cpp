#include "builtin/builtin_function.h"
#include "model/acceptor/acceptor.game.h"

using namespace fb::game;

int fb::game::acceptor::builtin_name2mob(lua_State* lua)
{
    auto name = lua_cp949(lua, 1);
    auto mob = fb::game::master::get().name2mob(name);

    if(mob == nullptr)  { lua_pushnil(lua); }
    else                { mob->to_lua(lua); }
    return 1;
}

int fb::game::acceptor::builtin_name2npc(lua_State* lua)
{
    auto name = lua_cp949(lua, 1);
    auto npc = fb::game::master::get().name2npc(name);
    
    if(npc == nullptr)  { lua_pushnil(lua); }
    else                { npc->to_lua(lua); }
    return 1;
}

int fb::game::acceptor::builtin_name2map(lua_State* lua)
{
    auto name = lua_cp949(lua, 1);
    auto map = fb::game::master::get().name2map(name);

    if(map == nullptr)  { lua_pushnil(lua); }
    else                { map->to_lua(lua); }
    return 1;
}

int fb::game::acceptor::builtin_name2item(lua_State* lua)
{
    auto name = lua_cp949(lua, 1);
    auto item = fb::game::master::get().name2item(name);

    if(item == nullptr) { lua_pushnil(lua); }
    else                { item->to_lua(lua); }
    return 1;
}

int fb::game::acceptor::builtin_timer(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto value = (uint32_t)lua_tointeger(lua, 1);
    auto decrease = lua_toboolean(lua, 2);

    acceptor->send(fb::protocol::game::response::timer(value, decrease ? timer::DECREASE : timer::INCREASE));
    return 0;
}

int fb::game::acceptor::builtin_weather(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto value = (uint32_t)lua_tointeger(lua, 1);

    acceptor->send(fb::protocol::game::response::weather(weather::type(value)));
    return 0;
}