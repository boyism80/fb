#include "builtin/builtin_function.h"
#include "model/acceptor/acceptor.game.h"

using namespace fb::game;

template<typename T>
inline void to_lua(lua_State* lua, const T* self)
{
    auto allocated = (void**)lua_newuserdata(lua, sizeof(void**));
    *allocated = (const void*)self;

    auto metaname = self->metaname();
    luaL_getmetatable(lua, metaname.c_str());
    lua_setmetatable(lua, -2);
}

template <typename T>
int builtin_dialog(lua_State* lua)
{
    try
    {
        auto argc = lua_gettop(lua);
        if(argc < 3)
            throw std::runtime_error("not enough parameters");

        auto object = *(T**)lua_touserdata(lua, 1);
        auto session = *(fb::game::session**)lua_touserdata(lua, 2);
        auto message = lua_tostring(lua, 3);
        auto button_prev = argc < 4 ? false : lua_toboolean(lua, 4);
        auto button_next = argc < 5 ? false : lua_toboolean(lua, 5);
        auto acceptor = lua::env<fb::game::acceptor>("acceptor");

        acceptor->send_stream(*session, object->make_dialog_stream(message, button_prev, button_next), acceptor::scope::SELF);
        return lua_yield(lua, 1);
    }
    catch(std::exception&)
    {
        lua_pushnil(lua);
        return 1;
    }
}

template <typename T>
int builtin_menu_dialog(lua_State* lua)
{
    auto npc = *(T**)lua_touserdata(lua, 1);
    auto session = *(fb::game::session**)lua_touserdata(lua, 2);
    auto message = lua_tostring(lua, 3);

    // Read menu list
    auto size = lua_rawlen(lua, 4);
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
    auto size = lua_rawlen(lua, 4);
    std::vector<item::master*> items;
    for(int i = 0; i < size; i++)
    {
        lua_rawgeti(lua, 4, i+1);
        items.push_back(*(item::master**)lua_touserdata(lua, -1));
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
    auto argc = lua_gettop(lua);
    if(argc > 3)
    {
        auto message_top = lua_tostring(lua, 4);
        auto message_bot = lua_tostring(lua, 5);
        auto maxlen = argc < 6 ? 0xFF : (int)lua_tointeger(lua, 6);
        auto prev = argc < 7 ? false : lua_toboolean(lua, 7);
        
        acceptor->send_stream(*session, npc->make_input_dialog_stream(message, message_top, message_bot, maxlen, prev), acceptor::scope::SELF);
    }
    else
    {
        acceptor->send_stream(*session, npc->make_input_dialog_stream(message), acceptor::scope::SELF);
    }
    return lua_yield(lua, 1);
}