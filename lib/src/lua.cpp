#include <fb/lua.h>

using namespace fb::lua;

context* fb::lua::get()
{
    auto& main = container::ist().get();
    return main.pop();
}

context* fb::lua::get(lua_State* ctx)
{
    auto& main = container::ist().get();
    return main.get(*ctx);
}

void fb::lua::build(const std::string& name, lua_CFunction fn)
{
    auto& ist = container::ist();
    ist.init_fn([name, fn](main& m) {
        m.build(name, fn);
    });
    // lua_register(main::get(), name.c_str(), fn);
}

void fb::lua::load(const std::string& path)
{
    auto& ist = container::ist();
    ist.load(path);
}

void luable::to_lua(lua_State* ctx) const
{
    if (const auto context = fb::lua::get(ctx); context == nullptr)
        return;

    const auto allocated = static_cast<void**>(lua_newuserdata(ctx, sizeof(void**))); // [val]
    *allocated           = (void*)this;

    {
        auto& metaname = this->metaname();
        luaL_getmetatable(ctx, metaname.c_str());   // [val, mt]
        lua_pushcfunction(ctx, luable::builtin_gc); // [val, mt, gc]
        lua_setfield(ctx, -2, "__gc");              // [val, mt]
        lua_setmetatable(ctx, -2);                  // [val]
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
    if (const auto allocated = static_cast<void**>(lua_touserdata(ctx, 1)); allocated != nullptr)
        *allocated = nullptr;
    return 0;
}

context::context(lua_State* ctx) :
    _ctx(ctx),
    owner(nullptr)
{ }

context::context(lua_State* ctx, context& owner) :
    _ctx(ctx),
    owner(&owner)
{ }

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

std::string context::tostring(int offset)
{
    auto x = lua_tostring(*this, offset);
    if (x == nullptr)
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

    auto main = static_cast<fb::lua::main*>(this->owner);
    if (main != &lua::container::ist().get())
    {
        // thread mismatch
        main->revoke(*this);
        return false;
    }

    if (this->_state == LUA_PENDING)
        return *this;

    auto state = lua_resume(*this, nullptr, argc);

    if (this->_state != LUA_PENDING)
        this->_state = state;

    switch (this->_state)
    {
    case LUA_PENDING:
    case LUA_YIELD:
        return true;

    case LUA_ERRRUN:
    case LUA_ERRERR:
        fb::logger::fatal("lua error message : {}", this->tostring(-1).c_str());
        lua_pop(*this, 1);
        main->revoke(*this);
        return false;

    default:
        if (auto_release)
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
    auto main = static_cast<fb::lua::main*>(this->owner);
    switch (this->_state)
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

main::main() :
    context(::luaL_newstate())
{
    luaL_openlibs(*this);
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
    if (found == this->busy.end())
        return nullptr;

    return found->second.get();
}

bool main::load_file(const std::string& path)
{
    if (path.empty())
        return true;

    if (_bytecodes.contains(path))
        return true;

    luaL_loadfile(*this, path.c_str());
    void*      params[] = {this, (void*)path.c_str()};
    const auto callback = [](lua_State* ctx, const void* bytes, size_t size, void* params) {
        auto casted = (void**)(params);
        auto ist    = (main*)casted[0];
        auto path   = (const char*)casted[1];
        if (ist->_bytecodes.contains(path) == false)
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
    if (this->idle.empty() == false)
    {
        auto& ctx = this->idle.begin()->second;
        auto  key = (lua_State*)*ctx;
        this->busy.insert(std::make_pair(key, std::move(ctx)));
        this->idle.erase(key);
        return this->busy[key].get();
    }
    else if (this->idle.size() + this->busy.size() < DEFAULT_POOL_SIZE)
    {
        auto ptr = std::make_unique<thread>(*this);
        auto key = (lua_State*)*ptr.get();

        if (this->idle.contains(key) || this->busy.contains(key))
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

    if (this->idle.contains(ctx))
        return ctx;

    if (ctx.state() != LUA_OK)
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
    if (i == this->busy.end())
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

fb::lua::container::container()
{
    this->init_fn([this](main& m) {
        for (auto& path : this->_scripts)
            m.load_file(path);
    });
}

fb::lua::container::~container()
{ }

main& fb::lua::container::get()
{
    std::lock_guard gd(this->_mutex);

    auto id = (uint32_t)std::hash<std::thread::id>{}(std::this_thread::get_id());
    if (this->_mains.contains(id) == false)
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

void fb::lua::container::init_fn(init_func&& fn)
{
    this->_init_funcs.push_back(fn);
}

void fb::lua::container::load(const std::string& path)
{
    std::lock_guard gd(this->_mutex);

    if (path.empty())
        return;

    this->_scripts.push_back(path);
}

fb::lua::container& fb::lua::container::ist()
{
    static std::once_flag             _flag;
    static std::unique_ptr<container> _ist;

    std::call_once(_flag, [] {
        _ist = std::unique_ptr<container>(new container());
    });
    return *_ist;
}