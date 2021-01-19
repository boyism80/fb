#include "model/lua/lua.h"
#include "model/object/object.h"
#include "model/item/item.h"
#include "model/mob/mob.h"
using namespace fb::game::lua;

main*                           main::_instance;

std::string lua_cp949(lua_State* lua, int i)
{
    auto x = lua_tostring(lua, i);
    if(x == nullptr)
        return std::string();

    return CP949(x, PLATFORM::Windows);
}

void lua_push_utf8(lua_State* lua, const std::string& v)
{
    lua_pushstring(lua, UTF8(v, PLATFORM::Windows).c_str());
}

void fb::game::lua::luable::to_lua(lua_State* lua) const
{
    auto allocated = static_cast<void**>(lua_newuserdata(lua, sizeof(void**)));     // [val]
    *allocated = (void*)this;

    auto metaname = this->metaname();
    luaL_getmetatable(lua, metaname.c_str());                                       // [val, mt]
    lua_pushcfunction(lua, luable::builtin_gc);                                     // [val, mt, gc]
    lua_setfield(lua, -2, "__gc");                                                  // [val, mt]
    lua_setmetatable(lua, -2);                                                      // [val]
}

fb::game::lua::luable::luable()
{
}

fb::game::lua::luable::luable(uint32_t id)
{
}

fb::game::lua::luable::~luable()
{
}

int fb::game::lua::luable::builtin_gc(lua_State* lua)
{
    auto allocated = (void**)lua_touserdata(lua, 1);
    *allocated = nullptr;
    return 0;
}

state::state(lua_State* lua) : _lua(lua)
{
}

fb::game::lua::state::state(lua_State* lua, const char* format, ...) : state(lua)
{
    va_list args;
    va_start(args, format);

    char buffer[256];
    vsprintf(buffer, format, args);
    va_end(args);

    luaL_dofile(*this, buffer);
}

state::~state()
{
}

state& fb::game::lua::state::from(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsprintf(buffer, format, args);
    va_end(args);

    luaL_dofile(*this, buffer);
    return *this;
}

state& fb::game::lua::state::func(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsprintf(buffer, format, args);
    va_end(args);

    lua_getglobal(*this, buffer);
    return *this;
}

state& fb::game::lua::state::pushstring(const std::string& value)
{
    lua_push_utf8(this->_lua, value.c_str());
    return *this;
}

state& fb::game::lua::state::pushinteger(int value)
{
    lua_pushinteger(this->_lua, value);
    return *this;
}

state& fb::game::lua::state::pushnil()
{
    lua_pushnil(this->_lua);
    return *this;
}

state& fb::game::lua::state::pushboolean(bool value)
{
    lua_pushboolean(this->_lua, value);
    return *this;
}

state& fb::game::lua::state::pushobject(const luable* object)
{
    object->to_lua(*this);
    return *this;
}

state& fb::game::lua::state::pushobject(const luable& object)
{
    object.to_lua(*this);
    return *this;
}

state::operator lua_State* () const
{
    return this->_lua;
}

thread::thread() : 
    lua::state(lua_newthread(main::get())),
    _ref(luaL_ref(main::get(), LUA_REGISTRYINDEX)),
    _state(LUA_YIELD)
{ 
    lua::main::get().threads.insert(std::make_pair(this->_lua, this));
}

fb::game::lua::thread::thread(const char* format, ...) : thread()
{
    va_list args;
    va_start(args, format);

    char buffer[256];
    vsprintf(buffer, format, args);
    va_end(args);

    luaL_dofile(*this, buffer);
}

thread::~thread()
{
    luaL_unref(main::get(), LUA_REGISTRYINDEX, this->_ref);
}

thread& fb::game::lua::thread::from(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsprintf(buffer, format, args);
    va_end(args);

    luaL_dofile(*this, buffer);
    return *this;
}

thread& fb::game::lua::thread::func(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsprintf(buffer, format, args);
    va_end(args);

    lua_getglobal(*this, buffer);
    return *this;
}

thread& fb::game::lua::thread::pushstring(const std::string& value)
{
    return static_cast<thread&>(state::pushstring(value));
}

thread& fb::game::lua::thread::pushinteger(int value)
{
    return static_cast<thread&>(state::pushinteger(value));
}

thread& fb::game::lua::thread::pushnil()
{
    return static_cast<thread&>(state::pushnil());
}

thread& fb::game::lua::thread::pushboolean(bool value)
{
    return static_cast<thread&>(state::pushboolean(value));
}

thread& fb::game::lua::thread::pushobject(const luable* object)
{
    return static_cast<thread&>(state::pushobject(object));
}

thread& fb::game::lua::thread::pushobject(const luable& object)
{
    return static_cast<thread&>(state::pushobject(object));
}

int fb::game::lua::thread::argc() const
{
    return lua_gettop(this->_lua);
}

int fb::game::lua::thread::resume(int num_args)
{
    if(this->_state == LUA_PENDING)
        return this->_state;

    auto state = lua_resume(*this, nullptr, num_args);
    if(this->_state != LUA_PENDING)
        this->_state = state;

    switch(this->_state)
    {
    case LUA_PENDING:
    case LUA_YIELD:
        break;

    default:
        lua::main::get().threads.erase(this->_lua);
        break;
    }
    
    return this->_state;
}

int fb::game::lua::thread::state() const
{
    return this->_state;
}

bool fb::game::lua::thread::pending() const
{
    return this->_state == LUA_PENDING;
}

void fb::game::lua::thread::pending(bool value)
{
    this->_state = value ? LUA_PENDING : LUA_YIELD;
}

thread* fb::game::lua::thread::get(lua_State& lua_state)
{
    auto& threads = lua::main::get().threads;

    auto found = threads.find(&lua_state);
    if(found == threads.end())
        return nullptr;

    if(found->second != nullptr)
    {
        switch(found->second->_state)
        {
        case LUA_YIELD:
        case LUA_PENDING:
            return found->second;
        }
    }

    threads.erase(&lua_state);
    return nullptr;
}

main::main() : state(luaL_newstate())
{
    luaL_openlibs(*this);

    lua_pushinteger(*this, fb::game::object::types::UNKNOWN);
    lua_setglobal(*this, "UNKNOWN");
    lua_pushinteger(*this, fb::game::object::types::ITEM);
    lua_setglobal(*this, "ITEM");
    lua_pushinteger(*this, fb::game::object::types::NPC);
    lua_setglobal(*this, "NPC");
    lua_pushinteger(*this, fb::game::object::types::MOB);
    lua_setglobal(*this, "MOB");
    lua_pushinteger(*this, fb::game::object::types::SESSION);
    lua_setglobal(*this, "SESSION");
    lua_pushinteger(*this, fb::game::object::types::LIFE);
    lua_setglobal(*this, "LIFE");
    lua_pushinteger(*this, fb::game::object::types::OBJECT);
    lua_setglobal(*this, "OBJECT");


    lua_pushinteger(*this, fb::game::nation::GOGURYEO);
    lua_setglobal(*this, "NATION_GOGURYEO");
    lua_pushinteger(*this, fb::game::nation::BUYEO);
    lua_setglobal(*this, "NATION_BUYEO");


    lua_pushinteger(*this, fb::game::creature::PHOENIX);
    lua_setglobal(*this, "CREATURE_PHOENIX");
    lua_pushinteger(*this, fb::game::creature::TIGER);
    lua_setglobal(*this, "CREATURE_TIGER");
    lua_pushinteger(*this, fb::game::creature::TURTLE);
    lua_setglobal(*this, "CREATURE_TURTLE");
    lua_pushinteger(*this, fb::game::creature::DRAGON);
    lua_setglobal(*this, "CREATURE_DRAGON");


    lua_pushinteger(*this, fb::game::direction::TOP);
    lua_setglobal(*this, "DIRECTION_TOP");
    lua_pushinteger(*this, fb::game::direction::RIGHT);
    lua_setglobal(*this, "DIRECTION_RIGHT");
    lua_pushinteger(*this, fb::game::direction::BOTTOM);
    lua_setglobal(*this, "DIRECTION_BOTTOM");
    lua_pushinteger(*this, fb::game::direction::LEFT);
    lua_setglobal(*this, "DIRECTION_LEFT");


    lua_pushinteger(*this, fb::game::condition::NONE);
    lua_setglobal(*this, "CONDITION_NONE");
    lua_pushinteger(*this, fb::game::condition::MOVE);
    lua_setglobal(*this, "CONDITION_MOVE");
    lua_pushinteger(*this, fb::game::condition::SIGHT);
    lua_setglobal(*this, "CONDITION_SIGHT");
    lua_pushinteger(*this, fb::game::condition::HEAR);
    lua_setglobal(*this, "CONDITION_HEAR");
    lua_pushinteger(*this, fb::game::condition::ORAL);
    lua_setglobal(*this, "CONDITION_ORAL");
    lua_pushinteger(*this, fb::game::condition::MAP);
    lua_setglobal(*this, "CONDITION_MAP");


    lua_pushinteger(*this, fb::game::sex::MAN);
    lua_setglobal(*this, "SEX_MAN");
    lua_pushinteger(*this, fb::game::sex::WOMAN);
    lua_setglobal(*this, "SEX_WOMAN");
    lua_pushinteger(*this, fb::game::sex::BOTH);
    lua_setglobal(*this, "SEX_BOTH");

    lua_pushinteger(*this, fb::game::action::ATTACK);
    lua_setglobal(*this, "ACTION_ATTACK");
    lua_pushinteger(*this, fb::game::action::ARROW);
    lua_setglobal(*this, "ACTION_ARROW");
    lua_pushinteger(*this, fb::game::action::EMOTION);
    lua_setglobal(*this, "ACTION_EMOTION");
    lua_pushinteger(*this, fb::game::action::PICKUP);
    lua_setglobal(*this, "ACTION_PICKUP");
    lua_pushinteger(*this, fb::game::action::PICKUP_SILENT);
    lua_setglobal(*this, "ACTION_PICKUP_SILENT");
    lua_pushinteger(*this, fb::game::action::CAST_SPELL);
    lua_setglobal(*this, "ACTION_CAST_SPELL");
    lua_pushinteger(*this, fb::game::action::EAT);
    lua_setglobal(*this, "ACTION_EAT");


    lua_pushinteger(*this, fb::game::state::NORMAL);
    lua_setglobal(*this, "STATE_NORMAL");
    lua_pushinteger(*this, fb::game::state::GHOST);
    lua_setglobal(*this, "STATE_GHOST");
    lua_pushinteger(*this, fb::game::state::TRANSLUCENCY);
    lua_setglobal(*this, "STATE_TRANSLUCENCY");
    lua_pushinteger(*this, fb::game::state::RIDING);
    lua_setglobal(*this, "STATE_RIDING");
    lua_pushinteger(*this, fb::game::state::DISGUISE);
    lua_setglobal(*this, "STATE_DISGUISE");
    lua_pushinteger(*this, fb::game::state::HALF_CLOACK);
    lua_setglobal(*this, "STATE_HALF_CLOACK");
    lua_pushinteger(*this, fb::game::state::CLOACK);
    lua_setglobal(*this, "STATE_CLOACK");


    lua_pushinteger(*this, fb::game::item::penalties::NONE);
    lua_setglobal(*this, "ITEM_PENALTY_NONE");
    lua_pushinteger(*this, fb::game::item::penalties::DROP);
    lua_setglobal(*this, "ITEM_PENALTY_DROP");
    lua_pushinteger(*this, fb::game::item::penalties::DESTRUCTION);
    lua_setglobal(*this, "ITEM_PENALTY_DESTRUCTION");


    lua_pushinteger(*this, fb::game::item::delete_attr::DELETE_REMOVED);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_REMOVED");
    lua_pushinteger(*this, fb::game::item::delete_attr::DELETE_DROP);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_DROP");
    lua_pushinteger(*this, fb::game::item::delete_attr::DELETE_EAT);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_EAT");
    lua_pushinteger(*this, fb::game::item::delete_attr::DELETE_SMOKE);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_SMOKE");
    lua_pushinteger(*this, fb::game::item::delete_attr::DELETE_THROW);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_THROW");
    lua_pushinteger(*this, fb::game::item::delete_attr::DELETE_SHOOT);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_SHOOT");
    lua_pushinteger(*this, fb::game::item::delete_attr::DELETE_REDUCE);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_REDUCE");
    lua_pushinteger(*this, fb::game::item::delete_attr::DELETE_STICK);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_STICK");
    lua_pushinteger(*this, fb::game::item::delete_attr::DELETE_DECAY);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_DECAY");
    lua_pushinteger(*this, fb::game::item::delete_attr::DELETE_GIVE);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_GIVE");
    lua_pushinteger(*this, fb::game::item::delete_attr::DELETE_SELL);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_SELL");
    lua_pushinteger(*this, fb::game::item::delete_attr::DELETE_NONE);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_NONE");
    lua_pushinteger(*this, fb::game::item::delete_attr::DELETE_DESTROY);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_DESTROY");


    lua_pushinteger(*this, fb::game::item::attrs::ITEM_ATTR_NONE);
    lua_setglobal(*this, "ITEM_ATTR_NONE");
    lua_pushinteger(*this, fb::game::item::attrs::ITEM_ATTR_CONSUME);
    lua_setglobal(*this, "ITEM_ATTR_CONSUME");
    lua_pushinteger(*this, fb::game::item::attrs::ITEM_ATTR_BUNDLE);
    lua_setglobal(*this, "ITEM_ATTR_BUNDLE");
    lua_pushinteger(*this, fb::game::item::attrs::ITEM_ATTR_SCRIPT);
    lua_setglobal(*this, "ITEM_ATTR_SCRIPT");
    lua_pushinteger(*this, fb::game::item::attrs::ITEM_ATTR_CASH);
    lua_setglobal(*this, "ITEM_ATTR_CASH");
    lua_pushinteger(*this, fb::game::item::attrs::ITEM_ATTR_PACK);
    lua_setglobal(*this, "ITEM_ATTR_PACK");
    lua_pushinteger(*this, fb::game::item::attrs::ITEM_ATTR_EQUIPMENT);
    lua_setglobal(*this, "ITEM_ATTR_EQUIPMENT");
    lua_pushinteger(*this, fb::game::item::attrs::ITEM_ATTR_WEAPON);
    lua_setglobal(*this, "ITEM_ATTR_WEAPON");
    lua_pushinteger(*this, fb::game::item::attrs::ITEM_ATTR_ARMOR);
    lua_setglobal(*this, "ITEM_ATTR_ARMOR");
    lua_pushinteger(*this, fb::game::item::attrs::ITEM_ATTR_SHIELD);
    lua_setglobal(*this, "ITEM_ATTR_SHIELD");
    lua_pushinteger(*this, fb::game::item::attrs::ITEM_ATTR_HELMET);
    lua_setglobal(*this, "ITEM_ATTR_HELMET");
    lua_pushinteger(*this, fb::game::item::attrs::ITEM_ATTR_RING);
    lua_setglobal(*this, "ITEM_ATTR_RING");
    lua_pushinteger(*this, fb::game::item::attrs::ITEM_ATTR_AUXILIARY);
    lua_setglobal(*this, "ITEM_ATTR_AUXILIARY");
    lua_pushinteger(*this, fb::game::item::attrs::ITEM_ATTR_ARROW);
    lua_setglobal(*this, "ITEM_ATTR_ARROW");


    lua_pushinteger(*this, fb::game::equipment::EQUIPMENT_LEFT);
    lua_setglobal(*this, "EQUIPMENT_POSITION_LEFT");
    lua_pushinteger(*this, fb::game::equipment::EQUIPMENT_RIGHT);
    lua_setglobal(*this, "EQUIPMENT_POSITION_RIGHT");

    lua_pushinteger(*this, fb::game::equipment::slot::UNKNOWN_SLOT);
    lua_setglobal(*this, "ITEM_SLOT_UNKNOWN");
    lua_pushinteger(*this, fb::game::equipment::slot::WEAPON_SLOT);
    lua_setglobal(*this, "ITEM_SLOT_WEAPON");
    lua_pushinteger(*this, fb::game::equipment::slot::ARMOR_SLOT);
    lua_setglobal(*this, "ITEM_SLOT_ARMOR");
    lua_pushinteger(*this, fb::game::equipment::slot::SHIELD_SLOT);
    lua_setglobal(*this, "ITEM_SLOT_SHIELD");
    lua_pushinteger(*this, fb::game::equipment::slot::HELMET_SLOT);
    lua_setglobal(*this, "ITEM_SLOT_HELMET");
    lua_pushinteger(*this, fb::game::equipment::slot::LEFT_HAND_SLOT);
    lua_setglobal(*this, "ITEM_SLOT_LEFT_HAND");
    lua_pushinteger(*this, fb::game::equipment::slot::RIGHT_HAND_SLOT);
    lua_setglobal(*this, "ITEM_SLOT_RIGHT_HAND");
    lua_pushinteger(*this, fb::game::equipment::slot::LEFT_AUX_SLOT);
    lua_setglobal(*this, "ITEM_SLOT_LEFT_AUX");
    lua_pushinteger(*this, fb::game::equipment::slot::RIGHT_AUX_SLOT);
    lua_setglobal(*this, "ITEM_SLOT_RIGHT_AUX");


    lua_pushinteger(*this, fb::game::weapon::types::NORMAL);
    lua_setglobal(*this, "WEAPON_TYPE_NORMAL");
    lua_pushinteger(*this, fb::game::weapon::types::SPEAR);
    lua_setglobal(*this, "WEAPON_TYPE_SPEAR");
    lua_pushinteger(*this, fb::game::weapon::types::BOW);
    lua_setglobal(*this, "WEAPON_TYPE_BOW");
    lua_pushinteger(*this, fb::game::weapon::types::FAN);
    lua_setglobal(*this, "WEAPON_TYPE_FAN");
    lua_pushinteger(*this, fb::game::weapon::types::UNKNOWN);
    lua_setglobal(*this, "WEAPON_TYPE_UNKNOWN");


    lua_pushinteger(*this, fb::game::mob::offensive_type::CONTAINMENT);
    lua_setglobal(*this, "OFFENSIVE_CONTAINMENT");
    lua_pushinteger(*this, fb::game::mob::offensive_type::COUNTER);
    lua_setglobal(*this, "OFFENSIVE_COUNTER");
    lua_pushinteger(*this, fb::game::mob::offensive_type::NONE);
    lua_setglobal(*this, "OFFENSIVE_NONE");
    lua_pushinteger(*this, fb::game::mob::offensive_type::NON_MOVE);
    lua_setglobal(*this, "OFFENSIVE_NON_MOVE");
    lua_pushinteger(*this, fb::game::mob::offensive_type::RUN_AWAY);
    lua_setglobal(*this, "OFFENSIVE_RUN_AWAY");


    lua_pushinteger(*this, fb::game::mob::sizes::SMALL);
    lua_setglobal(*this, "MOB_SIZE_SMALL");
    lua_pushinteger(*this, fb::game::mob::sizes::LARGE);
    lua_setglobal(*this, "MOB_SIZE_LARGE");

    lua_pushinteger(*this, fb::game::sound::type::SWING);
    lua_setglobal(*this, "SOUND_SWING");
    lua_pushinteger(*this, fb::game::sound::type::EAT);
    lua_setglobal(*this, "SOUND_EAT");
    lua_pushinteger(*this, fb::game::sound::type::EQUIPMENT_OFF);
    lua_setglobal(*this, "SOUND_EQUIPMENT_OFF");
    lua_pushinteger(*this, fb::game::sound::type::EQUIPMENT_ON);
    lua_setglobal(*this, "SOUND_EQUIPMENT_ON");
    lua_pushinteger(*this, fb::game::sound::type::DAMAGE);
    lua_setglobal(*this, "SOUND_DAMAGE");

    lua_pushinteger(*this, 0);
    lua_setglobal(*this, "DIALOG_RESULT_PREV");
    lua_pushinteger(*this, 1);
    lua_setglobal(*this, "DIALOG_RESULT_QUIT");
    lua_pushinteger(*this, 2);
    lua_setglobal(*this, "DIALOG_RESULT_NEXT");
}

main::~main()
{
    for(auto x : threads)
        delete x.second;

    lua_close(*this);
}

main& main::get()
{
    if(main::_instance == nullptr)
        main::_instance = new main();

    return *_instance;
}

void main::release()
{
    if(main::_instance == nullptr)
        return;
    
    delete main::_instance;
    main::_instance = nullptr;
}

void fb::game::lua::release()
{
    main::release();
}

void fb::game::lua::bind_function(const std::string& name, lua_CFunction fn)
{
    lua_register(main::get(), name.c_str(), fn);
}