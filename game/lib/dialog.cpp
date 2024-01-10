#include <fb/game/dialog.h>
#include <fb/game/session.h>

fb::game::dialog::dialog(fb::game::session& owner) : 
    _owner(owner)
{ }

fb::game::dialog::~dialog()
{ }

fb::game::lua::context& fb::game::dialog::from(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    auto buffer = fstring_c(format, &args);
    va_end(args);

    auto ctx = fb::game::lua::get();
    ctx->from(buffer.c_str());
    this->_scripts.push(ctx);
    return *ctx;
}

fb::game::lua::context& fb::game::dialog::func(const char* format, ...)
{
    auto ctx = this->current();
    if(ctx == nullptr)
        throw std::runtime_error("current lua context is empty");

    va_list args;
    va_start(args, format);
    auto buffer = fstring_c(format, &args);
    va_end(args);

    ctx->func(buffer.c_str());
    return *ctx;
}

fb::game::lua::context* fb::game::dialog::current()
{
    if(this->_scripts.size() == 0)
        return nullptr;

    return this->_scripts.top();
}

void fb::game::dialog::resume(int argc)
{
    auto ctx = this->current();
    if(ctx == nullptr)
        return;

    auto success = ctx->resume(argc);
    switch(ctx->state())
    {
        case LUA_YIELD:
        case LUA_PENDING:
            return;

        default:
            this->_scripts.pop();
            break;
    }

    while(true)
    {
        auto next = this->current();
        if(next == nullptr)
            break;

        next->resume(0);
        auto state = next->state();
        if(state == LUA_YIELD || state == LUA_PENDING)
            break;

        this->_scripts.pop();
    }
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
