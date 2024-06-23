#include <fb/game/npc.h>
#include <fb/game/context.h>

int fb::model::npc::builtin_input(lua_State* lua)
{
    // Ex) npc::input(session, "message")
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::model::npc>(1);
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

int fb::model::npc::builtin_menu(lua_State* lua)
{
    // Ex) npc::menu(session, "hello", {"hello 1", "hello 2", "hello 3"})
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::model::npc>(1);
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

int fb::model::npc::builtin_item(lua_State* lua)
{
    // Ex) npc::menu(session, "hello", {item1, item2, item3})
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::model::npc>(1);
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
        auto item = static_cast<fb::model::item*>(nullptr);
        auto price = uint32_t(0);

        { // get 1st field
            thread->pushinteger(1);
            lua_gettable(*thread, -2);
            switch(lua_type(*thread, -1))
            {
                case LUA_TSTRING:
                    item = context->model.item.name2item(thread->tostring(-1));
                    break;

                case LUA_TUSERDATA:
                    item = thread->touserdata<fb::model::item>(-1);
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

        items.push_back({ *item, price });
        thread->pop(1);
    }

    session->dialog.show(*npc, message, items);
    return thread->yield(1);
}

int fb::model::npc::builtin_slot(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::model::npc>(1);
    if(npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if(session == nullptr || context->exists(*session) == false)
        return 0;

    auto message = thread->tostring(3);
    auto slots = std::vector<uint8_t>();
    auto size = lua_rawlen(*thread, 4);
    for(int i = 0; i < size; i++)
    {
        thread->pushinteger(i+1);
        lua_gettable(*thread, 4);
        slots.push_back(thread->tointeger(-1));
        thread->pop(1);
    }

    session->dialog.show(*npc, message, slots);
    return thread->yield(1);
}

int fb::model::npc::builtin_sell(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    auto& dialog = session->dialog.from("scripts/common/npc.lua")
                   .func("sell")
                   .pushobject(session)
                   .pushobject(npc);

    if (npc->sell.size() == 1)
    {
        auto pursuit = npc->sell[0];
        dialog.pushinteger(pursuit);
    }
    else if(npc->sell.size() > 1)
    {
        dialog.new_table();
        auto i = 1;
        for (auto& pursuit : npc->sell)
        {
            auto& menu = context->model.sell_attribute[pursuit].group;
            dialog.pushinteger(i);
            dialog.new_table();
            {
                dialog.pushinteger(1);
                dialog.pushstring(menu);
                dialog.set_table();

                dialog.pushinteger(2);
                dialog.pushinteger(pursuit);
                dialog.set_table();
            }
            dialog.set_table();
            i++;
        }
    }
    else
    {
        dialog.pushnil();
    }

    dialog.resume(3);
    return thread->yield(1);
}

int fb::model::npc::builtin_buy(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    auto& dialog = session->dialog.from("scripts/common/npc.lua")
                   .func("buy")
                   .pushobject(session)
                   .pushobject(npc);

    if (npc->buy.has_value())
    {
        dialog.pushinteger(npc->buy.value());
    }
    else
    {
        dialog.pushnil();
    }

    dialog.resume(3);
    return thread->yield(1);
}

int fb::model::npc::builtin_repair(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    session->dialog.from("scripts/common/npc.lua")
                   .func("repair")
                   .pushobject(session)
                   .pushobject(npc)
                   .resume(2);
    return thread->yield(1);
}

int fb::model::npc::builtin_repair_all(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    session->dialog.from("scripts/common/npc.lua")
                   .func("repair_all")
                   .pushobject(session)
                   .pushobject(npc)
                   .resume(2);
    return thread->yield(1);
}

int fb::model::npc::builtin_hold_money(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    session->dialog.from("scripts/common/npc.lua")
                   .func("hold_money")
                   .pushobject(session)
                   .pushobject(npc)
                   .resume(2);
    return thread->yield(1);
}

int fb::model::npc::builtin_hold_item(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    session->dialog.from("scripts/common/npc.lua")
                   .func("hold_item")
                   .pushobject(session)
                   .pushobject(npc)
                   .resume(2);
    return thread->yield(1);
}

int fb::model::npc::builtin_return_money(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    session->dialog.from("scripts/common/npc.lua")
                   .func("return_money")
                   .pushobject(session)
                   .pushobject(npc)
                   .resume(2);
    return thread->yield(1);
}

int fb::model::npc::builtin_return_item(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    session->dialog.from("scripts/common/npc.lua")
                   .func("return_item")
                   .pushobject(session)
                   .pushobject(npc)
                   .resume(2);
    return thread->yield(1);
}

int fb::model::npc::builtin_rename_weapon(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::model::npc>(1);
    if (npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    session->dialog.from("scripts/common/npc.lua")
                   .func("rename_weapon")
                   .pushobject(session)
                   .pushobject(npc)
                   .resume(2);
    return thread->yield(1);
}