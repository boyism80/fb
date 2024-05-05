#include <fb/game/lua.h>
#include <fb/game/object.h>
#include <fb/game/item.h>
#include <fb/game/mob.h>
using namespace fb::game::lua;

context* fb::game::lua::get()
{
    auto& main = container::ist().get();
    return main.pop();
}

context* fb::game::lua::get(lua_State* ctx)
{
    auto& main = container::ist().get();
    return main.get(*ctx);
}

void fb::game::lua::bind_function(const std::string& name, lua_CFunction fn)
{
    auto& ist = container::ist();
    ist.init_fn([name, fn] (main& m)
    {
        m.bind_function(name, fn);
    });
    //lua_register(main::get(), name.c_str(), fn);
}

void fb::game::lua::load(const std::string& path)
{
    auto& ist = container::ist();
    ist.load(path);
}

void luable::to_lua(lua_State* ctx) const
{
    auto context = fb::game::lua::get(ctx);
    if (context == nullptr)
        return;

    auto allocated = static_cast<void**>(lua_newuserdata(ctx, sizeof(void**)));     // [val]
    *allocated = (void*)this;

    {
        auto& metaname = this->metaname();
        luaL_getmetatable(ctx, metaname.c_str());                                   // [val, mt]
        lua_pushcfunction(ctx, luable::builtin_gc);                                 // [val, mt, gc]
        lua_setfield(ctx, -2, "__gc");                                              // [val, mt]
        lua_setmetatable(ctx, -2);                                                  // [val]
    }
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

context::context(lua_State* ctx) : _ctx(ctx), owner(nullptr)
{ }

context::context(lua_State* ctx, context& owner) : _ctx(ctx), owner(&owner)
{ }

context::~context()
{ }

context& context::from(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    auto fname = fb::format(fmt, &args);
    va_end(args);

#if defined DEBUG || defined _DEBUG
    luaL_dofile(*this, fname.c_str());
#else
    auto main = static_cast<fb::game::lua::main*>(this->owner);
    if(main->_bytecodes.contains(fname) == false)
    {
        fb::logger::fatal("cannot find script %s", fname.c_str());
        return *this;
    }

    auto& bytes = main->_bytecodes[fname];
    if(luaL_loadbuffer(*this, bytes.data(), bytes.size(), 0))
        return *this;

    if(lua_pcall(*this, 0, LUA_MULTRET, 0))
        return *this;
#endif

    return *this;
}

context& context::func(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    auto fname = fb::format(fmt, &args);
    va_end(args);

    lua_getglobal(*this, fname.c_str());
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

context& context::pushnumber(lua_Number value)
{
    lua_pushnumber(this->_ctx, value);
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

context& context::pop(int offset)
{
    lua_pop(this->_ctx, offset);
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

int context::argc()
{
    return lua_gettop(this->_ctx);
}

bool context::resume(int argc, bool auto_release)
{
    if (this->owner == nullptr)
        throw std::runtime_error("this context is not lua thread");

    auto main = static_cast<fb::game::lua::main*>(this->owner);
    if (main != &lua::container::ist().get())
    {
        // thread mismatch
        main->revoke(*this);
        return false;
    }

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
        fb::logger::fatal("lua error message : %s", this->tostring(-1).c_str());
        lua_pop(*this, 1);
        main->revoke(*this);
        return false;

    default:
        if(auto_release)
            main->release(*this);
        return true;
    }
}

int context::state() const
{
    return this->_state;
}

void context::release()
{
    auto main = static_cast<fb::game::lua::main*>(this->owner);
    switch(this->_state)
    {
    case LUA_OK:
        main->release(*this);
        break;

    default:
        main->revoke(*this);
        break;
    }
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

    this->load_file("scripts/script.lua");
    this->load_file("scripts/common/npc.lua");
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


    lua_pushinteger(*this, fb::game::NATION_TYPE::GOGURYEO);
    lua_setglobal(*this, "NATION_GOGURYEO");
    lua_pushinteger(*this, fb::game::NATION_TYPE::BUYEO);
    lua_setglobal(*this, "NATION_BUYEO");


    lua_pushinteger(*this, fb::game::CREATURE_TYPE::PHOENIX);
    lua_setglobal(*this, "CREATURE_PHOENIX");
    lua_pushinteger(*this, fb::game::CREATURE_TYPE::TIGER);
    lua_setglobal(*this, "CREATURE_TIGER");
    lua_pushinteger(*this, fb::game::CREATURE_TYPE::TURTLE);
    lua_setglobal(*this, "CREATURE_TURTLE");
    lua_pushinteger(*this, fb::game::CREATURE_TYPE::DRAGON);
    lua_setglobal(*this, "CREATURE_DRAGON");


    lua_pushinteger(*this, fb::game::DIRECTION_TYPE::TOP);
    lua_setglobal(*this, "DIRECTION_TOP");
    lua_pushinteger(*this, fb::game::DIRECTION_TYPE::RIGHT);
    lua_setglobal(*this, "DIRECTION_RIGHT");
    lua_pushinteger(*this, fb::game::DIRECTION_TYPE::BOTTOM);
    lua_setglobal(*this, "DIRECTION_BOTTOM");
    lua_pushinteger(*this, fb::game::DIRECTION_TYPE::LEFT);
    lua_setglobal(*this, "DIRECTION_LEFT");


    lua_pushinteger(*this, fb::game::CONDITION_TYPE::NONE);
    lua_setglobal(*this, "CONDITION_NONE");
    lua_pushinteger(*this, fb::game::CONDITION_TYPE::MOVE);
    lua_setglobal(*this, "CONDITION_MOVE");
    lua_pushinteger(*this, fb::game::CONDITION_TYPE::SIGHT);
    lua_setglobal(*this, "CONDITION_SIGHT");
    lua_pushinteger(*this, fb::game::CONDITION_TYPE::HEAR);
    lua_setglobal(*this, "CONDITION_HEAR");
    lua_pushinteger(*this, fb::game::CONDITION_TYPE::ORAL);
    lua_setglobal(*this, "CONDITION_ORAL");
    lua_pushinteger(*this, fb::game::CONDITION_TYPE::MAP);
    lua_setglobal(*this, "CONDITION_MAP");


    lua_pushinteger(*this, fb::game::SEX_TYPE::MAN);
    lua_setglobal(*this, "SEX_MAN");
    lua_pushinteger(*this, fb::game::SEX_TYPE::WOMAN);
    lua_setglobal(*this, "SEX_WOMAN");
    lua_pushinteger(*this, fb::game::SEX_TYPE::BOTH);
    lua_setglobal(*this, "SEX_BOTH");

    lua_pushinteger(*this, fb::game::ACTION_TYPE::ATTACK);
    lua_setglobal(*this, "ACTION_ATTACK");
    lua_pushinteger(*this, fb::game::ACTION_TYPE::ARROW);
    lua_setglobal(*this, "ACTION_ARROW");
    lua_pushinteger(*this, fb::game::ACTION_TYPE::EMOTION);
    lua_setglobal(*this, "ACTION_EMOTION");
    lua_pushinteger(*this, fb::game::ACTION_TYPE::PICKUP);
    lua_setglobal(*this, "ACTION_PICKUP");
    lua_pushinteger(*this, fb::game::ACTION_TYPE::PICKUP_SILENT);
    lua_setglobal(*this, "ACTION_PICKUP_SILENT");
    lua_pushinteger(*this, fb::game::ACTION_TYPE::CAST_SPELL);
    lua_setglobal(*this, "ACTION_CAST_SPELL");
    lua_pushinteger(*this, fb::game::ACTION_TYPE::EAT);
    lua_setglobal(*this, "ACTION_EAT");


    lua_pushinteger(*this, fb::game::STATE_TYPE::NORMAL);
    lua_setglobal(*this, "STATE_NORMAL");
    lua_pushinteger(*this, fb::game::STATE_TYPE::GHOST);
    lua_setglobal(*this, "STATE_GHOST");
    lua_pushinteger(*this, fb::game::STATE_TYPE::TRANSLUCENCY);
    lua_setglobal(*this, "STATE_TRANSLUCENCY");
    lua_pushinteger(*this, fb::game::STATE_TYPE::RIDING);
    lua_setglobal(*this, "STATE_RIDING");
    lua_pushinteger(*this, fb::game::STATE_TYPE::DISGUISE);
    lua_setglobal(*this, "STATE_DISGUISE");
    lua_pushinteger(*this, fb::game::STATE_TYPE::HALF_CLOACK);
    lua_setglobal(*this, "STATE_HALF_CLOACK");
    lua_pushinteger(*this, fb::game::STATE_TYPE::CLOACK);
    lua_setglobal(*this, "STATE_CLOACK");


    lua_pushinteger(*this, fb::game::item::DEATH_PENALTY_TYPE::NONE);
    lua_setglobal(*this, "ITEM_PENALTY_NONE");
    lua_pushinteger(*this, fb::game::item::DEATH_PENALTY_TYPE::DROP);
    lua_setglobal(*this, "ITEM_PENALTY_DROP");
    lua_pushinteger(*this, fb::game::item::DEATH_PENALTY_TYPE::DESTRUCTION);
    lua_setglobal(*this, "ITEM_PENALTY_DESTRUCTION");


    lua_pushinteger(*this, fb::game::item::DELETE_TYPE::REMOVED);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_REMOVED");
    lua_pushinteger(*this, fb::game::item::DELETE_TYPE::DROP);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_DROP");
    lua_pushinteger(*this, fb::game::item::DELETE_TYPE::EAT);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_EAT");
    lua_pushinteger(*this, fb::game::item::DELETE_TYPE::SMOKE);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_SMOKE");
    lua_pushinteger(*this, fb::game::item::DELETE_TYPE::THROW);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_THROW");
    lua_pushinteger(*this, fb::game::item::DELETE_TYPE::SHOOT);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_SHOOT");
    lua_pushinteger(*this, fb::game::item::DELETE_TYPE::REDUCE);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_REDUCE");
    lua_pushinteger(*this, fb::game::item::DELETE_TYPE::STICK);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_STICK");
    lua_pushinteger(*this, fb::game::item::DELETE_TYPE::DECAY);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_DECAY");
    lua_pushinteger(*this, fb::game::item::DELETE_TYPE::GIVE);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_GIVE");
    lua_pushinteger(*this, fb::game::item::DELETE_TYPE::SELL);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_SELL");
    lua_pushinteger(*this, fb::game::item::DELETE_TYPE::NONE);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_NONE");
    lua_pushinteger(*this, fb::game::item::DELETE_TYPE::DESTROY);
    lua_setglobal(*this, "ITEM_DELETE_ATTR_DESTROY");


    lua_pushinteger(*this, fb::game::item::ATTRIBUTE::NONE);
    lua_setglobal(*this, "ITEM_ATTR_NONE");
    lua_pushinteger(*this, fb::game::item::ATTRIBUTE::CONSUME);
    lua_setglobal(*this, "ITEM_ATTR_CONSUME");
    lua_pushinteger(*this, fb::game::item::ATTRIBUTE::BUNDLE);
    lua_setglobal(*this, "ITEM_ATTR_BUNDLE");
    lua_pushinteger(*this, fb::game::item::ATTRIBUTE::SCRIPT);
    lua_setglobal(*this, "ITEM_ATTR_SCRIPT");
    lua_pushinteger(*this, fb::game::item::ATTRIBUTE::CASH);
    lua_setglobal(*this, "ITEM_ATTR_CASH");
    lua_pushinteger(*this, fb::game::item::ATTRIBUTE::PACK);
    lua_setglobal(*this, "ITEM_ATTR_PACK");
    lua_pushinteger(*this, fb::game::item::ATTRIBUTE::EQUIPMENT);
    lua_setglobal(*this, "ITEM_ATTR_EQUIPMENT");
    lua_pushinteger(*this, fb::game::item::ATTRIBUTE::WEAPON);
    lua_setglobal(*this, "ITEM_ATTR_WEAPON");
    lua_pushinteger(*this, fb::game::item::ATTRIBUTE::ARMOR);
    lua_setglobal(*this, "ITEM_ATTR_ARMOR");
    lua_pushinteger(*this, fb::game::item::ATTRIBUTE::SHIELD);
    lua_setglobal(*this, "ITEM_ATTR_SHIELD");
    lua_pushinteger(*this, fb::game::item::ATTRIBUTE::HELMET);
    lua_setglobal(*this, "ITEM_ATTR_HELMET");
    lua_pushinteger(*this, fb::game::item::ATTRIBUTE::RING);
    lua_setglobal(*this, "ITEM_ATTR_RING");
    lua_pushinteger(*this, fb::game::item::ATTRIBUTE::AUXILIARY);
    lua_setglobal(*this, "ITEM_ATTR_AUXILIARY");
    lua_pushinteger(*this, fb::game::item::ATTRIBUTE::ARROW);
    lua_setglobal(*this, "ITEM_ATTR_ARROW");


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

    lua_pushinteger(*this, fb::game::SOUND_TYPE::SWING);
    lua_setglobal(*this, "SOUND_SWING");
    lua_pushinteger(*this, fb::game::SOUND_TYPE::EAT);
    lua_setglobal(*this, "SOUND_EAT");
    lua_pushinteger(*this, fb::game::SOUND_TYPE::EQUIPMENT_OFF);
    lua_setglobal(*this, "SOUND_EQUIPMENT_OFF");
    lua_pushinteger(*this, fb::game::SOUND_TYPE::EQUIPMENT_ON);
    lua_setglobal(*this, "SOUND_EQUIPMENT_ON");
    lua_pushinteger(*this, fb::game::SOUND_TYPE::DAMAGE);
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
    this->idle.clear();
    this->busy.clear();
    lua_close(*this);
}

context* main::get(lua_State& ctx)
{
    auto found = this->busy.find(&ctx);
    if(found == this->busy.end())
        return nullptr;

    return found->second.get();
}

bool main::load_file(const std::string& path)
{
    if(path.empty())
        return true;

    if(_bytecodes.contains(path))
        return true;

    luaL_loadfile(*this, path.c_str());
    void* params[] = {this, (void*)path.c_str()};
    const auto callback = [](lua_State* ctx, const void* bytes, size_t size, void* params)
    {
        auto casted = (void**)(params);
        auto ist = (main*)casted[0];
        auto path = (const char*)casted[1];
        if(ist->_bytecodes.contains(path) == false)
            ist->_bytecodes[path] = std::vector<char>();

        for (int i = 0; i < size; i++)
            ist->_bytecodes[path].push_back(static_cast<const char*>(bytes)[i]);

        return 0;
    };

    ::lua_dump(*this, callback, params, 1);
    return true;
}

context* main::pop()
{
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
        auto ptr = std::make_unique<thread>(*this);
        auto key = (lua_State*)*ptr.get();

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
    if (this->busy.contains(ctx) == false)
        return ctx;

    if(this->idle.contains(ctx))
        return ctx;

    if(ctx.state() != LUA_OK)
        throw std::runtime_error("lua ctx's current state is not LUA_OK");

    lua_pop(ctx, -1);

    auto key = (lua_State*)ctx;
    this->idle.insert(std::make_pair(key, std::move(this->busy[key])));
    this->busy.erase(key);

    return *this->idle[key];
}

void main::revoke(context& ctx)
{
    auto i = this->busy.find(ctx);
    if(i == this->busy.end())
        return;
    
    this->busy.erase(i);
}

thread::thread(context& owner) : 
    context(::lua_newthread(owner), owner),
    ref(luaL_ref(owner, LUA_REGISTRYINDEX))
{ }

thread::thread(thread&& ctx) : 
    context(ctx._ctx),
    ref(ctx.ref)
{ }

thread::~thread()
{
    luaL_unref(this->_ctx, LUA_REGISTRYINDEX, this->ref);
}

container::container()
{
    this->init_fn([this] (main& m)
    {
        for(auto& path : this->_scripts)
            m.load_file(path);
    });
}

container::~container()
{ }

main& container::get()
{
    std::lock_guard gd(this->_mutex);

    auto id = (uint32_t)std::hash<std::thread::id>{}(std::this_thread::get_id());
    if(this->_mains.contains(id) == false)
    {
        auto ptr = std::unique_ptr<main>(new main());
        for (auto& fn : this->_init_funcs)
        {
            fn(*ptr);
        }
        this->_mains.insert({id, std::move(ptr)});
    }

    return *this->_mains[id];
}

void container::init_fn(init_func&& fn)
{
    this->_init_funcs.push_back(fn);
}

void container::load(const std::string& path)
{
    std::lock_guard gd(this->_mutex);
    
    if(path.empty())
        return;

    this->_scripts.push_back(path);
}

container& container::ist()
{
    static std::once_flag               _flag;
    static std::unique_ptr<container>   _ist;

    std::call_once(_flag, [] { _ist = std::unique_ptr<container>(new container()); });
    return *_ist;
}