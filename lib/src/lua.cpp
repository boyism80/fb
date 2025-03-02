#include <fb/lua.h>
#include <fb/thread_container.h>

using namespace fb::lua;

context* fb::lua::new_context()
{
    auto& ist = context_pool::ist();
    return ist.pop();
}

context* fb::lua::get(lua_State* ctx)
{
    auto& ist = context_pool::ist();
    return ist.get(ctx);
}

async::task<void> fb::lua::build(const std::string& name, lua_CFunction fn)
{
    static auto& ist = context_pool::ist();
    auto         n   = std::string{name};
    for (auto& [_, root] : ist)
    {
        co_await root->thread.switching();
        root->build(n, fn);
    }
}

async::task<void> fb::lua::dump(const std::string& path)
{
    static auto& ist = context_pool::ist();
    auto         p   = std::string{path};
    for (auto& [_, root] : ist)
    {
        co_await root->thread.switching();
        root->dump(p);
    }
}

void luable::to_lua(lua_State* ctx) const
{
    if (const auto context = fb::lua::get(ctx); context == nullptr)
        return;

    const auto allocated = static_cast<void**>(lua_newuserdata(ctx, sizeof(void**))); // [val]
    *allocated           = (void*)this;

    auto& metaname = this->metaname();
    luaL_getmetatable(ctx, metaname.c_str());   // [val, mt]
    lua_pushcfunction(ctx, luable::builtin_gc); // [val, mt, gc]
    lua_setfield(ctx, -2, "__gc");              // [val, mt]
    lua_setmetatable(ctx, -2);                  // [val]
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

std::string context::get_type(int offset)
{
    return lua_typename(*this, lua_type(*this, offset));
}

std::string context::tostring(int offset)
{
    if (this->argc() < offset)
        return "";

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

    auto root = static_cast<fb::lua::root*>(this->owner);
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
        root->revoke(*this);
        return false;

    default:
        if (auto_release)
            root->release(*this);
        return true;
    }
}

int context::state() const
{
    return this->_state;
}

void context::release()
{
    auto root = static_cast<fb::lua::root*>(this->owner);
    switch (this->_state)
    {
    case LUA_OK:
        root->release(*this);
        break;

    default:
        root->revoke(*this);
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

root::root(fb::thread& thread) :
    context(::luaL_newstate()),
    thread(thread)
{
    luaL_openlibs(*this);
}

root::~root()
{
    auto _ = std::lock_guard(this->_mutex);

    this->idle.clear();
    this->busy.clear();
    lua_close(*this);
}

context* root::get(lua_State* ctx)
{
    auto _ = std::lock_guard(this->_mutex);

    auto found = this->busy.find(ctx);
    if (found == this->busy.end())
        return nullptr;

    return found->second.get();
}

bool root::dump(const std::string& path)
{
    if (path.empty())
        return true;

    if (_bytecodes.contains(path))
        return true;

    luaL_loadfile(*this, path.c_str());
    void*      params[] = {this, (void*)path.c_str()};
    const auto callback = [](lua_State* ctx, const void* bytes, size_t size, void* params) {
        auto casted = (void**)(params);
        auto ist    = (root*)casted[0];
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

context* root::pop()
{
    auto _ = std::lock_guard(this->_mutex);

    if (this->idle.empty() == false)
    {
        auto& ctx = this->idle.begin()->second;
        auto  key = (lua_State*)*ctx;
        this->busy.insert({key, std::move(ctx)});
        this->idle.erase(key);

        return this->busy[key].get();
    }
    else if (this->idle.size() + this->busy.size() < DEFAULT_POOL_SIZE)
    {
        auto ptr = std::make_unique<fb::lua::thread>(*this);
        auto key = (lua_State*)*ptr.get();

        if (this->idle.contains(key) || this->busy.contains(key))
            return nullptr;

        for (auto& [name, _] : this->_bytecodes)
            ptr->load(name);

        context_pool::ist().record(*ptr);
        this->busy.insert({key, std::move(ptr)});
        return this->busy[key].get();
    }
    else
    {
        return nullptr;
    }
}

void root::release(context& ctx)
{
    auto _ = std::lock_guard(this->_mutex);

    if (this->busy.contains(ctx) == false)
        return;

    if (this->idle.contains(ctx))
        return;

    if (ctx.state() != LUA_OK)
        throw std::runtime_error("lua ctx's current state is not LUA_OK");

    lua_pop(ctx, -1);

    auto key = (lua_State*)ctx;
    this->idle.insert({key, std::move(this->busy[key])});
    this->busy.erase(key);
}

void root::revoke(context& ctx)
{
    auto _ = std::lock_guard(this->_mutex);

    auto i = this->busy.find(ctx);
    if (i == this->busy.end())
        return;

    this->busy.erase(i);
    context_pool::ist().unrecord(ctx);
}

fb::lua::context_pool::~context_pool()
{
    for (auto& [_, root] : this->_roots)
    {
        delete root;
    }
}

context* fb::lua::context_pool::pop()
{
    auto id = std::this_thread::get_id();
    if (this->_roots.contains(id) == false)
        return nullptr;

    return this->_roots[id]->pop();
}

context* fb::lua::context_pool::get(lua_State* ctx)
{
    auto _ = std::shared_lock<std::shared_mutex>(this->_mapping_lock);

    if (this->_mapping.contains(ctx) == false)
        return nullptr;

    auto& id = this->_mapping[ctx];
    if (this->_roots.contains(id) == false)
        return nullptr;

    return this->_roots[id]->get(ctx);
}

void fb::lua::context_pool::setup(fb::thread_container& threads)
{
    if (this->_threads != nullptr)
        return;

    this->_threads = &threads;
    for (int i = 0; i < threads.size(); i++)
    {
        auto thread = threads.at(i);
        this->_roots.insert({thread->id(), new root(*thread)});
    }

    // for (auto& [tid, root] : this->_roots)
    //{
    //     auto thread = threads.at(tid);
    //     co_await thread->switching();

    //    for (auto& fn : this->_setup_funcs)
    //    {
    //        fn(*root);
    //    }
    //}
}

context_pool::base_type::iterator fb::lua::context_pool::begin()
{
    return this->_roots.begin();
}

context_pool::base_type::iterator fb::lua::context_pool::end()
{
    return this->_roots.end();
}

fb::lua::context_pool& fb::lua::context_pool::ist()
{
    static std::once_flag                _flag;
    static std::unique_ptr<context_pool> _ist;

    std::call_once(_flag, [] {
        _ist = std::unique_ptr<context_pool>(new context_pool());
    });
    return *_ist;
}

void fb::lua::context_pool::record(lua_State* L)
{
    auto _ = std::lock_guard<std::shared_mutex>(this->_mapping_lock);
    this->_mapping.insert({L, std::this_thread::get_id()});
}

void fb::lua::context_pool::unrecord(lua_State* L)
{
    auto _ = std::lock_guard<std::shared_mutex>(this->_mapping_lock);
    this->_mapping.erase(L);
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