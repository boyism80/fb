#ifndef FB_GAME_BUILTIN_SERVER_H
#define FB_GAME_BUILTIN_SERVER_H

#include <fb/game/server.h>

namespace fb::game::builtin {

struct server
{
    static int builtin_log(lua_State* L);
    static int builtin_seed(lua_State* L);
    static int builtin_sleep(lua_State* L);
    static int builtin_now(lua_State* L);
    static int builtin_time_forward(lua_State* L);
    static int builtin_time_backward(lua_State* L);
    static int builtin_datetime(lua_State* L);
    static int builtin_to_lunar(lua_State* L);
    static int builtin_from_lunar(lua_State* L);
    static int builtin_name2mob(lua_State* L);
    static int builtin_name2spell(lua_State* L);
    static int builtin_name2npc(lua_State* L);
    static int builtin_name2map(lua_State* L);
    static int builtin_name2ch(lua_State* L);
    static int builtin_name2item(lua_State* L);
    static int builtin_id2mob(lua_State* L);
    static int builtin_id2spell(lua_State* L);
    static int builtin_id2npc(lua_State* L);
    static int builtin_id2map(lua_State* L);
    static int builtin_id2ch(lua_State* L);
    static int builtin_id2item(lua_State* L);
    static int builtin_id2clan(lua_State* L);
    static int builtin_castle(lua_State* L);
    static int builtin_siege_active(lua_State* L);
    static int builtin_pursuit_sell(lua_State* L);
    static int builtin_pursuit_sell_price(lua_State* L);
    static int builtin_pursuit_buy(lua_State* L);
    static int builtin_timer(lua_State* L);
    static int builtin_weather(lua_State* L);
    static int builtin_weather_reroll(lua_State* L);
    static int builtin_bright(lua_State* L);
    static int builtin_name_with(lua_State* L);
    static int builtin_assert_korean(lua_State* L);
    static int builtin_cp949(lua_State* L);
    static int builtin_broadcast(lua_State* L);
    static int builtin_assert_alive(lua_State* L);
    static int builtin_debug(lua_State* L);
    static int builtin_name2class(lua_State* L);
    static int builtin_class2name(lua_State* L);
    static int builtin_save(lua_State* L);
    static int builtin_reload_table(lua_State* L);
    static int builtin_mknpc(lua_State* L);
    static int builtin_maps(lua_State* L);
    static int builtin_shutdown(lua_State* L);
    static int builtin_ban(lua_State* L);
    static int builtin_unban(lua_State* L);
    static int builtin_regex(lua_State* L);
    static int builtin_exp_multiplier(lua_State* L);
    static int builtin_drop_rate_multiplier(lua_State* L);
    static int builtin_http_response_delay(lua_State* L);
    static int builtin_property(lua_State* L);
};

} // namespace fb::game::builtin

#endif // FB_GAME_BUILTIN_SERVER_H