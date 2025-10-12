#include <fb/game/server.h>
#include <fb/model/model.h>
#include <fb/game/builtin/model.h>

using namespace fb::game;

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::model::equipment, "fb.model.equipment")
{"dress",               builtin::model::equipment::builtin_dress},
END_LUA_EXTENSION; // clang-format on

int builtin::model::equipment::builtin_dress(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto model = lua->touserdata<fb::model::equipment>(1);
    lua->pushinteger(model->dress);
    return 1;
}