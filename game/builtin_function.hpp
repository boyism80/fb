#include "builtin_function.h"
#include "fb_game.h"

using namespace fb::game;

template <typename T>
int builtin_dialog(lua_State* lua)
{
    auto object = *(T**)lua_touserdata(lua, 1);
    auto session = *(fb::game::session**)lua_touserdata(lua, 2);
    auto message = lua_tostring(lua, 3);
    auto button_prev = lua_toboolean(lua, 4);
    auto button_next = lua_toboolean(lua, 5);
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");

    acceptor->send_stream(*session, object->make_dialog_stream(message, button_prev, button_next), acceptor::scope::SELF);
    return lua_yield(lua, 1);
}

template <typename T>
int builtin_input_dialog_ext(lua_State* lua)
{
    auto npc = *(T**)lua_touserdata(lua, 1);
    auto session = *(fb::game::session**)lua_touserdata(lua, 2);
    auto message = lua_tostring(lua, 3);
    auto top = lua_tostring(lua, 4);
    auto bottom = lua_tostring(lua, 5);
    auto maxlen = lua_tointeger(lua, 6);
    auto prev = lua_toboolean(lua, 7);
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");

    acceptor->send_stream(*session, npc->make_input_dialog_stream(message, top, bottom, maxlen, prev), acceptor::scope::SELF);

    return lua_yield(lua, 1);
}

template <typename T>
int builtin_menu_dialog(lua_State* lua)
{
    auto npc = *(T**)lua_touserdata(lua, 1);
    auto session = *(fb::game::session**)lua_touserdata(lua, 2);
    auto message = lua_tostring(lua, 3);

    // Read menu list
    auto size = lua_objlen(lua, 4);
    std::vector<std::string> menus;
    for(int i = 0; i < size; i++)
    {
        lua_rawgeti(lua, 4, i+1);
        menus.push_back(lua_tostring(lua, -1));
    }

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    acceptor->send_stream(*session, npc->make_dialog_stream(message, menus), acceptor::scope::SELF);
    return lua_yield(lua, 1);
}

template <typename T>
int builtin_item_dialog(lua_State* lua)
{
    auto npc = *(T**)lua_touserdata(lua, 1);
    auto session = *(fb::game::session**)lua_touserdata(lua, 2);
    auto message = lua_tostring(lua, 3);

    // Read menu list
    auto size = lua_objlen(lua, 4);
    std::vector<item::core*> items;
    for(int i = 0; i < size; i++)
    {
        lua_rawgeti(lua, 4, i+1);
        items.push_back(*(item::core**)lua_touserdata(lua, -1));
    }

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    acceptor->send_stream(*session, npc->make_dialog_stream(message, items), acceptor::scope::SELF);
    return lua_yield(lua, 1);
}

template <typename T>
int builtin_input_dialog(lua_State* lua)
{
    auto npc = *(T**)lua_touserdata(lua, 1);
    auto session = *(fb::game::session**)lua_touserdata(lua, 2);
    auto message = lua_tostring(lua, 3);

    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    acceptor->send_stream(*session, npc->make_input_dialog_stream(message), acceptor::scope::SELF);
    return lua_yield(lua, 1);
}