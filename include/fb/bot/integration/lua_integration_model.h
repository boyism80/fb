#ifndef FB_BOT_INTEGRATION_LUA_INTEGRATION_MODEL_H
#define FB_BOT_INTEGRATION_LUA_INTEGRATION_MODEL_H

#include <fb/lua.h>

namespace fb::bot::integration {

void register_model_globals(fb::lua::root& lua);

} // namespace fb::bot::integration

#endif // FB_BOT_INTEGRATION_LUA_INTEGRATION_MODEL_H
