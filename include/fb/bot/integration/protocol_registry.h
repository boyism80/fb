#ifndef __BOT_INTEGRATION_PROTOCOL_REGISTRY_H__
#define __BOT_INTEGRATION_PROTOCOL_REGISTRY_H__

#include <cstddef>
#include <cstdint>
#include <memory>

#include <fb/bot/integration/protocol_registry.generated.h>

struct lua_State;

namespace fb::protocol {
class header;
}

namespace fb::bot {
class game_bot_controller;
}

namespace fb::bot::integration {

enum class protocol_direction : uint8_t
{
    request,
    response,
};

struct protocol_entry
{
    const char*        type_key;
    const char*        name;
    protocol_direction direction;
    uint8_t            opcode;

    void (*bind)(game_bot_controller&);
    std::shared_ptr<fb::protocol::header> (*clone)(const fb::protocol::header&);
    std::shared_ptr<fb::protocol::header> (*create)();
    void (*marshal_lua)(lua_State* L, const fb::protocol::header& header);
};

class protocol_registry
{
public:
    static constexpr size_t entry_count()
    {
        return FB_BOT_INTEGRATION_PROTOCOL_ENTRY_COUNT;
    }
    static const protocol_entry* entries();

    static const protocol_entry* find(protocol_direction direction, const char* name);
    static const protocol_entry* find_by_type_key(const char* type_key);

    static void register_all();
    static void bind(game_bot_controller& controller);
    static void marshal_lua_stub(lua_State* L, const fb::protocol::header& header);
};

} // namespace fb::bot::integration

#endif
