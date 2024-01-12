#ifndef __LUA_BUILTIN_H__
#define __LUA_BUILTIN_H__

#include <fb/game/context.h>
#include <fb/game/session.h>

template <typename T>
int builtin_dialog(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    try
    {
        auto argc = thread->argc();
        if(argc < 3)
            throw std::runtime_error("not enough parameters");

        auto context = thread->env<fb::game::context>("context");
        auto object = thread->touserdata<T>(1);
        if(object == nullptr)
            return 0;

        auto session = thread->touserdata<fb::game::session>(2);
        if(session == nullptr || context->exists(*session) == false)
            return 0;

        auto message = thread->tostring(3);
        auto button_prev = argc < 4 ? false : thread->toboolean(4);
        auto button_next = argc < 5 ? false : thread->toboolean(5);

        session->dialog.show(*object, message, button_prev, button_next);
        return thread->yield(1);
    }
    catch(std::exception&)
    {
        thread->pushnil();
        return 1;
    }
}

template <typename T>
int builtin_menu(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<T>(1);
    if(npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    auto message = thread->tostring(3);

    // Read menu list
    auto size = thread->rawlen(4);
    auto menus = std::vector<std::string>();
    for(int i = 0; i < size; i++)
    {
        thread->rawgeti(4, i+1);
        menus.push_back(thread->tostring(-1));
    }

    session->dialog.show(*npc, message, menus);
    return thread->yield(1);
}

template <typename T>
int builtin_item(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<T>(1);
    if(npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    auto message = thread->tostring(3);

    auto items = fb::game::dialog::item_pairs();
    thread->pushnil();
    while (thread->next(4))
    {
        // auto i = thread->tointeger(-2);
        auto item = static_cast<fb::game::item::model*>(nullptr);
        auto price = uint32_t(0);

        { // get 1st field
            thread->pushinteger(1);
            lua_gettable(*thread, -2);
            switch(lua_type(*thread, -1))
            {
                case LUA_TSTRING:
                    item = fb::game::model::items.name2item(thread->tostring(-1));
                    break;

                case LUA_TUSERDATA:
                    item = thread->touserdata<fb::game::item::model>(-1);
                    break;
            }
            thread->pop(1);
        }

        { // get 2nd field
            thread->pushinteger(2);
            lua_gettable(*thread, -2);
            switch(lua_type(*thread, -1))
            {
                case LUA_TNUMBER:
                    price = thread->tointeger(-1);
                    break;
            }
        }
        thread->pop(1);

        if (item == nullptr)
            continue;

        items.push_back({ item, price });
        thread->pop(1);
    }

    session->dialog.show(*npc, message, items);
    return thread->yield(1);
}

template <typename T>
int builtin_input(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<T>(1);
    if(npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    auto message = thread->tostring(3);

    auto argc = thread->argc();
    if(argc > 3)
    {
        auto message_top = thread->tostring(4);
        auto message_bot = thread->tostring(5);
        auto maxlen = argc < 6 ? 0xFF : (int)thread->tointeger(6);
        auto prev = argc < 7 ? false : thread->toboolean(7);

        session->dialog.input(*npc, message, message_top, message_bot, maxlen, prev);
    }
    else
    {
        session->dialog.input(*npc, message);
    }
    return thread->yield(1);
}

#endif