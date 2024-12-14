#include <fb/lua.h>

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::lua::luable, "fb.luable")
{"__gc",                fb::lua::luable::builtin_gc},
END_LUA_EXTENSION; // clang-format on