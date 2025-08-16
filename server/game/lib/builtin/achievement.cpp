#include <fb/game/achievement.h>
#include <fb/game/builtin/achievement.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(achievement, "fb.game.achievement")
{"model",              	builtin::achievement::builtin_model},
{"text",               	builtin::achievement::builtin_text},
END_LUA_EXTENSION; // clang-format on

int builtin::achievement::builtin_model(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto achievement = lua->touserdata<fb::game::achievement>(1);
    if (achievement == nullptr)
        return 0;

    lua->pushobject(achievement->model);
    return 1;
}

int builtin::achievement::builtin_text(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto achievement = lua->touserdata<fb::game::achievement>(1);
    if (achievement == nullptr)
        return 0;

    lua->pushstring(achievement->text.value_or(achievement->model.text));
    return 1;
}