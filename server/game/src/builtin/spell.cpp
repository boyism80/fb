#include <fb/game/spell.h>
#include <fb/game/server.h>
#include <fb/game/builtin/spell.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(spell, "fb.game.spell")
{"model",              builtin::spell::builtin_model},
{"delay",              builtin::spell::builtin_delay},
{"delay2",             builtin::spell::builtin_delay2},
END_LUA_EXTENSION; // clang-format on

int builtin::spell::builtin_model(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto spell = lua->touserdata<fb::game::spell>(1);
    if (spell == nullptr)
        return 0;

    auto model_ptr = std::make_shared<const fb::model::spell*>();
    auto weak      = const_cast<life&>(spell->owner).weak_from_this();
    auto builder   = lua->new_co_builder();
    builder.weak   = weak;
    builder.yield  = [=]() -> async::task<void> {
        *model_ptr = &spell->model;
        co_return;
    };
    builder.resume = [=]() -> async::task<int> {
        lua->pushobject(**model_ptr);
        co_return 1;
    };
    return builder.run();
}

int builtin::spell::builtin_delay(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto argc  = lua->argc();
    auto spell = lua->touserdata<fb::game::spell>(1);
    if (spell == nullptr)
        return 0;

    auto weak    = const_cast<life&>(spell->owner).weak_from_this();
    auto builder = lua->new_co_builder();
    builder.weak = weak;

    if (argc == 1)
    {
        auto delay    = std::make_shared<uint16_t>();
        builder.yield = [=]() -> async::task<void> {
            *delay = spell->delay();
            co_return;
        };
        builder.resume = [=]() -> async::task<int> {
            lua->pushinteger(*delay);
            co_return 1;
        };
    }
    else
    {
        auto value    = static_cast<uint16_t>(lua->tointeger(2));
        builder.yield = [=]() -> async::task<void> {
            spell->delay(value);
            co_return;
        };
        builder.resume = []() -> async::task<int> {
            co_return 0;
        };
    }
    return builder.run();
}

int builtin::spell::builtin_delay2(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;
    auto spell = lua->touserdata<fb::game::spell>(1);
    if (spell == nullptr)
        return 0;

    auto delay    = lua->tointeger(2);
    auto weak     = const_cast<life&>(spell->owner).weak_from_this();
    auto builder  = lua->new_co_builder();
    builder.weak  = weak;
    builder.yield = [=]() -> async::task<void> {
        auto& owner = const_cast<life&>(spell->owner);
        co_await owner.listener.on_spell_delay(owner, *spell, delay);
        co_return;
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}
