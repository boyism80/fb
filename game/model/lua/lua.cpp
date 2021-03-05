#include "model/lua/lua.h"
#include "model/object/object.h"
#include "model/item/item.h"
#include "model/mob/mob.h"
using namespace fb::game::lua;

main*                           main::_instance;
std::mutex                      lua::_mutex;

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

lua& fb::game::lua::get()
{
    auto& main = fb::game::lua::main::get();
    return main.alloc();
}

lua* fb::game::lua::get(lua_State* lua)
{
    auto& main = fb::game::lua::main::get();
    return main.get(*lua);
}

void fb::game::lua::reserve(int capacity)
{
    auto& main = fb::game::lua::main::get();
    main.reserve(capacity);
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

lua::lua(lua_State* lua) : _lua(lua)
{
    luaL_openlibs(*this);

    bind_builtin_functions();

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

lua::~lua()
{
    lua_close(*this);
}

void fb::game::lua::lua::bind_builtin_functions()
{
    auto& main = fb::game::lua::main::get();

    for(auto name : main.inheritances)
    {
        luaL_newmetatable(*this, name.c_str());
        if(main.relations.find(name) != main.relations.end())
        {
            auto base_name = main.relations[name].c_str();
            luaL_getmetatable(*this, base_name);
            lua_setmetatable(*this, -2);
        }
        lua_pushvalue(*this, -1);
        lua_setfield(*this, -2, "__index");
        luaL_setfuncs(*this, main.builtin_local_funcs[name], 0);
    }

    for(auto pair : main.builtin_global_funcs)
    {
        auto name = pair.first.c_str();
        auto func = pair.second;
        
        lua_register(*this, name, func);
    }

    for(auto pair : main.environments)
    {
        auto name = pair.first.c_str();
        auto data = pair.second;

        lua_pushstring(*this, name);
        lua_pushlightuserdata(*this, data);
        lua_settable(*this, LUA_REGISTRYINDEX);
    }
}

lua& fb::game::lua::lua::from(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsprintf(buffer, format, args);
    va_end(args);

    luaL_dofile(*this, buffer);
    return *this;
}

lua& fb::game::lua::lua::func(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsprintf(buffer, format, args);
    va_end(args);

    lua_getglobal(*this, buffer);
    return *this;
}

lua& fb::game::lua::lua::pushstring(const std::string& value)
{
    lua_push_utf8(this->_lua, value.c_str());
    return *this;
}

lua& fb::game::lua::lua::pushinteger(lua_Integer value)
{
    lua_pushinteger(this->_lua, value);
    return *this;
}

lua& fb::game::lua::lua::pushnil()
{
    lua_pushnil(this->_lua);
    return *this;
}

lua& fb::game::lua::lua::pushboolean(bool value)
{
    lua_pushboolean(this->_lua, value);
    return *this;
}

lua& fb::game::lua::lua::pushobject(const luable* object)
{
    object->to_lua(*this);
    return *this;
}

lua& fb::game::lua::lua::pushobject(const luable& object)
{
    object.to_lua(*this);
    return *this;
}

lua::operator lua_State* () const
{
    return this->_lua;
}

int fb::game::lua::lua::argc() const
{
    return lua_gettop(this->_lua);
}

int fb::game::lua::lua::resume(int argc)
{
    if(this->_state == LUA_PENDING)
        return this->_state;

    auto state = lua_resume(*this, nullptr, argc);;

    if(this->_state != LUA_PENDING)
        this->_state = state;

    switch(this->_state)
    {
    case LUA_PENDING:
    case LUA_YIELD:
        break;

    default:
        main::get().release(*this);
        break;
    }
    
    return this->_state;
}

int fb::game::lua::lua::state() const
{
    return this->_state;
}

void fb::game::lua::lua::release()
{
    auto& main = fb::game::lua::main::get();
    main.release(*this);
}

bool fb::game::lua::lua::pending() const
{
    return this->_state == LUA_PENDING;
}

void fb::game::lua::lua::pending(bool value)
{
    this->_state = value ? LUA_PENDING : LUA_YIELD;
}

lua* fb::game::lua::main::get(lua_State& lua)
{
    std::lock_guard gd(lua::_mutex);

    auto found = this->busy.find(&lua);
    if(found == this->busy.end())
        return nullptr;

    return found->second;
}

void fb::game::lua::main::reserve(int capacity)
{
    std::lock_guard gd(lua::_mutex);

    this->clear();
    for(int i = 0; i < capacity; i++)
    {
        auto created = new lua(luaL_newstate());
        this->idle.insert(std::make_pair((lua_State*)*created, created));
    }
}

lua& fb::game::lua::main::alloc()
{
    std::lock_guard gd(lua::_mutex);

    if(this->idle.empty())
    {
        auto created = new lua(luaL_newstate());
        this->busy.insert(std::make_pair((lua_State*)*created, created));
        return *created;
    }
    else
    {
        auto lua = this->idle.begin()->second;
        this->idle.erase((lua_State*)*lua);
        this->busy.insert(std::make_pair((lua_State*)*lua, lua));
        return *lua;
    }
}

void fb::game::lua::main::release(lua& lua)
{
    std::lock_guard gd(lua::_mutex);

    if(this->idle.find((lua_State*)lua) != this->idle.end())
        return;

    this->idle.insert(std::make_pair((lua_State*)lua, &lua));
    this->busy.erase((lua_State*)lua);
}

main::main()
{
}

main::~main()
{
    this->clear();
}

void fb::game::lua::main::update_inheritances()
{
    std::map<std::string, int> references;
    for(auto pair : this->relations)
    {
        auto derived = pair.first;
        auto based = pair.second;

        if(references.find(derived) == references.end())
            references.insert(std::make_pair(derived, 0));

        if(references.find(based) == references.end())
            references.insert(std::make_pair(based, 1));
        else
            references[based]++;
    }

    auto sorted = std::vector<std::pair<std::string, int>>(references.begin(), references.end());
    std::sort
    (
        sorted.begin(), 
        sorted.end(), 
        [] (const std::pair<std::string, int>& pair1, const std::pair<std::string, int>& pair2)
        {
            return pair1.second > pair2.second;
        }
    );

    this->inheritances.clear();
    for(auto pair : sorted)
        this->inheritances.push_back(pair.first);
}

void fb::game::lua::main::clear()
{
    for(auto x : idle)
        delete x.second;

    for(auto x : busy)
        delete x.second;
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
    auto& main = fb::game::lua::main::get();
    main.bind_function(name, fn);
    //lua_register(main::get(), name.c_str(), fn);
}