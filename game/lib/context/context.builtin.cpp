#include <fb/game/context.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::context, "")
END_LUA_EXTENSION; // clang-format on

int fb::game::context::builtin_seed(lua_State* lua)
{
    static std::random_device random;
    static std::mt19937       gen(random());
    static auto               distribution = std::uniform_int_distribution<uint32_t>(0, 0xFFFFFFFF);

    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto seed = distribution(gen);
    thread->pushinteger(seed);
    return 1;
}

int fb::game::context::builtin_sleep(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto ms = (uint32_t)thread->tointeger(1);
    thread->pending(true);

    auto context = thread->env<fb::game::context>("context");
    async::awaitable_then(context->sleep(std::chrono::milliseconds(ms)), [lua](auto result) {
        auto thread = fb::lua::get(lua);
        if (thread == nullptr)
            return;

        thread->pending(false);
        thread->resume(0);
    });
    return thread->yield(0);
}

int fb::game::context::builtin_name2mob(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto name    = thread->tostring(1);
    auto mob     = context->model.mob.name2mob(name);

    if (mob == nullptr)
    {
        thread->pushnil();
    }
    else
    {
        thread->pushobject(mob);
    }
    return 1;
}

int fb::game::context::builtin_name2spell(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto name    = thread->tostring(1);
    auto spell   = context->model.spell.name2spell(name);

    if (spell == nullptr)
    {
        thread->pushnil();
    }
    else
    {
        thread->pushobject(spell);
    }
    return 1;
}

int fb::game::context::builtin_name2npc(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto name    = thread->tostring(1);
    auto npc     = context->model.npc.name2npc(name);

    if (npc == nullptr)
    {
        thread->pushnil();
    }
    else
    {
        thread->pushobject(npc);
    }
    return 1;
}

int fb::game::context::builtin_name2map(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto name    = thread->tostring(1);
    auto map     = context->model.map.name2map(name);

    if (map == nullptr)
    {
        thread->pushnil();
    }
    else
    {
        thread->pushobject(map);
    }
    return 1;
}

int fb::game::context::builtin_name2ch(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto name    = thread->tostring(1);

    auto ch = context->_shard[name]->names.template lock<character*>([&name](auto& names) -> character* {
        if (names.contains(name) == false)
            return nullptr;

        return names.at(name);
    });

    if (ch == nullptr)
    {
        thread->pushnil();
        return 1;
    }

    return context->builtin(*ch, thread, 1, [=]() -> async::task<void> {
        thread->pushobject(ch);
        co_return;
    });
}

int fb::game::context::builtin_name2item(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto name    = thread->tostring(1);
    auto item    = context->model.item.name2item(name);

    if (item == nullptr)
    {
        thread->pushnil();
    }
    else
    {
        thread->pushobject(item);
    }
    return 1;
}

int fb::game::context::builtin_pursuit_sell(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto pursuit = thread->tointeger(1);

    thread->new_table();
    if (context->model.sell.contains(pursuit))
    {
        auto& sell = context->model.sell[pursuit];
        auto  i    = 0;
        for (auto& [k, v] : sell)
        {
            auto& item  = context->model.item[k];
            auto  price = v.price.value_or(item.price);

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
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto pursuit = thread->tointeger(1);
    thread->new_table();
    if (context->model.buy.contains(pursuit))
    {
        auto& buy = context->model.buy[pursuit];
        auto  i   = 0;
        for (auto& [k, v] : buy)
        {
            auto& item  = context->model.item[k];
            auto  price = v.price.value_or(item.price / 2);

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

int fb::game::context::builtin_timer(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context  = thread->env<fb::game::context>("context");
    auto value    = (uint32_t)thread->tointeger(1);
    auto decrease = thread->toboolean(2);

    auto type = decrease ? TIMER_TYPE::DECREASE : TIMER_TYPE::INCREASE;
    context->foreach_ch([value, type](auto& ch) -> async::task<void> {
        ch.timer(value, type);
        co_return;
    });
    return 0;
}

int fb::game::context::builtin_weather(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto value   = (uint32_t)thread->tointeger(1);

    context->foreach_ch([weather = WEATHER_TYPE(value)](auto& ch) -> async::task<void> {
        ch.weather(weather);
        co_return;
    });
    return 0;
}

int fb::game::context::builtin_name_with(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto argc = thread->argc();
    auto name = thread->tostring(1);

    if (argc < 3)
    {
        thread->pushstring(name_with(name));
    }
    else
    {
        auto first  = thread->tostring(2);
        auto second = thread->tostring(3);
        thread->pushstring(name_with(name, {first, second}));
    }
    return 1;
}

int fb::game::context::builtin_assert_korean(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto argc = thread->argc();
    auto text = thread->tostring(1);
    thread->pushboolean(assert_korean(text));
    return 1;
}

int fb::game::context::builtin_cp949(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto argc = thread->argc();
    auto text = thread->tostring(1);
    thread->pushstring(CP949(text));
    return 1;
}

int fb::game::context::builtin_debug(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

#if defined DEBUG | defined _DEBUG
    thread->pushboolean(true);
#else
    thread->pushboolean(false);
#endif
    return 1;
}

int fb::game::context::builtin_broadcast(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context    = thread->env<fb::game::context>("context");
    auto argc       = thread->argc();
    auto text       = thread->tostring(1);
    auto type       = argc < 2 ? MESSAGE_TYPE::STATE : static_cast<MESSAGE_TYPE>(thread->tointeger(2));
    auto broad_type = argc < 3 ? BROADCAST_TYPE::GLOBAL : static_cast<BROADCAST_TYPE>(thread->tointeger(3));

    if (broad_type == BROADCAST_TYPE::WORLD)
    {
        std::ignore = context->broadcast(text, type, broad_type);
        return 0;
    }
    else
    {
        async::awaitable_then(context->broadcast(text, type, broad_type), [thread](auto result) {
            thread->resume(0);
        });
        return thread->yield(0);
    }
}

int fb::game::context::builtin_assert_alive(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc    = thread->argc();
    auto obj     = thread->touserdata<fb::game::object>(1);

    thread->pushboolean(context->alive(*obj));
    return 1;
}