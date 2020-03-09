#ifndef __NPC_H__
#define __NPC_H__

#include "item.h"
#include "map.h"

namespace fb { namespace game {

class npc : public object
{
public:
    class core : public object::core
    {
        friend class npc;

	public:
		DECLARE_LUA_PROTOTYPE

    public:
        core(const std::string& name, uint16_t look, uint8_t color = 0);
        ~core();

    public:
        using object::core::make_dialog_stream;

    public:
        object::types       type() const { return object::types::NPC; }
        object*             make() const;

    public:
        fb::ostream         make_dialog_stream(const std::string& message, const std::vector<std::string>& menus, fb::game::map* map = nullptr, dialog::interaction interaction = dialog::interaction::MENU) const;
        fb::ostream         make_dialog_stream(const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::map* map = nullptr, dialog::interaction interaction = dialog::interaction::SLOT) const;
        fb::ostream         make_dialog_stream(const std::string& message, const std::vector<item::core*>& cores, fb::game::map* map = nullptr, uint16_t pursuit = 0xFFFF, dialog::interaction interaction = dialog::interaction::SELL) const;
        fb::ostream         make_input_dialog_stream(const std::string& message, fb::game::map* map = nullptr, dialog::interaction interaction = dialog::interaction::INPUT) const;
		fb::ostream			make_input_dialog_stream(const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false, fb::game::map* map = nullptr, dialog::interaction interaction = dialog::interaction::INPUT_EX) const;

	public:
		IMPLEMENT_NEW_LUA

        static int          builtin_input_dialog(lua_State* lua);
        static int          builtin_menu_dialog(lua_State* lua);
        static int          builtin_item_dialog(lua_State* lua);
    };

private:
    std::string             _script;

public:
    npc(const core* core);
    npc(const npc& right);
    ~npc();

public:
    using object::make_dialog_stream;

public:
	DECLARE_LUA_PROTOTYPE
	BUILTIN_CORE(fb::game::npc)

public:
    const std::string&      script() const;
    void                    script(const std::string& value);

public:
    npc*                    make();

public:
    fb::ostream             make_dialog_stream(const std::string& message, const std::vector<std::string>& menus) const;
    fb::ostream             make_dialog_stream(const std::string& message, const std::vector<uint8_t>& item_slots) const;
    fb::ostream             make_dialog_stream(const std::string& message, const std::vector<item::core*>& cores, fb::game::map* map = nullptr) const;
    fb::ostream             make_input_dialog_stream(const std::string& message, fb::game::map* map = nullptr) const;
	fb::ostream				make_input_dialog_stream(const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false) const;

public:
	IMPLEMENT_NEW_LUA

    static int              builtin_input_dialog(lua_State* lua);
    static int              builtin_menu_dialog(lua_State* lua);
    static int              builtin_item_dialog(lua_State* lua);
};

} }

#endif // !__NPC_H__
