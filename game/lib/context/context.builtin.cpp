#include <fb/game/context.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::game::context, "")
END_LUA_EXTENSION; // clang-format on

int context::builtin::builtin_seed(lua_State* L)
{
    static std::random_device random;
    static std::mt19937       gen(random());
    static auto               distribution = std::uniform_int_distribution<uint32_t>(0, 0xFFFFFFFF);

    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto seed = distribution(gen);
    lua->pushinteger(seed);
    return 1;
}

int context::builtin::builtin_sleep(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto ms = (uint32_t)lua->tointeger(1);
    lua->pending(true);

    auto context = lua->env<fb::game::context>("context");
    async::awaitable_then(context->sleep(std::chrono::milliseconds(ms)), [L](auto result) {
        auto lua = fb::lua::get(L);
        if (lua == nullptr)
            return;

        lua->pending(false);
        lua->resume(0);
    });
    return lua->yield(0);
}

int context::builtin::builtin_name2mob(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto name    = lua->tostring(1);
    auto mob     = context->model.mob.name2mob(name);

    if (mob == nullptr)
    {
        lua->pushnil();
    }
    else
    {
        lua->pushobject(mob);
    }
    return 1;
}

int context::builtin::builtin_name2spell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto name    = lua->tostring(1);
    auto spell   = context->model.spell.name2spell(name);

    if (spell == nullptr)
    {
        lua->pushnil();
    }
    else
    {
        lua->pushobject(spell);
    }
    return 1;
}

int context::builtin::builtin_name2npc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto name    = lua->tostring(1);
    auto npc     = context->model.npc.name2npc(name);

    if (npc == nullptr)
    {
        lua->pushnil();
    }
    else
    {
        lua->pushobject(npc);
    }
    return 1;
}

int context::builtin::builtin_name2map(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto name    = lua->tostring(1);
    auto map     = context->model.map.name2map(name);

    if (map == nullptr)
    {
        lua->pushnil();
    }
    else
    {
        lua->pushobject(map);
    }
    return 1;
}

int context::builtin::builtin_name2ch(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto name    = lua->tostring(1);

    auto ch = context->_shard[name]->names.template read<character*>([&name](const auto& names) -> character* {
        if (names.contains(name) == false)
            return nullptr;

        return names.at(name);
    });

    if (ch == nullptr)
    {
        lua->pushnil();
        return 1;
    }

    return lua->ensure_yield(*context, *ch, [=]() {
        return lua->ensure_resume(*context, *ch, [=]() {
            lua->pushobject(ch);
            return 1;
        });
    });
}

int context::builtin::builtin_name2item(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto name    = lua->tostring(1);
    auto item    = context->model.item.name2item(name);

    if (item == nullptr)
    {
        lua->pushnil();
    }
    else
    {
        lua->pushobject(item);
    }
    return 1;
}

int context::builtin::builtin_pursuit_sell(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");

    lua->new_table();
    if (lua->is_nil(1))
        return 1;

    auto pursuit = lua->tointeger(1);
    if (context->model.sell.contains(pursuit))
    {
        auto& sell = context->model.sell[pursuit];
        auto  i    = 0;
        for (auto& v : sell)
        {
            auto& item  = context->model.item[v.item];
            auto  price = v.price.value_or(item.price);

            lua->pushinteger(++i);
            lua->new_table();
            {
                lua->pushinteger(1);
                lua->pushobject(item);
                lua_settable(L, -3);

                lua->pushinteger(2);
                lua->pushinteger(price);
                lua_settable(L, -3);
            }
            lua_settable(L, -3);
        }
    }

    return 1;
}

int context::builtin::builtin_pursuit_sell_price(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto pursuit = lua->tointeger(1);
    auto name    = lua->tostring(2);

    if (context->model.sell.contains(pursuit) == false)
        return 0;

    for (auto& x : context->model.sell[pursuit])
    {
        auto& model = context->model.item[x.item];
        if (model.name != name)
            continue;

        lua->pushinteger(x.price.value_or(model.price));
        return 1;
    }

    return 0;
}

int context::builtin::builtin_pursuit_sell_name(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto pursuit = lua->tointeger(1);

    if (context->model.sell_attribute.contains(pursuit) == false)
        return 0;

    lua->pushstring(context->model.sell_attribute[pursuit].group);
    return 1;
}

int context::builtin::builtin_pursuit_buy(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto pursuit = lua->tointeger(1);
    lua->new_table();
    if (context->model.buy.contains(pursuit))
    {
        auto& buy = context->model.buy[pursuit];
        auto  i   = 0;
        for (auto& [k, v] : buy)
        {
            auto& item  = context->model.item[k];
            auto  price = v.price.value_or(item.price / 2);

            lua->pushinteger(++i);
            lua->new_table();
            {
                lua->pushinteger(1);
                lua->pushobject(item);
                lua_settable(L, -3);

                lua->pushinteger(2);
                lua->pushinteger(price);
                lua_settable(L, -3);
            }
            lua_settable(L, -3);
        }
    }

    return 1;
}

int context::builtin::builtin_timer(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context  = lua->env<fb::game::context>("context");
    auto value    = (uint32_t)lua->tointeger(1);
    auto decrease = lua->toboolean(2);

    auto type = decrease ? TIMER_TYPE::DECREASE : TIMER_TYPE::INCREASE;
    context->foreach_ch([value, type](auto& ch) -> async::task<void> {
        ch.timer(value, type);
        co_return;
    });
    return 0;
}

int context::builtin::builtin_weather(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto value   = (uint32_t)lua->tointeger(1);

    context->foreach_ch([weather = WEATHER_TYPE(value)](auto& ch) -> async::task<void> {
        ch.weather(weather);
        co_return;
    });
    return 0;
}

int context::builtin::builtin_bright(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto value   = (uint32_t)lua->tointeger(1);

    context->foreach_ch([value](auto& ch) -> async::task<void> {
        ch.bright(value);
        co_return;
    });
    return 0;
}

int context::builtin::builtin_name_with(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto name = lua->tostring(1);

    if (argc < 3)
    {
        lua->pushstring(name_with(name));
    }
    else
    {
        auto first  = lua->tostring(2);
        auto second = lua->tostring(3);
        lua->pushstring(name_with(name, {first, second}));
    }
    return 1;
}

int context::builtin::builtin_assert_korean(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto text = lua->tostring(1);
    lua->pushboolean(assert_korean(text));
    return 1;
}

int context::builtin::builtin_cp949(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto argc = lua->argc();
    auto text = lua->tostring(1);
    lua->pushstring(CP949(text));
    return 1;
}

int context::builtin::builtin_debug(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

#if defined DEBUG | defined _DEBUG
    lua->pushboolean(true);
#else
    lua->pushboolean(false);
#endif
    return 1;
}

int context::builtin::builtin_name2class(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto name    = lua->tostring(1);

    auto cls       = CLASS::NONE;
    auto promotion = uint8_t{0};
    if (context->model.promotion.name2class(name, cls, promotion) == false)
        return 0;

    lua->pushinteger(cls);
    lua->pushinteger(promotion);
    return 2;
}

int context::builtin::builtin_class2name(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context   = lua->env<fb::game::context>("context");
    auto cls       = (uint8_t)lua->tointeger(1);
    auto promotion = (uint8_t)lua->tointeger(2);
    auto name      = std::string{};
    if (context->model.promotion.class2name(static_cast<CLASS>(cls), promotion, name) == false)
        lua->pushnil();
    else
        lua->pushstring(name);

    return 1;
}

int context::builtin::builtin_save(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    for (int i = 0; i < context->threads.size(); i++)
    {
        std::ignore = context->threads[i]->dispatch([context](auto& thread) -> async::task<void> {
            auto params = thread.template data<thread_params>();
            for (auto& [id, character] : params->characters)
            {
                std::ignore = context->save(*character);
            }
            co_return;
        });
    }
    return 0;
}

int context::builtin::builtin_mknpc(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto name    = lua->tostring(1);
    auto model   = context->model.npc.name2npc(name);
    if (model == nullptr)
        return 0;

    auto map = static_cast<fb::game::map*>(nullptr);
    if (lua->is_string(2))
    {
        auto name      = lua->tostring(2);
        auto map_model = context->model.map.name2map(name);
        if (map_model == nullptr)
            return 0;

        if (context->maps.contains(map_model->id) == false)
            return 0;

        map = &context->maps[map_model->id];
    }
    else if (lua->is_userdata<fb::game::map>(2))
    {
        map = lua->touserdata<fb::game::map>(2);
        if (map == nullptr)
            return 0;
    }
    else if (lua->is_userdata<fb::model::map>(2))
    {
        auto map_model = lua->touserdata<fb::model::map>(2);
        if (map_model == nullptr)
            return 0;

        if (context->maps.contains(map_model->id) == false)
            return 0;

        map = &context->maps[map_model->id];
    }
    else
    {
        return 0;
    }

    uint16_t x = 0, y = 0;
    auto     direction = DIRECTION::BOTTOM;
    if (argc < 3)
    {
        x = y = 0;
    }
    else if (lua->is_table(3))
    {
        if (argc >= 4)
            direction = static_cast<DIRECTION>(lua->tointeger(4));

        lua->rawgeti(3, 1);
        x = (uint16_t)lua->tointeger(-1);
        lua->remove(-1);

        lua->rawgeti(3, 2);
        y = (uint16_t)lua->tointeger(-1);
        lua->remove(-1);
    }
    else
    {
        if (argc >= 5)
            direction = static_cast<DIRECTION>(lua->tointeger(5));

        x = (uint16_t)lua->tointeger(3);
        y = (uint16_t)lua->tointeger(4);
    }

    if (map->thread()->id() == std::this_thread::get_id())
    {
        return lua->ensure_yield(*context, *map, [=]() {
            auto npc = model->make<fb::game::npc>(*context);
            npc->direction(direction);
            npc->map(map, fb::model::point16_t{x, y});

            return lua->ensure_resume(*context, *npc, [=]() {
                lua->pushobject(npc);
                return 1;
            });
        });
    }
    else
    {
        return lua->ensure_yield(*context, *map, [=]() {
            auto npc = model->make<fb::game::npc>(*context);
            npc->direction(direction);
            npc->map(map, fb::model::point16_t{x, y});

            return lua->ensure_resume(*context, *npc, [=]() {
                lua->pushobject(npc);
                return 1;
            });
        });
    }
}

int context::builtin::builtin_maps(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    lua->new_table();
    auto i = 1;
    for (auto& [id, map] : context->maps)
    {
        lua->pushobject(map);
        lua_rawseti(L, -2, i++);
    }
    return 1;
}

int context::builtin::builtin_shutdown(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    std::ignore  = context->http.post("internal", "/system/shutdown", Shutdown{});
    return 0;
}

int context::builtin::builtin_broadcast(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context    = lua->env<fb::game::context>("context");
    auto argc       = lua->argc();
    auto text       = lua->tostring(1);
    auto type       = lua->toenum(2, MESSAGE_TYPE::STATE);
    auto broad_type = lua->toenum(3, BROADCAST_TYPE::GLOBAL);

    if (broad_type == BROADCAST_TYPE::WORLD)
    {
        std::ignore = context->broadcast(text, type, broad_type);
        return 0;
    }
    else
    {
        async::awaitable_then(context->broadcast(text, type, broad_type), [lua](auto result) {
            lua->resume(0);
        });
        return lua->yield(0);
    }
}

int context::builtin::builtin_assert_alive(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto context = lua->env<fb::game::context>("context");
    auto argc    = lua->argc();
    auto obj     = lua->touserdata<fb::game::object>(1);

    lua->pushboolean(context->alive(*obj));
    return 1;
}