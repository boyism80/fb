#include <fb/game/handler/protocol/dialog.h>

#include <fb/game/item/base.h>
#include <fb/game/server.h>

#include <string>

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
dialog<V>::dialog(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::dialog<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
lua_Integer dialog<V>::selected(fb::lua::context* lua, const game_reqs::dialog<V>& request)
{
    if (lua->ref == LUA_NOREF)
        return 0;

    lua_rawgeti(*lua, LUA_REGISTRYINDEX, lua->ref);
    auto found = lua_Integer{0};
    if (lua_istable(*lua, -1))
    {
        auto n = static_cast<int>(lua_rawlen(*lua, -1));
        if constexpr (V == fb::protocol::CLIENT_VERSION::v651)
        {
            if (request.type == fb::game::dialog::type::BUY)
            {
                found = static_cast<lua_Integer>(request.id);
                if (found < 1 || found > n)
                    found = 0;
                lua->pop(1);
                return found;
            }
        }

        for (int i = 1; i <= n; i++)
        {
            lua_rawgeti(*lua, -1, i);
            auto label = std::string{};
            if (lua->is_string(-1))
            {
                label = lua->tostring(-1);
            }
            else if (lua->is_userdata<fb::model::item>(-1))
            {
                auto model = lua->touserdata<fb::model::item>(-1);
                if (model != nullptr)
                    label = model->name;
            }
            else if (lua->is_userdata<fb::game::item>(-1))
            {
                auto item = lua->touserdata<fb::game::item>(-1);
                if (item != nullptr)
                    label = item->name();
            }
            else if (lua->is_table(-1))
            {
                lua_rawgeti(*lua, -1, 1);
                if (lua->is_string(-1))
                {
                    label = lua->tostring(-1);
                }
                else if (lua->is_userdata<fb::model::item>(-1))
                {
                    auto model = lua->touserdata<fb::model::item>(-1);
                    if (model != nullptr)
                        label = model->name;
                }
                else if (lua->is_userdata<fb::game::item>(-1))
                {
                    auto item = lua->touserdata<fb::game::item>(-1);
                    if (item != nullptr)
                        label = item->name();
                }
                lua->pop(1);

                if (label.empty())
                {
                    lua->pushstring("model");
                    if (lua_rawget(*lua, -2) != LUA_TNIL)
                    {
                        if (lua->is_userdata<fb::model::item>(-1))
                        {
                            auto model = lua->touserdata<fb::model::item>(-1);
                            if (model != nullptr)
                                label = model->name;
                        }
                        else if (lua->is_userdata<fb::game::item>(-1))
                        {
                            auto item = lua->touserdata<fb::game::item>(-1);
                            if (item != nullptr)
                                label = item->name();
                        }
                        else if (lua->is_string(-1))
                        {
                            label = lua->tostring(-1);
                        }
                    }
                    lua->pop(1);
                }
            }
            lua->pop(1);
            if (label.empty())
                continue;

            if (label == request.name)
            {
                found = i;
                break;
            }
        }
    }
    lua->pop(1);
    return found;
}

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> dialog<V>::handle(fb::socket<character>& session, game_reqs::dialog<V>& request)
{
    auto ch = session.data();
    if (ch == nullptr || ch->inited() == false)
        co_return true;

    auto response = fb::game::dialog::response::MENU;
    switch (request.type)
    {
    case fb::game::dialog::type::INPUT:
    case fb::game::dialog::type::INPUT_NO_EXT:
        response = fb::game::dialog::response::INPUT;
        break;

    case fb::game::dialog::type::MENU:
    case fb::game::dialog::type::MENU_NO_EXT:
        response = fb::game::dialog::response::MENU;
        break;

    case fb::game::dialog::type::SLOT:
    case fb::game::dialog::type::SPELL:
        response = fb::game::dialog::response::SLOT;
        break;

    case fb::game::dialog::type::ITEM:
    case fb::game::dialog::type::PURSUIT:
    case fb::game::dialog::type::BUY:
        response = fb::game::dialog::response::SELECT;
        break;

    default:
        co_return true;
    }

    auto lua = ch->take_dialog(response);
    if (lua == nullptr)
        co_return true;

    auto choices = ch->dialog_choices();
    auto reply   = request;
    co_await lua->switching();

    switch (reply.type)
    {
    case fb::game::dialog::type::INPUT:
    case fb::game::dialog::type::INPUT_NO_EXT:
        lua->pushstring(reply.message);
        lua->resume(1);
        break;

    case fb::game::dialog::type::MENU:
    case fb::game::dialog::type::MENU_NO_EXT:
        if (reply.index >= 1 && reply.index <= choices)
            lua->pushinteger(reply.index);
        else
            lua->pushnil();
        lua->resume(1);
        break;

    case fb::game::dialog::type::SLOT:
    case fb::game::dialog::type::SPELL:
        lua->pushinteger(reply.index);
        lua->resume(1);
        break;

    case fb::game::dialog::type::ITEM:
    case fb::game::dialog::type::PURSUIT:
    {
        auto found = this->selected(lua, reply);
        if (found != 0)
            lua->pushinteger(found);
        else
            lua->pushnil();
        lua->resume(1);
        break;
    }

    case fb::game::dialog::type::BUY:
    {
        // Yields: selected (1-based index or nil), count (v651 NEW quantity; else nil)
        auto found = this->selected(lua, reply);
        if (found != 0)
        {
            lua->pushinteger(found);
            if constexpr (V == fb::protocol::CLIENT_VERSION::v651)
                lua->pushinteger(reply.count);
            else
                lua->pushnil();
            lua->resume(2);
        }
        else
        {
            lua->pushnil();
            lua->resume(1);
        }
        break;
    }

    default:
        break;
    }

    co_return true;
}

template class dialog<fb::protocol::CLIENT_VERSION::v550>;
template class dialog<fb::protocol::CLIENT_VERSION::v565>;
template class dialog<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
