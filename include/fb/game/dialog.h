#ifndef __DIALOG_H__
#define __DIALOG_H__

#include <stdint.h>
#include <string>
#include <stack>
#include <fb/game/npc.h>
#include <fb/game/item.h>

namespace fb { namespace game {

class session;

class dialog
{
public:
    enum class interaction : uint8_t
    {
        NORMAL,
        INPUT,
        INPUT_EX,
        MENU,
        SLOT,
        ITEM,
    };

public:
    interface listener
    {
        virtual void        on_dialog(session& me, const object::model& object, const std::string& message, bool button_prev, bool button_next, interaction interaction = interaction::NORMAL) = 0;
        virtual void        on_dialog(session& me, const npc::model& npc, const std::string& message, const std::vector<std::string>& menus, interaction interaction = interaction::NORMAL) = 0;
        virtual void        on_dialog(session& me, const npc::model& npc, const std::string& message, const std::vector<uint8_t>& item_slots, interaction interaction = interaction::NORMAL) = 0;
        virtual void        on_dialog(session& me, const npc::model& npc, const std::string& message, const std::vector<std::pair<item::model*, std::optional<uint32_t>>>& pairs, uint16_t pursuit = 0xFFFF, interaction interaction = interaction::NORMAL) = 0;
        virtual void        on_dialog(session& me, const npc::model& npc, const std::string& message, interaction interaction = interaction::NORMAL) = 0;
        virtual void        on_dialog(session& me, const npc::model& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false, interaction interaction = interaction::NORMAL) = 0;
    };

public:
    interface listener;

public:
    using lua_stack = std::stack<lua::context*>;

private:
    session&                _owner;
    lua_stack               _scripts;


public:
    dialog(session& owner);
    ~dialog();

public:
    lua::context&           from(const char* format, ...);
    lua::context&           func(const char* format, ...);
    lua::context*           current();
    void                    resume(int argc);

public:
    void                    show(const object::model& object, const std::string& message, bool button_prev = false, bool button_next = true, interaction interaction = interaction::NORMAL);
    void                    show(const object& object, const std::string& message, bool button_prev = false, bool button_next = true, interaction interaction = interaction::NORMAL);
    void                    show(const npc::model& npc, const std::string& message, const std::vector<std::string>& menus, interaction interaction = interaction::MENU);
    void                    show(const npc& npc, const std::string& message, const std::vector<std::string>& menus, interaction interaction = interaction::MENU);
    void                    show(const npc::model& npc, const std::string& message, const std::vector<uint8_t>& item_slots, interaction interaction = interaction::SLOT);
    void                    show(const npc& npc, const std::string& message, const std::vector<uint8_t>& item_slots, interaction interaction = interaction::SLOT);
    void                    show(const npc::model& npc, const std::string& message, const std::vector<std::pair<item::model*, std::optional<uint32_t>>>& pairs, uint16_t pursuit = 0xFFFF, interaction interaction = interaction::ITEM);
    void                    show(const npc& npc, const std::string& message, const std::vector<std::pair<item::model*, std::optional<uint32_t>>>& pairs, uint16_t pursuit = 0xFFFF, interaction interaction = interaction::ITEM);
    void                    input(const npc::model& npc, const std::string& message, interaction interaction = interaction::INPUT);
    void                    input(const npc& npc, const std::string& message, interaction interaction = interaction::INPUT);
    void                    input(const npc::model& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false, interaction interaction = interaction::INPUT_EX);
    void                    input(const npc& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false, interaction interaction = interaction::INPUT_EX);
};

} }

#endif