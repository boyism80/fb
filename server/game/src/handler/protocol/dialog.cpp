#include <fb/game/handler/protocol/dialog.h>
#include <fb/game/server.h>
#include <algorithm>
#include <cstring>

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
dialog<V>::dialog(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::dialog<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> dialog<V>::handle(fb::socket<character>& session, game_reqs::dialog<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    if (ch->dialog == nullptr)
        co_return true;

    auto lua   = ch->dialog;
    ch->dialog = nullptr;
    switch (request.type)
    {
    case fb::game::dialog::type::INPUT:
    case fb::game::dialog::type::INPUT_NO_EXT:
        lua->pushstring(request.message);
        lua->resume(1);
        break;

    case fb::game::dialog::type::MENU:
    case fb::game::dialog::type::MENU_NO_EXT:
        lua->pushinteger(request.index);
        lua->resume(1);
        break;

    case fb::game::dialog::type::SLOT:
    case fb::game::dialog::type::SPELL:
        lua->pushinteger(request.index);
        lua->resume(1);
        break;

    case fb::game::dialog::type::ITEM:
    case fb::game::dialog::type::PURSUIT:
    {
        auto found = lua_Integer{0};
        if (lua->ref != LUA_NOREF)
        {
            lua_rawgeti(*lua, LUA_REGISTRYINDEX, lua->ref);
            if (lua_istable(*lua, -1))
            {
                auto n = static_cast<int>(lua_rawlen(*lua, -1));
                for (int i = 1; i <= n; i++)
                {
                    lua_rawgeti(*lua, -1, i);
                    auto label = std::string{};
                    if (lua->is_string(-1))
                    {
                        label = lua->tostring(-1);
                    }
                    else if (lua->is_table(-1))
                    {
                        lua_rawgeti(*lua, -1, 1);
                        if (lua->is_string(-1))
                            label = lua->tostring(-1);
                        lua->pop(1);
                    }
                    lua->pop(1);
                    if (label.empty() == false && label == request.name)
                    {
                        found = i;
                        break;
                    }
                }
            }
            lua->pop(1);
        }

        if (found != 0)
            lua->pushinteger(found);
        else
            lua->pushnil();
        lua->resume(1);
        break;
    }

    case fb::game::dialog::type::DUAL_FIELD:
    {
        auto found = lua_Integer{0};
        if (lua->ref != LUA_NOREF)
        {
            lua_rawgeti(*lua, LUA_REGISTRYINDEX, lua->ref);
            if (lua_istable(*lua, -1))
            {
                auto n = static_cast<int>(lua_rawlen(*lua, -1));
                for (int i = 1; i <= n; i++)
                {
                    lua_rawgeti(*lua, -1, i);
                    auto label = std::string{};
                    if (lua->is_string(-1))
                    {
                        label = lua->tostring(-1);
                    }
                    else if (lua->is_table(-1))
                    {
                        lua_rawgeti(*lua, -1, 1);
                        if (lua->is_string(-1))
                            label = lua->tostring(-1);
                        lua->pop(1);
                    }
                    lua->pop(1);
                    if (label.empty())
                        continue;

                    if constexpr (V == fb::protocol::CLIENT_VERSION::v651)
                    {
                        uint8_t prefix[4] = {
                            static_cast<uint8_t>(request.item_value >> 24),
                            static_cast<uint8_t>(request.item_value >> 16),
                            static_cast<uint8_t>(request.item_value >> 8),
                            static_cast<uint8_t>(request.item_value),
                        };
                        auto bytes = std::min<size_t>(4, label.size());
                        if (bytes != 0 && std::memcmp(label.data(), prefix, bytes) == 0)
                        {
                            found = i;
                            break;
                        }
                    }
                    else if (label == request.name)
                    {
                        found = i;
                        break;
                    }
                }
            }
            lua->pop(1);
        }

        if (found != 0)
            lua->pushinteger(found);
        else
            lua->pushnil();
        lua->resume(1);
        break;
    }

    default:
        lua->release();
        break;
    }

    co_return true;
}

template class dialog<fb::protocol::CLIENT_VERSION::v550>;
template class dialog<fb::protocol::CLIENT_VERSION::v565>;
template class dialog<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
