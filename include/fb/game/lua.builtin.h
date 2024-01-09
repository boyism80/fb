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
    std::vector<std::string> menus;
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

    auto items = std::map<item::model*, std::optional<uint32_t>>();
    thread->pushnil();
    while (thread->next(4))
    {
        auto item = static_cast<fb::game::item::model*>(nullptr);
        auto price = std::optional<uint32_t>{};

        if (thread->is_num(-2))
        {
            if (thread->is_obj(-1))
            {
                item = thread->touserdata<item::model>(-1);
            }
            else if (thread->is_str(-1))
            {
                item = fb::game::model::items.name2item(thread->tostring(-1));
            }
            else
            {
                
            }
        }
        else if (thread->is_str(-2))
        {
            item = fb::game::model::items.name2item(thread->tostring(-2));
            price = thread->tointeger(-1);
        }
        else
        {
            
        }

        if (item == nullptr)
            continue;

        items.insert({ item, price });
    }

    session->dialog.show(*npc, message, items);
    return thread->yield(1);
}

template <typename T>
int builtin_sale(lua_State* lua)
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

    // auto message = "무엇을 사시게요?";
    auto message = "제가 파는 물건들입니다. 그림도 있고, 옆에 가격도 함께 드리니 잘 생각하시고 골라주세요.";

    std::map<item::model*, std::optional<uint32_t>> items;
    items.insert({ fb::game::model::items.name2item("막걸리"), 100 });
    items.insert({ fb::game::model::items.name2item("동동주"), 100 });
    items.insert({ fb::game::model::items.name2item("도토리"), 100 });
    session->dialog.show(*npc, message, items, fb::game::dialog::interaction::SALE);
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