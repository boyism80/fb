#include <fb/game/npc.h>
#include <fb/game/context.h>
#include <fb/game/built_in.h>

int __builtin_sell(fb::game::lua::context* thread, fb::game::session* session, const fb::game::npc::model* npc)
{
    auto& dialog = session->dialog.from("scripts/common/npc.lua")
                   .func("sell")
                   .pushobject(session)
                   .pushobject(npc);

    if (npc->sell.size() == 1 && npc->sell.contains(""))
    {
        auto pursuit = npc->sell.at("");
        dialog.pushinteger(pursuit);
    }
    else if(npc->sell.size() > 1)
    {
        dialog.new_table();
        auto i = 1;
        for (auto& [menu, pursuit] : npc->sell)
        {
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

int __builtin_buy(fb::game::lua::context* thread, fb::game::session* session, const fb::game::npc::model* npc)
{
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

int __builtin_repair(fb::game::lua::context* thread, fb::game::session* session, const fb::game::npc::model* npc)
{
    session->dialog.from("scripts/common/npc.lua")
                   .func("repair")
                   .pushobject(session)
                   .pushobject(npc)
                   .resume(2);
    return thread->yield(1);
}

int __builtin_repair_all(fb::game::lua::context* thread, fb::game::session* session, const fb::game::npc::model* npc)
{
    session->dialog.from("scripts/common/npc.lua")
                   .func("repair_all")
                   .pushobject(session)
                   .pushobject(npc)
                   .resume(2);
    return thread->yield(1);
}

int __builtin_hold_money(fb::game::lua::context* thread, fb::game::session* session, const fb::game::npc::model* npc)
{
    session->dialog.from("scripts/common/npc.lua")
                   .func("hold_money")
                   .pushobject(session)
                   .pushobject(npc)
                   .resume(2);
    return thread->yield(1);
}

int __builtin_hold_item(fb::game::lua::context* thread, fb::game::session* session, const fb::game::npc::model* npc)
{
    session->dialog.from("scripts/common/npc.lua")
                   .func("hold_item")
                   .pushobject(session)
                   .pushobject(npc)
                   .resume(2);
    return thread->yield(1);
}

int __builtin_return_money(fb::game::lua::context* thread, fb::game::session* session, const fb::game::npc::model* npc)
{
    session->dialog.from("scripts/common/npc.lua")
                   .func("return_money")
                   .pushobject(session)
                   .pushobject(npc)
                   .resume(2);
    return thread->yield(1);
}

int __builtin_return_item(fb::game::lua::context* thread, fb::game::session* session, const fb::game::npc::model* npc)
{
    session->dialog.from("scripts/common/npc.lua")
                   .func("return_item")
                   .pushobject(session)
                   .pushobject(npc)
                   .resume(2);
    return thread->yield(1);
}

int __builtin_rename_weapon(fb::game::lua::context* thread, fb::game::session* session, const fb::game::npc::model* npc)
{
    session->dialog.from("scripts/common/npc.lua")
                   .func("rename_weapon")
                   .pushobject(session)
                   .pushobject(npc)
                   .resume(2);
    return thread->yield(1);
}

int fb::game::npc::model::builtin_input(lua_State* lua)
{
    // Ex) npc::input(session, "message")
    return ::builtin_input<fb::game::npc::model>(lua);
}

int fb::game::npc::model::builtin_menu(lua_State* lua)
{
    // Ex) npc::menu(session, "hello", {"hello 1", "hello 2", "hello 3"})
    return ::builtin_menu<fb::game::npc::model>(lua);
}

int fb::game::npc::model::builtin_item(lua_State* lua)
{
    // Ex) npc::menu(session, "hello", {item1, item2, item3})
    return ::builtin_item<fb::game::npc::model>(lua);
}

int fb::game::npc::model::builtin_slot(lua_State* lua)
{
    return ::builtin_slot<fb::game::npc::model>(lua);
}

int fb::game::npc::model::builtin_sell(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::game::npc::model>(1);
    if (npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    return __builtin_sell(thread, session, npc);
}

int fb::game::npc::model::builtin_buy(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::game::npc::model>(1);
    if (npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    return __builtin_buy(thread, session, npc);
}

int fb::game::npc::model::builtin_repair(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::game::npc::model>(1);
    if (npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    return __builtin_repair(thread, session, npc);
}

int fb::game::npc::model::builtin_repair_all(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::game::npc::model>(1);
    if (npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    return __builtin_repair_all(thread, session, npc);
}

int fb::game::npc::model::builtin_hold_money(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::game::npc::model>(1);
    if (npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    return __builtin_hold_money(thread, session, npc);
}

int fb::game::npc::model::builtin_hold_item(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::game::npc::model>(1);
    if (npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    return __builtin_hold_item(thread, session, npc);
}

int fb::game::npc::model::builtin_return_money(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::game::npc::model>(1);
    if (npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    return __builtin_return_money(thread, session, npc);
}

int fb::game::npc::model::builtin_return_item(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::game::npc::model>(1);
    if (npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    return __builtin_return_item(thread, session, npc);
}

int fb::game::npc::model::builtin_rename_weapon(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto npc = thread->touserdata<fb::game::npc::model>(1);
    if (npc == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    return __builtin_rename_weapon(thread, session, npc);
}

int fb::game::npc::builtin_input(lua_State* lua)
{
    return ::builtin_input<npc>(lua);
}

int fb::game::npc::builtin_menu(lua_State* lua)
{
    return ::builtin_menu<npc>(lua);
}

int fb::game::npc::builtin_item(lua_State* lua)
{
    return ::builtin_item<npc>(lua);
}

int fb::game::npc::builtin_slot(lua_State* lua)
{
    return ::builtin_slot<npc>(lua);
}

int fb::game::npc::builtin_sell(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<fb::game::npc>(1);
    if (obj == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    auto npc = obj->based<fb::game::npc::model>();
    return __builtin_sell(thread, session, npc);
}

int fb::game::npc::builtin_buy(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<fb::game::npc>(1);
    if (obj == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    auto npc = obj->based<fb::game::npc::model>();
    return __builtin_buy(thread, session, npc);
}

int fb::game::npc::builtin_repair(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<fb::game::npc>(1);
    if (obj == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    auto npc = obj->based<fb::game::npc::model>();
    return __builtin_repair(thread, session, npc);
}

int fb::game::npc::builtin_repair_all(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<fb::game::npc>(1);
    if (obj == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    auto npc = obj->based<fb::game::npc::model>();
    return __builtin_repair_all(thread, session, npc);
}

int fb::game::npc::builtin_hold_money(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<fb::game::npc>(1);
    if (obj == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    auto npc = obj->based<fb::game::npc::model>();
    return __builtin_hold_money(thread, session, npc);
}

int fb::game::npc::builtin_hold_item(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<fb::game::npc>(1);
    if (obj == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    auto npc = obj->based<fb::game::npc::model>();
    return __builtin_hold_item(thread, session, npc);
}

int fb::game::npc::builtin_return_money(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<fb::game::npc>(1);
    if (obj == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    auto npc = obj->based<fb::game::npc::model>();
    return __builtin_return_money(thread, session, npc);
}

int fb::game::npc::builtin_return_item(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<fb::game::npc>(1);
    if (obj == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    auto npc = obj->based<fb::game::npc::model>();
    return __builtin_return_item(thread, session, npc);
}

int fb::game::npc::builtin_rename_weapon(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto obj = thread->touserdata<fb::game::npc>(1);
    if (obj == nullptr)
        return 0;

    auto session = thread->touserdata<fb::game::session>(2);
    if (session == nullptr || context->exists(*session) == false)
        return 0;

    auto npc = obj->based<fb::game::npc::model>();
    return __builtin_rename_weapon(thread, session, npc);
}