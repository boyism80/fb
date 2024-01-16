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
    context->threads().dispatch
    (
        [lua]
        {
            auto thread = fb::game::lua::get(lua);
            if(thread == nullptr)
                return;
            
            thread->pending(false);
            thread->resume(0);
        },
        std::chrono::milliseconds(ms)
    );
    return thread->yield(0);
}

int fb::game::context::builtin_name2mob(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto name = thread->tostring(1);
    auto mob = fb::game::model::mobs.name2mob(name);

    if(mob == nullptr)  { thread->pushnil(); }
    else                { thread->pushobject(mob); }
    return 1;
}

int fb::game::context::builtin_name2npc(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto name = thread->tostring(1);
    auto npc = fb::game::model::npcs.name2npc(name);
    
    if(npc == nullptr)  { thread->pushnil(); }
    else                { thread->pushobject(npc); }
    return 1;
}

int fb::game::context::builtin_name2map(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto name = thread->tostring(1);
    auto map = fb::game::model::maps.name2map(name);

    if(map == nullptr)  { thread->pushnil(); }
    else                { thread->pushobject(map); }
    return 1;
}

int fb::game::context::builtin_name2item(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto name = thread->tostring(1);
    auto item = fb::game::model::items.name2item(name);

    if(item == nullptr) { thread->pushnil(); }
    else                { thread->pushobject(item); }
    return 1;
}

int fb::game::context::builtin_pursuit_sell(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto pursuit = thread->tointeger(1);
    auto sell = fb::game::model::sell[pursuit];
    thread->new_table();
    if(sell != nullptr)
    {
        for (int i = 0, size = sell->size(); i < size; i++)
        {
            auto& [k, v] = sell->at(i);
            auto price = v.has_value() ? v.value() : k->price;
            thread->pushinteger(i+1);
            thread->new_table();
            {
                thread->pushinteger(1);
                thread->pushobject(k);
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

    auto pursuit = thread->tointeger(1);
    auto buy = fb::game::model::buy[pursuit];
    thread->new_table();
    if(buy != nullptr)
    {
        for (int i = 0, size = buy->size(); i < size; i++)
        {
            auto& [k, v] = buy->at(i);
            auto price = v.has_value() ? v.value() : k->price;
            thread->pushinteger(i+1);
            thread->new_table();
            {
                thread->pushinteger(1);
                thread->pushobject(k);
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

    auto pursuit = thread->tointeger(1);
    if(fb::game::model::sell.contains(pursuit) == false)
    {
        thread->pushnil();
        return 1;
    }
    auto sell = fb::game::model::sell[pursuit];
    
    auto name = thread->tostring(2);
    for(auto& [k, v] : *sell)
    {
        if(k->name == name)
        {
            auto price = v.has_value() ? v.value() : k->price;
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

    auto pursuit = thread->tointeger(1);
    if(fb::game::model::buy.contains(pursuit) == false)
    {
        thread->pushnil();
        return 1;
    }
    auto buy = fb::game::model::buy[pursuit];
    
    auto name = thread->tostring(2);
    for(auto& [k, v] : *buy)
    {
        if(k->name == name)
        {
            auto price = v.has_value() ? v.value() : k->price;
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

    context->send(fb::protocol::game::response::timer(value, decrease ? timer::type::DECREASE : timer::type::INCREASE));
    return 0;
}

int fb::game::context::builtin_weather(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto value = (uint32_t)thread->tointeger(1);

    context->send(fb::protocol::game::response::weather(weather::type(value)));
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