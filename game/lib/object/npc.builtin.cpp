#include <fb/game/npc.h>
#include <fb/game/context.h>
#include <fb/game/built_in.h>

int __builtin_sale(fb::game::lua::context* thread, fb::game::session* session, const fb::game::npc::model* npc, uint16_t pursuit)
{
    auto pairs = fb::game::model::sale[pursuit];
    if(pairs == nullptr)
        return 0;

    auto message = "제가 파는 물건들입니다. 그림도 있고, 옆에 가격도 함께 드리니 잘 생각하시고 골라주세요.";
    session->dialog.show(*npc, message, pairs, pursuit, fb::game::dialog::interaction::SALE);
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

int fb::game::npc::model::builtin_sale(lua_State* lua)
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

    auto pursuit = (uint16_t)thread->tointeger(3);

    return __builtin_sale(thread, session, npc, pursuit);
}

fb::game::npc::npc(fb::game::context& context, const fb::game::npc::model* model) : 
    fb::game::object(context, model, fb::game::object::config())
{ }

fb::game::npc::npc(const npc& right) :
    object(right)
{ }

fb::game::npc::~npc()
{ }

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

int fb::game::npc::builtin_sale(lua_State* lua)
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

    auto pursuit = (uint16_t)thread->tointeger(3);
    return __builtin_sale(thread, session, npc, pursuit);
}