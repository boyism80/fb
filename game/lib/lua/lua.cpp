#include <fb/game/lua.h>
#include <fb/game/object.h>
#include <fb/game/item.h>
#include <fb/game/mob.h>
using namespace fb::game::lua;

std::mutex                      main::mutex;

context* fb::game::lua::get()
{
    return main::get().pop();
}

context* fb::game::lua::get(lua_State* ctx)
{
    return main::get().get(*ctx);
}

void fb::game::lua::bind_function(const std::string& name, lua_CFunction fn)
{
    auto& main = main::get();
    main.bind_function(name, fn);
    //lua_register(main::get(), name.c_str(), fn);
}

void luable::to_lua(lua_State* ctx) const
{
    auto allocated = static_cast<void**>(lua_newuserdata(ctx, sizeof(void**)));     // [val]
    *allocated = (void*)this;

    auto& metaname = this->metaname();
    luaL_getmetatable(ctx, metaname.c_str());                                       // [val, mt]
    lua_pushcfunction(ctx, luable::builtin_gc);                                     // [val, mt, gc]
    lua_setfield(ctx, -2, "__gc");                                                  // [val, mt]
    lua_setmetatable(ctx, -2);                                                      // [val]
}

luable::luable()
{ }

luable::luable(uint32_t id)
{ }

luable::~luable()
{ }

int luable::builtin_gc(lua_State* ctx)
{
    auto allocated = (void**)lua_touserdata(ctx, 1);
    if(allocated != nullptr)
        *allocated = nullptr;
    return 0;
}

context::context(lua_State* ctx) : _ctx(ctx)
{ }

context::~context()
{ }

context& context::from(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsprintf(buffer, format, args);
    va_end(args);

    auto& ist = main::get();
    if(ist.bytecodes.contains(buffer) == false)
        return *this;

    auto& bytes = ist.bytecodes[buffer];
    if(luaL_loadbuffer(*this, bytes.data(), bytes.size(), 0))
        return *this;

    if(lua_pcall(*this, 0, LUA_MULTRET, 0))
        return *this;

    return *this;
}

context& context::func(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsprintf(buffer, format, args);
    va_end(args);

    lua_getglobal(*this, buffer);
    return *this;
}

context& context::pushstring(const std::string& value)
{
    lua_pushstring(*this, UTF8(value, PLATFORM::Windows).c_str());
    return *this;
}

context& context::pushinteger(lua_Integer value)
{
    lua_pushinteger(this->_ctx, value);
    return *this;
}

context& context::pushnil()
{
    lua_pushnil(this->_ctx);
    return *this;
}

context& context::pushboolean(bool value)
{
    lua_pushboolean(this->_ctx, value);
    return *this;
}

context& context::pushobject(const luable* object)
{
    object->to_lua(*this);
    return *this;
}

context& context::pushobject(const luable& object)
{
    object.to_lua(*this);
    return *this;
}

context& context::push(const void* value)
{
    lua_pushlightuserdata(this->_ctx, const_cast<void*>(value));
    return *this;
}

const std::string context::tostring(int offset)
{
    auto x = lua_tostring(*this, offset);
    if(x == nullptr)
        return std::string();

    return CP949(x, PLATFORM::Windows); 
}

context::operator lua_State* () const
{
    return this->_ctx;
}

int context::argc() const
{
    return lua_gettop(this->_ctx);
}

bool context::resume(int argc)
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
        return true;

    case LUA_ERRRUN:
    case LUA_ERRERR:
        main::get().revoke(*this);
        return false;

    default:
        main::get().release(*this);
        return true;
    }
}

int context::state() const
{
    return this->_state;
}

void context::release()
{
    auto& main = main::get();
    main.release(*this);
}

bool context::pending() const
{
    return this->_state == LUA_PENDING;
}

void context::pending(bool value)
{
    this->_state = value ? LUA_PENDING : LUA_YIELD;
}

main::main() : context(::luaL_newstate())
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
}

main::~main()
{
    lua_close(*this);
}

context* main::get(lua_State& ctx)
{
    std::lock_guard gd(main::mutex);

    auto found = this->busy.find(&ctx);
    if(found == this->busy.end())
        return nullptr;

    return found->second.get();
}

bool main::load_file(const std::string& path)
{
    if(path.empty())
        return true;

    std::lock_guard gd(main::mutex);

    if(bytecodes.contains(path))
        return true;

    luaL_loadfile(*this, path.c_str());
    void* params[] = {this, (void*)path.c_str()};
    const auto callback = [](lua_State* ctx, const void* bytes, size_t size, void* params)
    {
        auto casted = (void**)(params);
        auto ist = (main*)casted[0];
        auto path = (const char*)casted[1];
        if(ist->bytecodes.contains(path) == false)
            ist->bytecodes[path] = std::vector<char>();

        for (int i = 0; i < size; i++)
            ist->bytecodes[path].push_back(static_cast<const char*>(bytes)[i]);

        return 0;
    };

    ::lua_dump(*this, callback, params, 1);
    return true;
}

context* main::pop()
{
    std::lock_guard gd(main::mutex);

    if(this->idle.empty() == false)
    {
        auto& ctx = this->idle.begin()->second;
        auto  key = (lua_State*)*ctx;
        this->busy.insert(std::make_pair(key, std::move(ctx)));
        this->idle.erase(key);
        return this->busy[key].get();
    }
    else if(this->idle.size() + this->busy.size() < DEFAULT_POOL_SIZE)
    {
        auto ptr = std::make_unique<thread>(this->_ctx);
        auto key = (lua_State*)*ptr.get();

        // lua_newthread가 이미 존재하는 lua_State*를 다시
        // 반환하는 경우가 있는데.. 확인해볼 필요가 있을듯
        if(this->idle.contains(key) || this->busy.contains(key))
            return nullptr;

        this->busy.insert(std::make_pair(key, std::move(ptr)));
        return this->busy[key].get();
    }
    else
    {
        return nullptr;
    }
}

context& main::release(context& ctx)
{
    std::lock_guard gd(main::mutex);

    if(this->busy.contains(ctx) == false)
        return ctx;

    if(this->idle.contains(ctx))
        return ctx;

    auto key = (lua_State*)ctx;
    this->idle.insert(std::make_pair(key, std::move(this->busy[key])));
    this->busy.erase(key);

    return *this->idle[key];
}

void main::revoke(context& ctx)
{
    std::lock_guard gd(main::mutex);

    auto i = this->busy.find(ctx);
    if(i == this->busy.end())
        return;
    
    this->busy.erase(i);
}

main& main::get()
{
    static std::once_flag           flag;
    static std::unique_ptr<main>    ist;

    std::call_once(flag, [] () { ist = std::unique_ptr<main>(new main()); });
    return *ist;
}

thread::thread(lua_State* ctx) : 
    context(::lua_newthread(ctx)),
    ref(luaL_ref(ctx, LUA_REGISTRYINDEX))
{ }

thread::thread(thread&& ctx) : 
    context(ctx._ctx),
    ref(ctx.ref)
{ }

thread::~thread()
{
    luaL_unref(this->_ctx, LUA_REGISTRYINDEX, this->ref);
}

//int thread::builtin_gc(lua_State* ctx)
//{
//    puts("delete lua thread");
//    return 0;
//}