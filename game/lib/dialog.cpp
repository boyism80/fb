#include <character.h>
#include <dialog.h>

fb::game::dialog::dialog(fb::game::character& owner) :
    _owner(owner)
{ }

fb::game::dialog::~dialog()
{
    this->release();
}

fb::game::lua::context* fb::game::dialog::current() const
{
    if (this->_scripts.size() == 0)
        return nullptr;

    return this->_scripts.top();
}

fb::game::dialog& fb::game::dialog::resume(int argc)
{
    auto ctx = this->current();
    if (ctx == nullptr)
        return *this;

    ctx->resume(argc, false);
    switch (ctx->state())
    {
    case LUA_YIELD:
    case LUA_PENDING:
        return *this;

    case LUA_OK:
        this->_scripts.pop();
        break;

    default:
        this->_scripts.pop();
        return *this;
    }

    auto prev        = ctx;
    auto current     = this->current();
    auto return_size = lua_gettop(*ctx);
    if (current != nullptr)
    {
        for (int i = 0; i < return_size; i++)
        {
            auto index = i - return_size;
            switch (lua_type(*prev, index))
            {
            case LUA_TNUMBER:
                current->pushnumber(prev->tonumber(index));
                break;

            case LUA_TSTRING:
                current->pushstring(prev->tostring(index));
                break;

            case LUA_TNIL:
                current->pushnil();
                break;

            case LUA_TBOOLEAN:
                current->pushboolean(prev->toboolean(index));
                break;
            }
        }
    }

    prev->release();
    return this->resume(return_size);
}

fb::game::dialog& fb::game::dialog::release()
{
    while (true)
    {
        auto ctx = this->current();
        if (ctx == nullptr)
            break;

        ctx->release();
        this->_scripts.pop();
    }

    return *this;
}

bool fb::game::dialog::active() const
{
    auto ctx = this->current();
    return ctx != nullptr;
}

fb::game::dialog& fb::game::dialog::new_table()
{
    auto ctx = this->current();
    if (ctx == nullptr)
        throw inactive_error();

    ctx->new_table();
    return *this;
}

fb::game::dialog& fb::game::dialog::set_table()
{
    auto ctx = this->current();
    if (ctx == nullptr)
        throw inactive_error();

    lua_settable(*ctx, -3);
    return *this;
}

fb::game::dialog& fb::game::dialog::pushstring(const std::string& value)
{
    auto ctx = this->current();
    if (ctx == nullptr)
        throw inactive_error();

    ctx->pushstring(value);
    return *this;
}

fb::game::dialog& fb::game::dialog::pushinteger(lua_Integer value)
{
    auto ctx = this->current();
    if (ctx == nullptr)
        throw inactive_error();

    ctx->pushinteger(value);
    return *this;
}

fb::game::dialog& fb::game::dialog::pushnil()
{
    auto ctx = this->current();
    if (ctx == nullptr)
        throw inactive_error();

    ctx->pushnil();
    return *this;
}

fb::game::dialog& fb::game::dialog::pushboolean(bool value)
{
    auto ctx = this->current();
    if (ctx == nullptr)
        throw inactive_error();

    ctx->pushboolean(value);
    return *this;
}

fb::game::dialog& fb::game::dialog::pushobject(const fb::game::lua::luable* object)
{
    auto ctx = this->current();
    if (ctx == nullptr)
        throw inactive_error();

    ctx->pushobject(object);
    return *this;
}

fb::game::dialog& fb::game::dialog::pushobject(const fb::game::lua::luable& object)
{
    auto ctx = this->current();
    if (ctx == nullptr)
        throw inactive_error();

    ctx->pushobject(object);
    return *this;
}

// new
void fb::game::dialog::show(const fb::model::object& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction)
{
    auto listener = this->_owner.get_listener<fb::game::character>();
    if (listener != nullptr)
        listener->on_dialog(this->_owner, object, message, button_prev, button_next, interaction);
}

void fb::game::dialog::show(const fb::model::npc& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction)
{
    auto listener = this->_owner.get_listener<fb::game::character>();
    if (listener != nullptr)
        listener->on_dialog(this->_owner, npc, message, menus, interaction);
}

void fb::game::dialog::show(const fb::model::npc& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction)
{
    auto listener = this->_owner.get_listener<fb::game::character>();
    if (listener != nullptr)
        listener->on_dialog(this->_owner, npc, message, item_slots, interaction);
}

void fb::game::dialog::show(const fb::model::npc&               npc,
                            const std::string&                  message,
                            const fb::game::dialog::item_pairs& pairs,
                            uint16_t                            pursuit,
                            fb::game::dialog::interaction       interaction)
{
    auto listener = this->_owner.get_listener<fb::game::character>();
    if (listener != nullptr)
        listener->on_dialog(this->_owner, npc, message, pairs, pursuit, interaction);
}

void fb::game::dialog::input(const fb::model::npc& npc, const std::string& message, fb::game::dialog::interaction interaction)
{
    auto listener = this->_owner.get_listener<fb::game::character>();
    if (listener != nullptr)
        listener->on_dialog(this->_owner, npc, message, interaction);
}

void fb::game::dialog::input(const fb::model::npc&         npc,
                             const std::string&            message,
                             const std::string&            top,
                             const std::string&            bottom,
                             int                           maxlen,
                             bool                          prev,
                             fb::game::dialog::interaction interaction)
{
    auto listener = this->_owner.get_listener<fb::game::character>();
    if (listener != nullptr)
        listener->on_dialog(this->_owner, npc, message, top, bottom, maxlen, prev, interaction);
}