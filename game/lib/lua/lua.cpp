#include <fb/game/lua.h>
#include <fb/game/object.h>
#include <fb/game/item.h>
#include <fb/game/mob.h>
using namespace fb::game::lua;

std::mutex                      main::mutex;

lua* fb::game::lua::get()
{
    return fb::game::lua::main::get().pop();
}

lua* fb::game::lua::get(lua_State* lua)
{
    return fb::game::lua::main::get().get(*lua);
}

void fb::game::lua::luable::to_lua(lua_State* lua) const
{
    auto allocated = static_cast<void**>(lua_newuserdata(lua, sizeof(void**)));     // [val]
    *allocated = (void*)this;

    auto& metaname = this->metaname();
    luaL_getmetatable(lua, metaname.c_str());                                       // [val, mt]
    lua_pushcfunction(lua, luable::builtin_gc);                                     // [val, mt, gc]
    lua_setfield(lua, -2, "__gc");                                                  // [val, mt]
    lua_setmetatable(lua, -2);                                                      // [val]
}

fb::game::lua::luable::luable()
{ }

fb::game::lua::luable::luable(uint32_t id)
{ }

fb::game::lua::luable::~luable()
{ }

int fb::game::lua::luable::builtin_gc(lua_State* lua)
{
    auto allocated = (void**)lua_touserdata(lua, 1);
    if(allocated != nullptr)
        *allocated = nullptr;
    return 0;
}

lua::lua(lua_State* lua) : _lua(lua)
{ }

lua::~lua()
{
    lua_close(*this);
}

lua& fb::game::lua::lua::from(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsprintf(buffer, format, args);
    va_end(args);

    auto& ist = main::get();
    auto  i = ist.bytecodes.find(buffer);
    if(i == ist.bytecodes.end())
        return *this;

    auto& bytes = i->second;
    if(luaL_loadbuffer(*this, bytes.data(), bytes.size(), 0))
        return *this;

    if(lua_pcall(*this, 0, LUA_MULTRET, 0))
        return *this;

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
    lua_pushstring(*this, UTF8(value, PLATFORM::Windows).c_str());
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

const std::string fb::game::lua::lua::tostring(int offset)
{
    auto x = lua_tostring(*this, offset);
    if(x == nullptr)
        return std::string();

    return CP949(x, PLATFORM::Windows); 
}

lua::operator lua_State* () const
{
    return this->_lua;
}

int fb::game::lua::lua::argc() const
{
    return lua_gettop(this->_lua);
}

lua& fb::game::lua::lua::resume(int argc)
{
    if(this->_state == LUA_PENDING)
        return *this;

    auto state = lua_resume(*this, nullptr, argc);

    if(this->_state != LUA_PENDING)
        this->_state = state;

    switch(this->_state)
    {
    case LUA_PENDING:
    case LUA_YIELD:
        return *this;
        break;

    default:
        return main::get().release(*this);
        break;
    }
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

fb::game::lua::main::main() : lua(luaL_newstate())
{
    luaL_openlibs(*this);

    this->load_file("scripts/common/door.lua");
    this->load_file("scripts/common/pickup.lua");
    this->load_file("scripts/common/attack.lua");

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


    lua_pushinteger(*this, fb::game::item::delete_attr::REMOVED);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_REMOVED");
    lua_pushinteger(*this, fb::game::item::delete_attr::DROP);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_DROP");
    lua_pushinteger(*this, fb::game::item::delete_attr::EAT);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_EAT");
    lua_pushinteger(*this, fb::game::item::delete_attr::SMOKE);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_SMOKE");
    lua_pushinteger(*this, fb::game::item::delete_attr::THROW);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_THROW");
    lua_pushinteger(*this, fb::game::item::delete_attr::SHOOT);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_SHOOT");
    lua_pushinteger(*this, fb::game::item::delete_attr::REDUCE);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_REDUCE");
    lua_pushinteger(*this, fb::game::item::delete_attr::STICK);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_STICK");
    lua_pushinteger(*this, fb::game::item::delete_attr::DECAY);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_DECAY");
    lua_pushinteger(*this, fb::game::item::delete_attr::GIVE);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_GIVE");
    lua_pushinteger(*this, fb::game::item::delete_attr::SELL);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_SELL");
    lua_pushinteger(*this, fb::game::item::delete_attr::NONE);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_NONE");
    lua_pushinteger(*this, fb::game::item::delete_attr::DESTROY);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_DESTROY");


    lua_pushinteger(*this, fb::game::item::attrs::NONE);
    lua_setglobal(*this, "NONE");
    lua_pushinteger(*this, fb::game::item::attrs::CONSUME);
    lua_setglobal(*this, "CONSUME");
    lua_pushinteger(*this, fb::game::item::attrs::BUNDLE);
    lua_setglobal(*this, "BUNDLE");
    lua_pushinteger(*this, fb::game::item::attrs::SCRIPT);
    lua_setglobal(*this, "SCRIPT");
    lua_pushinteger(*this, fb::game::item::attrs::CASH);
    lua_setglobal(*this, "CASH");
    lua_pushinteger(*this, fb::game::item::attrs::PACK);
    lua_setglobal(*this, "PACK");
    lua_pushinteger(*this, fb::game::item::attrs::EQUIPMENT);
    lua_setglobal(*this, "EQUIPMENT");
    lua_pushinteger(*this, fb::game::item::attrs::WEAPON);
    lua_setglobal(*this, "WEAPON");
    lua_pushinteger(*this, fb::game::item::attrs::ARMOR);
    lua_setglobal(*this, "ARMOR");
    lua_pushinteger(*this, fb::game::item::attrs::SHIELD);
    lua_setglobal(*this, "SHIELD");
    lua_pushinteger(*this, fb::game::item::attrs::HELMET);
    lua_setglobal(*this, "HELMET");
    lua_pushinteger(*this, fb::game::item::attrs::RING);
    lua_setglobal(*this, "RING");
    lua_pushinteger(*this, fb::game::item::attrs::AUXILIARY);
    lua_setglobal(*this, "AUXILIARY");
    lua_pushinteger(*this, fb::game::item::attrs::ARROW);
    lua_setglobal(*this, "ARROW");


    lua_pushinteger(*this, fb::game::equipment::position::LEFT);
    lua_setglobal(*this, "POSITION_LEFT");
    lua_pushinteger(*this, fb::game::equipment::position::RIGHT);
    lua_setglobal(*this, "POSITION_RIGHT");

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

    ::lua_checkstack(*this, DEFAULT_POOL_SIZE);
}

lua* fb::game::lua::main::get(lua_State& lua)
{
    std::lock_guard gd(main::mutex);

    auto found = this->busy.find(&lua);
    if(found == this->busy.end())
        return nullptr;

    return found->second.get();
}

std::unique_ptr<lua> fb::game::lua::main::new_thread()
{
    auto ptr = (fb::game::lua::lua*)new fb::game::lua::thread(*this);
    return std::unique_ptr<fb::game::lua::lua>(ptr);
}

bool fb::game::lua::main::load_file(const std::string& path)
{
    if(path.empty())
        return true;

    std::lock_guard gd(main::mutex);

    auto i = bytecodes.find(path);
    if(i != bytecodes.end())
        return true;

    luaL_loadfile(*this, path.c_str());
    void* params[] = {this, (void*)path.c_str()};
    const auto callback = [](lua_State* lua, const void* bytes, size_t size, void* params)
    {
        auto casted = (void**)(params);
        auto ist = (main*)casted[0];
        auto path = (const char*)casted[1];
        auto  i = ist->bytecodes.find(path);
        if (i == ist->bytecodes.end())
            ist->bytecodes[path] = std::vector<char>();

        for (int i = 0; i < size; i++)
            ist->bytecodes[path].push_back(static_cast<const char*>(bytes)[i]);

        return 0;
    };

    ::lua_dump(*this, callback, params, 1);
    return true;
}

lua* fb::game::lua::main::pop()
{
    std::lock_guard gd(main::mutex);

    if(this->idle.empty() == false)
    {
        auto& lua = this->idle.begin()->second;
        auto  key = (lua_State*)*lua;
        this->busy.insert(std::make_pair(key, std::move(lua)));
        this->idle.erase(key);
        return this->busy[key].get();
    }
    else if(this->idle.size() + this->busy.size() < DEFAULT_POOL_SIZE)
    {
        auto ptr = new_thread();
        auto key = (lua_State*)*ptr.get();
        this->busy.insert(std::make_pair(key, std::move(ptr)));
        return this->busy[key].get();
    }
    else
    {
        return nullptr;
    }
}

lua& fb::game::lua::main::release(lua& lua)
{
    std::lock_guard gd(main::mutex);

    if(this->busy.find(lua) == this->busy.end())
        return lua;

    if(this->idle.find((lua_State*)lua) != this->idle.end())
        return lua;

    auto key = (lua_State*)lua;
    this->idle.insert(std::make_pair(key, std::move(this->busy[key])));
    this->busy.erase(key);

    return *this->idle[key];
}

main& main::get()
{
    static std::once_flag           flag;
    static std::unique_ptr<main>    ist;

    std::call_once(flag, [] () { ist = std::unique_ptr<main>(new main()); });
    return *ist;
}

void fb::game::lua::bind_function(const std::string& name, lua_CFunction fn)
{
    auto& main = fb::game::lua::main::get();
    main.bind_function(name, fn);
    //lua_register(main::get(), name.c_str(), fn);
}

fb::game::lua::thread::thread(lua_State* lua) : lua(lua_newthread(lua))
{ }