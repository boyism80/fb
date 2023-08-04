#ifndef __NPC_H__
#define __NPC_H__

#include <fb/game/object.h>

namespace fb { namespace game {

class npc : public object
{
public:
    LUA_PROTOTYPE

public:
    class master;

public:
    npc(fb::game::context& context, const master* master);
    npc(const npc& right);
    ~npc();

public:
    npc*                        make(fb::game::context& context);

public:
    static int                  builtin_input_dialog(lua_State* lua);
    static int                  builtin_menu_dialog(lua_State* lua);
    static int                  builtin_item_dialog(lua_State* lua);
};

class npc::master : public object::master
{
public:
    LUA_PROTOTYPE

public:
    interface listener;

    struct config : public fb::game::object::master::config
    {
    public:
        std::string     script;
    };

public:
    std::string                 script;

public:
    friend class npc;

public:
    master(const fb::game::npc::master::config& config);
    ~master();

public:
    object::types               type() const { return object::types::NPC; }

public:
    static int                  builtin_input_dialog(lua_State* lua);
    static int                  builtin_menu_dialog(lua_State* lua);
    static int                  builtin_item_dialog(lua_State* lua);
};


interface npc::master::listener : public virtual fb::game::object::listener
{ };

} }

#endif // !__NPC_H__
