#ifndef __BOT_INTEGRATION_LUA_INTEGRATION_PROTOCOL_H__
#define __BOT_INTEGRATION_LUA_INTEGRATION_PROTOCOL_H__

#include <fb/protocol/client_version.h>
#include <fb/protocol/header.h>

#include <functional>
#include <memory>

struct lua_State;

namespace fb::lua {
class root;
}

namespace fb::bot::integration {

struct protocol_entry;

namespace lua_protocol {

constexpr const char* RESPONSE_METATABLE = "fb.integration.response.token";
constexpr const char* REQUEST_METATABLE  = "fb.integration.protocol.request";

using request_factory = std::function<std::shared_ptr<fb::protocol::header>(fb::protocol::CLIENT_VERSION)>;

void register_metatables(lua_State* L);
void register_modules(lua_State* L);
void register_modules(fb::lua::root& root);
void register_manual_builders(lua_State* L);

const protocol_entry*                 to_response_token(lua_State* L, int index);
std::shared_ptr<fb::protocol::header> to_request(lua_State* L, int index, fb::protocol::CLIENT_VERSION version);
void                                  push_request(lua_State* L, request_factory factory);
void                                  push_response_token(lua_State* L, const protocol_entry* entry);

template <template <fb::protocol::CLIENT_VERSION> class RequestType, typename... Args>
void push_request(lua_State* L, Args... args)
{
    push_request(L, request_factory([args...](fb::protocol::CLIENT_VERSION version) {
                     return fb::protocol::visit_client_version(
                         version,
                         [&]<fb::protocol::CLIENT_VERSION V>() -> std::shared_ptr<fb::protocol::header> {
                             return std::make_shared<RequestType<V>>(args...);
                         });
                 }));
}

} // namespace lua_protocol
} // namespace fb::bot::integration

#endif
