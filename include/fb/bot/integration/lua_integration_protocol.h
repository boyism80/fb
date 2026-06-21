#ifndef __BOT_INTEGRATION_LUA_INTEGRATION_PROTOCOL_H__
#define __BOT_INTEGRATION_LUA_INTEGRATION_PROTOCOL_H__

#include <memory>

struct lua_State;

namespace fb::lua {
class root;
}

namespace fb::protocol {
class header;
}

namespace fb::bot::integration {

struct protocol_entry;

namespace lua_protocol {

constexpr const char* RESPONSE_METATABLE = "fb.integration.response.token";
constexpr const char* REQUEST_METATABLE  = "fb.integration.protocol.request";

void register_metatables(lua_State* L);
void register_modules(lua_State* L);
void register_modules(fb::lua::root& root);
void register_manual_builders(lua_State* L);

const protocol_entry*                 to_response_token(lua_State* L, int index);
std::shared_ptr<fb::protocol::header> to_request(lua_State* L, int index);
void                                  push_request(lua_State* L, std::shared_ptr<fb::protocol::header> request);
void                                  push_response_token(lua_State* L, const protocol_entry* entry);

} // namespace lua_protocol
} // namespace fb::bot::integration

#endif
