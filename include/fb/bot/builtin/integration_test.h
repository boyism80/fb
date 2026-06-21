#ifndef FB_BOT_BUILTIN_INTEGRATION_TEST_H
#define FB_BOT_BUILTIN_INTEGRATION_TEST_H

#include <fb/bot/integration/lua_integration_test.h>

struct lua_State;

namespace fb::bot::builtin {

struct integration_test
{
    static int builtin_bot(lua_State* L);
    static int builtin_bot_count(lua_State* L);
    static int builtin_sleep(lua_State* L);
    static int builtin_log(lua_State* L);
    static int builtin_hook(lua_State* L);
    static int builtin_unhook(lua_State* L);
};

} // namespace fb::bot::builtin

#endif // FB_BOT_BUILTIN_INTEGRATION_TEST_H
