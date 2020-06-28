#ifndef __NPC_H__
#define __NPC_H__

#include <model/item/item.h>
#include <model/map/map.h>

namespace fb { namespace game {

class listener;

class npc : public object
{
#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion


#pragma region master class
public:
class master : public object::master
{
#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion


#pragma region friend
public:
    friend class npc;
#pragma endregion


#pragma region constructor / destructor
public:
    master(const std::string& name, uint16_t look, uint8_t color = 0);
    ~master();
#pragma endregion


#pragma region using parent method
public:
    using object::master::make_dialog_stream;
#pragma endregion

#pragma region public method
public:
    object::types               type() const { return object::types::NPC; }
    object*                     make(listener* listener) const;

public:
    fb::ostream                 make_dialog_stream(const std::string& message, const std::vector<std::string>& menus, fb::game::map* map = nullptr, dialog::interaction interaction = dialog::interaction::MENU) const;
    fb::ostream                 make_dialog_stream(const std::string& message, const std::vector<uint8_t>& item_slots, fb::game::map* map = nullptr, dialog::interaction interaction = dialog::interaction::SLOT) const;
    fb::ostream                 make_dialog_stream(const std::string& message, const std::vector<item::master*>& cores, fb::game::map* map = nullptr, uint16_t pursuit = 0xFFFF, dialog::interaction interaction = dialog::interaction::SELL) const;
    fb::ostream                 make_input_dialog_stream(const std::string& message, fb::game::map* map = nullptr, dialog::interaction interaction = dialog::interaction::INPUT) const;
    fb::ostream                 make_input_dialog_stream(const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false, fb::game::map* map = nullptr, dialog::interaction interaction = dialog::interaction::INPUT_EX) const;
#pragma endregion


#pragma region built-in method
public:
    static int                  builtin_input_dialog(lua_State* lua);
    static int                  builtin_menu_dialog(lua_State* lua);
    static int                  builtin_item_dialog(lua_State* lua);
#pragma endregion
};
#pragma endregion


#pragma region private field
private:
    std::string                 _script;
    listener*                   _listener;
#pragma endregion


#pragma region constructor
public:
    npc(const master* master, listener* listener);
    npc(const npc& right);
    ~npc();
#pragma endregion


#pragma region using parent method
public:
    using object::make_dialog_stream;
#pragma endregion


#pragma region public method
public:
    const std::string&          script() const;
    void                        script(const std::string& value);

public:
    npc*                        make();
#pragma endregion


#pragma region make stream method
public:
    fb::ostream                 make_dialog_stream(const std::string& message, const std::vector<std::string>& menus) const;
    fb::ostream                 make_dialog_stream(const std::string& message, const std::vector<uint8_t>& item_slots) const;
    fb::ostream                 make_dialog_stream(const std::string& message, const std::vector<item::master*>& cores, fb::game::map* map = nullptr) const;
    fb::ostream                 make_input_dialog_stream(const std::string& message, fb::game::map* map = nullptr) const;
    fb::ostream                 make_input_dialog_stream(const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false) const;
#pragma endregion


#pragma region built-in method
public:
    static int                  builtin_input_dialog(lua_State* lua);
    static int                  builtin_menu_dialog(lua_State* lua);
    static int                  builtin_item_dialog(lua_State* lua);
#pragma endregion
};

} }

#endif // !__NPC_H__
