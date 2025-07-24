#include <fb/lua.h>

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::lua::luable, "fb.luable")
{"__gc",                fb::lua::luable::builtin_gc<const luable*>},
END_LUA_EXTENSION; // clang-format on