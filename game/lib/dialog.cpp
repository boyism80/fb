#include <fb/game/dialog.h>
#include <fb/game/session.h>

fb::game::dialog::dialog(fb::game::session& owner) : 
    _owner(owner),
    _thread(nullptr)
{ }

fb::game::dialog::~dialog()
{ }

fb::game::dialog& fb::game::dialog::pushstring(const std::string& value)
{
    if(this->_thread != nullptr)
        this->_thread->pushstring(value);

    return *this;
}

fb::game::dialog& fb::game::dialog::pushinteger(int value)
{
    if(this->_thread != nullptr)
        this->_thread->pushinteger(value);

    return *this;
}

fb::game::dialog& fb::game::dialog::pushnil()
{
    if(this->_thread != nullptr)
        this->_thread->pushnil();

    return *this;
}

fb::game::dialog& fb::game::dialog::pushboolean(bool value)
{
    if(this->_thread != nullptr)
        this->_thread->pushboolean(value);

    return *this;
}

fb::game::dialog& fb::game::dialog::pushobject(const fb::game::lua::luable* object)
{
    if(this->_thread != nullptr)
        this->_thread->pushobject(object);

    return *this;
}

fb::game::dialog& fb::game::dialog::pushobject(const fb::game::lua::luable& object)
{
    if(this->_thread != nullptr)
        this->_thread->pushobject(object);
    
    return *this;
}

fb::game::dialog& fb::game::dialog::from(const char* format, ...)
{
    if(this->_thread != nullptr)
        this->_thread->release();

    this->_thread = fb::game::lua::get();
    if(this->_thread == nullptr)
        return *this;
    
    va_list args;
    va_start(args, format);
    auto buffer = fstring_c(format, &args);
    va_end(args);

    this->_thread->from(buffer.c_str());
    return *this;
}

fb::game::dialog& fb::game::dialog::func(const char* format, ...)
{
    if(this->_thread == nullptr)
        return *this;

    va_list args;
    va_start(args, format);
    auto buffer = fstring_c(format, &args);
    va_end(args);

    this->_thread->func(buffer.c_str());
    return *this;
}

bool fb::game::dialog::resume(int argc)
{
    if(this->_thread == nullptr)
        return false;

    if (this->_thread->resume(argc) == false)
    {
        this->_thread = nullptr;
        return false;
    }

    auto result = this->_thread->state();
    if(result != LUA_YIELD)
        this->_thread = nullptr;

    return result;
}

void fb::game::dialog::show(const object::model& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction)
{
    auto listener = this->_owner.get_listener<fb::game::session>();
    listener->on_dialog(this->_owner, object, message, button_prev, button_next, interaction);
}

void fb::game::dialog::show(const object& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction)
{
    return this->show(*object.based<fb::game::object>(), message, button_prev, button_next, interaction);
}

void fb::game::dialog::show(const npc::model& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction)
{
    auto listener = this->_owner.get_listener<fb::game::session>();
    listener->on_dialog(this->_owner, npc, message, menus, interaction);
}

void fb::game::dialog::show(const npc& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction)
{
    this->show(*npc.based<fb::game::npc>(), message, menus, interaction);
}

void fb::game::dialog::show(const npc::model& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction)
{
    auto listener = this->_owner.get_listener<fb::game::session>();
    listener->on_dialog(this->_owner, npc, message, item_slots, interaction);
}

void fb::game::dialog::show(const npc& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction)
{
    return this->show(*npc.based<fb::game::npc>(), message, item_slots, interaction);
}

void fb::game::dialog::show(const npc::model& npc, const std::string& message, const std::vector<std::pair<item::model*, std::optional<uint32_t>>>& pairs, uint16_t pursuit, fb::game::dialog::interaction interaction)
{
    auto listener = this->_owner.get_listener<fb::game::session>();
    listener->on_dialog(this->_owner, npc, message, pairs, pursuit, interaction);
}

void fb::game::dialog::show(const npc& npc, const std::string& message, const std::vector<std::pair<item::model*, std::optional<uint32_t>>>& pairs, uint16_t pursuit, fb::game::dialog::interaction interaction)
{
    this->show(*npc.based<fb::game::npc>(), message, pairs, pursuit, interaction);
}

void fb::game::dialog::input(const npc::model& npc, const std::string& message, fb::game::dialog::interaction interaction)
{
    auto listener = this->_owner.get_listener<fb::game::session>();
    listener->on_dialog(this->_owner, npc, message, interaction);
}

void fb::game::dialog::input(const npc& npc, const std::string& message, fb::game::dialog::interaction interaction)
{
    this->input(*npc.based<fb::game::npc>(), message, interaction);
}

void fb::game::dialog::input(const npc::model& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen, bool prev, fb::game::dialog::interaction interaction)
{
    auto listener = this->_owner.get_listener<fb::game::session>();
    listener->on_dialog(this->_owner, npc, message, top, bottom, maxlen, prev, interaction);
}

void fb::game::dialog::input(const npc& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen, bool prev, fb::game::dialog::interaction interaction)
{
    this->input(*npc.based<fb::game::npc>(), message, top, bottom, maxlen, prev, interaction);
}
