#include <fb/game/context.h>
#include <fb/game/lua.h>

using namespace fb::game;

int fb::game::context::builtin_seed(lua_State* lua)
{
    static std::random_device random;
    static std::mt19937 gen(random());
    static auto distribution = std::uniform_int_distribution<uint32_t>(0, 0xFFFFFFFF);

    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto seed = distribution(gen);
    thread->pushinteger(seed);
    return 1;
}

int fb::game::context::builtin_sleep(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto ms = (uint32_t)thread->tointeger(1);
    thread->pending(true);

    auto context = thread->env<fb::game::context>("context");
    context->threads().dispatch([lua] () -> async::task<void>
    {
        auto thread = fb::game::lua::get(lua);
        if(thread == nullptr)
            co_return;
        
        thread->pending(false);
        thread->resume(0);
    }, std::chrono::milliseconds(ms));
    return thread->yield(0);
}

int fb::game::context::builtin_name2mob(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto name = thread->tostring(1);
    auto mob = context->model.mob.name2mob(name);

    if(mob == nullptr)  { thread->pushnil(); }
    else                { thread->pushobject(mob); }
    return 1;
}

int fb::game::context::builtin_name2npc(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto name = thread->tostring(1);
    auto npc = context->model.npc.name2npc(name);
    
    if(npc == nullptr)  { thread->pushnil(); }
    else                { thread->pushobject(npc); }
    return 1;
}

int fb::game::context::builtin_name2map(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto name = thread->tostring(1);
    auto map = context->model.map.name2map(name);

    if(map == nullptr)  { thread->pushnil(); }
    else                { thread->pushobject(map); }
    return 1;
}

int fb::game::context::builtin_name2item(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto name = thread->tostring(1);
    auto item = context->model.item.name2item(name);

    if(item == nullptr) { thread->pushnil(); }
    else                { thread->pushobject(item); }
    return 1;
}

int fb::game::context::builtin_pursuit_sell(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto pursuit = thread->tointeger(1);

    thread->new_table();
    if(context->model.sell.contains(pursuit))
    {
        auto& sell = context->model.sell[pursuit];
        auto i = 0;
        for (auto& [k, v] : sell)
        {
            auto& item = context->model.item[k];
            auto price = v.price.value_or(item.price);

            thread->pushinteger(++i);
            thread->new_table();
            {
                thread->pushinteger(1);
                thread->pushobject(item);
                lua_settable(*thread, -3);

                thread->pushinteger(2);
                thread->pushinteger(price);
                lua_settable(*thread, -3);
            }
            lua_settable(*thread, -3);
        }
    }

    return 1;
}

int fb::game::context::builtin_pursuit_buy(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto pursuit = thread->tointeger(1);
    thread->new_table();
    if(context->model.buy.contains(pursuit))
    {
        auto& buy = context->model.buy[pursuit];
        auto i = 0;
        for (auto& [k, v] : buy)
        {
            auto& item = context->model.item[k];
            auto price = v.price.value_or(item.price / 2);

            thread->pushinteger(++i);
            thread->new_table();
            {
                thread->pushinteger(1);
                thread->pushobject(item);
                lua_settable(*thread, -3);

                thread->pushinteger(2);
                thread->pushinteger(price);
                lua_settable(*thread, -3);
            }
            lua_settable(*thread, -3);
        }
    }

    return 1;
}

int fb::game::context::builtin_sell_price(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto pursuit = thread->tointeger(1);
    if(context->model.sell.contains(pursuit) == false)
    {
        thread->pushnil();
        return 1;
    }
    
    auto name = thread->tostring(2);
    for(auto& [_, x] : context->model.sell[pursuit])
    {
        auto& item = context->model.item[x.item];
        if(item.name == name)
        {
            auto price = x.price.value_or(item.price);
            thread->pushinteger(price);
            return 1;
        }
    }
    
    thread->pushnil();
    return 1;
}

int fb::game::context::builtin_buy_price(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto pursuit = thread->tointeger(1);
    if(context->model.buy.contains(pursuit) == false)
    {
        thread->pushnil();
        return 1;
    }

    auto& buy = context->model.buy[pursuit];
    auto name = thread->tostring(2);
    for(auto& [_, x] : buy)
    {
        auto& item = context->model.item[x.item];
        if(item.name == name)
        {
            auto price = x.price.value_or(item.price);
            thread->pushinteger(price);
            return 1;
        }
    }
    
    thread->pushnil();
    return 1;
}

int fb::game::context::builtin_timer(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto value = (uint32_t)thread->tointeger(1);
    auto decrease = thread->toboolean(2);

    context->send(fb::protocol::game::response::timer(value, decrease ? TIMER_TYPE::DECREASE : TIMER_TYPE::INCREASE));
    return 0;
}

int fb::game::context::builtin_weather(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto value = (uint32_t)thread->tointeger(1);

    context->send(fb::protocol::game::response::weather(WEATHER_TYPE(value)));
    return 0;
}

int fb::game::context::builtin_name_with(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto argc = thread->argc();
    auto name = thread->tostring(1);
    
    if(argc < 3)
    {
        thread->pushstring(name_with(name));
    }
    else
    {
        auto first = thread->tostring(2);
        auto second = thread->tostring(3);
        thread->pushstring(name_with(name, { first, second }));
    }
    return 1;
}

int fb::game::context::builtin_assert_korean(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto argc = thread->argc();
    auto text = thread->tostring(1);
    thread->pushboolean(assert_korean(text));
    return 1;
}

int fb::game::context::builtin_cp949(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto argc = thread->argc();
    auto text = thread->tostring(1);
    thread->pushstring(CP949(text));
    return 1;
}