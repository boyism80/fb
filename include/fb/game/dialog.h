#ifndef __DIALOG_H__
#define __DIALOG_H__

#include <stdint.h>
#include <string>
#include <fb/game/npc.h>
#include <fb/game/item.h>

namespace fb { namespace game {

class session;

class dialog
{
public:
    enum class interaction : uint8_t
    {
        NORMAL              = 0x00,
        INPUT               = 0x01,
        INPUT_EX            = 0x02,
        MENU                = 0x04,
        SLOT                = 0x05,
        SALE                = 0x06,
        BUY                 = 0x07,
    };

public:
    interface listener
    {
        virtual void        on_dialog(session& me, const object::model& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) = 0;
        virtual void        on_dialog(session& me, const fb::game::npc::model& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) = 0;
        virtual void        on_dialog(session& me, const fb::game::npc::model& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) = 0;
        virtual void        on_dialog(session& me, const fb::game::npc::model& npc, const std::string& message, const std::map<item::model*, std::optional<uint32_t>>& pairs, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) = 0;
        virtual void        on_dialog(session& me, const fb::game::npc::model& npc, const std::string& message, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) = 0;
        virtual void        on_dialog(session& me, const fb::game::npc::model& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) = 0;
    };

public:
    interface listener;

private:
    session&                _owner;
    fb::game::lua::context* _thread = nullptr;

public:
    dialog(fb::game::session& owner);
    ~dialog();

public:
    dialog&                 pushstring(const std::string& value);
    dialog&                 pushinteger(int value);
    dialog&                 pushnil();
    dialog&                 pushboolean(bool value);
    dialog&                 pushobject(const fb::game::lua::luable* object);
    dialog&                 pushobject(const fb::game::lua::luable& object);

public:
    dialog&                 from(const char* format, ...);
    dialog&                 func(const char* format, ...);
    bool                    resume(int argc);

public:
    void                    show(const object::model& object, const std::string& message, bool button_prev = false, bool button_next = true, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);
    void                    show(const object& object, const std::string& message, bool button_prev = false, bool button_next = true, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL);
    void                    show(const npc::model& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::MENU);
    void                    show(const npc& npc, const std::string& message, const std::vector<std::string>& menus, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::MENU);
    void                    show(const npc::model& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::SLOT);
    void                    show(const npc& npc, const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::SLOT);
    void                    show(const npc::model& npc, const std::string& message, const std::map<fb::game::item::model*, std::optional<uint32_t>>& pairs, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::SLOT);
    void                    show(const npc& npc, const std::string& message, const std::map<fb::game::item::model*, std::optional<uint32_t>>& pairs, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::SLOT);
    void                    input(const npc::model& npc, const std::string& message, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::INPUT);
    void                    input(const npc& npc, const std::string& message, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::INPUT);
    void                    input(const npc::model& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::INPUT_EX);
    void                    input(const npc& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::INPUT_EX);
};

} }

#endif