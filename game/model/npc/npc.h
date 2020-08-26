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


#pragma region public method
public:
    object::types               type() const { return object::types::NPC; }
    object*                     make(listener* listener) const;
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

#pragma region public method
public:
    const std::string&          script() const;
    void                        script(const std::string& value);

public:
    npc*                        make();
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
