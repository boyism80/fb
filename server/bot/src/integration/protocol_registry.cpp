#include <fb/bot/integration/protocol_registry.h>
#include <fb/bot/integration/protocol_registry.generated.h>
#include <fb/bot/game_controller.h>
#include <fb/lua.h>

#include <cstring>

namespace fb::bot::integration {

void protocol_registry::marshal_lua_stub(lua_State* L, const fb::protocol::header&)
{
    lua_newtable(L);
}

const protocol_entry* protocol_registry::find(protocol_direction direction, const char* name)
{
    if (name == nullptr)
        return nullptr;

    for (size_t i = 0; i < entry_count(); ++i)
    {
        auto& entry = entries()[i];
        if (entry.direction != direction)
            continue;

        if (std::strcmp(entry.name, name) == 0)
            return &entry;
    }

    return nullptr;
}

const protocol_entry* protocol_registry::find_by_type_key(const char* type_key)
{
    if (type_key == nullptr)
        return nullptr;

    for (size_t i = 0; i < entry_count(); ++i)
    {
        auto& entry = entries()[i];
        if (std::strcmp(entry.type_key, type_key) == 0)
            return &entry;
    }

    return nullptr;
}

void protocol_registry::register_all()
{
    for (size_t i = 0; i < entry_count(); ++i)
    {
        auto& entry = entries()[i];

        if (entry.direction == protocol_direction::response && entry.clone != nullptr)
        {
            bot_controller<game_bot>::register_response_cloner(entry.type_key, entry.clone);
        }
    }
}

void protocol_registry::bind(game_bot_controller& controller)
{
    for (size_t i = 0; i < entry_count(); ++i)
    {
        auto& entry = entries()[i];
        if (entry.bind != nullptr)
            entry.bind(controller);
    }
}

} // namespace fb::bot::integration
