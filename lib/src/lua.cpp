#include <fb/abstract.h>
#include <fb/lua.h>
#include <fb/thread_container.h>
#include <async/awaitable_then.h>

using namespace fb::lua;

context* fb::lua::new_context(context* parent)
{
    auto& ist = context_pool::ist();
    return ist.pop(parent);
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
        co_await root->switching();
        root->build(n, fn);
    }
}

async::task<void> fb::lua::dump(const std::string& path)
{
    static auto& ist = context_pool::ist();
    auto         p   = std::string{path};
    for (auto& [_, root] : ist)
    {
        co_await root->switching();
        root->dump(p);
    }
}

luable::luable()
{ }

luable::luable(uint32_t id)
{ }

luable::~luable()
{ }

context::context(lua_State* ctx, fb::thread& initial_thread) :
    _ctx(ctx),
    _initial_thread(initial_thread)
{ }

context::context(lua_State* ctx, context& owner, context* parent) :
    _ctx(ctx),
    _initial_thread(owner._initial_thread),
    _parent(parent),
    owner(&owner)
{ }

context& context::pushstring(const std::string& value)
{
    lua_pushstring(*this, UTF8(value, PLATFORM::WINDOWS).c_str());
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

std::string context::metatable(int offset)
{
    lua_getmetatable(*this, offset);
    lua_getfield(*this, -1, "__name");
    auto name = this->tostring(-1);
    lua_pop(*this, 2);

    return name;
}

std::string fb::lua::context::basetable(const std::string& metaname)
{
    luaL_getmetatable(*this, metaname.c_str());
    lua_getfield(*this, -1, "__parent");

    if (this->is_table(-1) == false)
    {
        lua_pop(*this, 2);
        return std::string{};
    }

    lua_getfield(*this, -1, "__name");
    auto name = this->tostring(-1);
    lua_pop(*this, 3);
    return name;
}

std::string context::tostring(int offset, const std::string& default_value)
{
    if (this->argc() < offset)
        return default_value;

    if (lua_type(*this, offset) != LUA_TSTRING)
        return default_value;

    auto x = lua_tostring(*this, offset);
    if (x == nullptr)
        return std::string();

    return CP949(x, PLATFORM::WINDOWS);
}

void fb::lua::context::parent(context* parent)
{
    this->_parent = parent;
}

context* fb::lua::context::parent() const
{
    return this->_parent;
}

context::operator lua_State* () const
{
    return this->_ctx;
}

int context::argc()
{
    return lua_gettop(this->_ctx);
}

async::task<bool> context::call(int argc, bool auto_release, int* n)
{
    auto promise        = std::make_shared<async::task_completion_source<bool>>();
    this->_promise      = promise;
    this->_auto_release = auto_release;
    this->resume(argc, n);
    return promise->task();
}

void fb::lua::context::resume(int argc, int* n)
{
    if (this->_state == LUA_YIELD)
        this->_state = LUA_OK;

    if (this->_promise == nullptr)
        return;

    if (this->owner == nullptr)
    {
        this->_promise->set_exception(std::make_exception_ptr(std::runtime_error("this context is not lua thread")));
        return;
    }

    auto root = static_cast<fb::lua::root*>(this->owner);
    if (this->_state == LUA_PENDING)
        return;

    auto state = lua_resume(*this, nullptr, argc);

    if (this->_state != LUA_PENDING)
        this->_state = state;

    switch (this->_state)
    {
    case LUA_PENDING:
    case LUA_YIELD:
        break;

    case LUA_ERRRUN:
    case LUA_ERRERR:
    {
        lua_pop(*this, 1);
        auto message = std::format("lua error message : {}", this->tostring(-1).c_str());
        fb::logger::fatal(message);
        auto promise = promise_type{this->_promise};
        auto parent  = this->_parent;
        root->revoke(*this);

        if (parent != nullptr && parent->_state == LUA_YIELD)
        {
            async::awaitable_then(parent->_initial_thread.switching(), [=](auto result) {
                parent->resume(0);
            });
        }
        promise->set_exception(std::make_exception_ptr(std::runtime_error(message)));
    }
    break;

    default:
    {
        auto parent = this->_parent;
        if (parent != nullptr)
        {
            async::awaitable_then(parent->_initial_thread.switching(), [=, this](auto result) {
                auto argc = this->argc();
                if (n != nullptr)
                    *n = argc;

                lua_xmove(*this, *parent, argc);
                if (parent->_state == LUA_YIELD)
                    parent->resume(argc);
            });
        }
        if (this->_auto_release)
            root->release(*this);

        this->_promise->set_value(true);
    }
    break;
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

int context::ensure_yield(fb::context&                         ctx,
                          std::weak_ptr<fb::thread_switchable> weak,
                          std::function<int(bool)>             fn,
                          bool                                 no_yield)
{
    auto shared = weak.lock();
    if (shared == nullptr)
        return 0;

    if (this->_initial_thread.id() == shared->thread()->id())
    {
        return fn(false);
    }
    else
    {
        async::awaitable_then(ctx.threads.switching(weak), [this, fn](auto result) {
            try
            {
                result();
                return fn(true);
            }
            catch (std::exception& e)
            {
                fb::logger::fatal("lua error message : {}", e.what());
                this->release();
                return 0;
            }
        });
        if (no_yield)
            return 0;
        else
            return this->yield(0);
    }
}

int fb::lua::context::ensure_resume(fb::context&                         ctx,
                                    std::weak_ptr<fb::thread_switchable> weak,
                                    std::function<int()>                 fn,
                                    bool                                 force_resume)
{
    auto shared = weak.lock();
    if (shared == nullptr)
        return 0;

    if (this->_initial_thread.id() == std::this_thread::get_id())
    {
        auto n = fn();
        if (force_resume)
        {
            this->resume(n);
            return n;
        }
        else
        {
            return n;
        }
    }
    else
    {
        async::awaitable_then(this->_initial_thread.switching(), [this, fn, &ctx, weak](auto result) {
            try
            {
                result();

                auto shared = weak.lock();
                if (shared == nullptr)
                    throw std::runtime_error("object not alive");

                auto n = fn();
                this->resume(n);
            }
            catch (std::exception& e)
            {
                fb::logger::fatal("lua error message : {}", e.what());
                this->release();
            }
        });
        return 0;
    }
}

root::root(fb::thread& thread) :
    context(::luaL_newstate(), thread)
{
    luaL_openlibs(*this);
}

root::~root()
{
    this->idle.clear();
    this->busy.clear();
    lua_close(*this);
}

context* root::get(lua_State* ctx)
{
    auto it = this->busy.find(ctx);
    if (it == this->busy.end())
        return nullptr;

    return it->second.get();
}

bool root::dump(const std::string& path)
{
    if (path.empty())
        return true;

    // Atomic check-and-load: prevent race conditions
    if (this->_bytecodes.contains(path))
        return true;

    // Load file outside the callback to avoid nested locking issues
    if (luaL_loadfile(*this, path.c_str()) != LUA_OK)
    {
        auto error = lua_tostring(*this, -1);
        context::pop(1); // pop error message
        throw std::runtime_error(error);
    }

    // Prepare bytecode container
    this->_bytecodes[path] = std::vector<char>();

    // Dump bytecode directly into the container
    void*      params[] = {&this->_bytecodes[path]};
    const auto callback = [](lua_State* ctx, const void* bytes, size_t size, void* p) {
        auto params    = static_cast<void**>(p);
        auto bytecodes = static_cast<std::vector<char>*>(params[0]);

        for (size_t i = 0; i < size; i++)
            bytecodes->push_back(static_cast<const char*>(bytes)[i]);

        return 0;
    };

    ::lua_dump(*this, callback, params, 1);
    return true;
}

context* root::pop(context* parent)
{
    if (this->idle.empty() == false)
    {
        auto& ctx = this->idle.begin()->second;
        auto  key = (lua_State*)*ctx;
        this->busy.insert({key, std::move(ctx)});
        this->idle.erase(key);
        this->busy[key]->parent(parent);
        return this->busy[key].get();
    }
    else if (this->idle.size() + this->busy.size() < DEFAULT_POOL_SIZE)
    {
        auto ptr = std::make_unique<fb::lua::thread>(*this, parent);
        auto key = (lua_State*)*ptr.get();

        if (this->idle.contains(key) || this->busy.contains(key))
            return nullptr;

        for (auto& [name, _] : this->_bytecodes)
            ptr->load(name);

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
    auto internal_func = [this](context& ctx) {
        if (this->busy.contains(ctx) == false)
            return;

        if (this->idle.contains(ctx))
            return;

        if (ctx.state() != LUA_OK)
            throw std::runtime_error("lua ctx's current state is not LUA_OK");

        lua_settop(ctx, 0);
        ctx.parent(nullptr);

        auto key = (lua_State*)ctx;
        this->idle.insert({key, std::move(this->busy[key])});
        this->busy.erase(key);
    };

    if (this->_initial_thread.id() != std::this_thread::get_id())
    {
        std::ignore = this->_initial_thread.dispatch([=, &ctx](auto&) -> async::task<void> {
            internal_func(ctx);
            co_return;
        });
    }
    else
    {
        internal_func(ctx);
    }
}

void root::revoke(context& ctx)
{
    auto it = this->busy.find(ctx);
    if (it == this->busy.end())
        return;

    this->busy.erase(it);
}

async::task<void> fb::lua::context::switching()
{
    co_await this->_initial_thread.switching();
}

fb::thread& fb::lua::root::initial_thread()
{
    return this->_initial_thread;
}

fb::lua::context_pool::~context_pool()
{
    for (auto& [_, root] : this->_roots)
    {
        delete root;
    }
}

context* fb::lua::context_pool::pop(context* parent)
{
    auto id = std::this_thread::get_id();
    if (this->_roots.contains(id) == false)
        return nullptr;

    return this->_roots[id]->pop(parent);
}

context* fb::lua::context_pool::get(lua_State* ctx)
{
    auto id = std::this_thread::get_id();
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

thread::thread(context& owner, context* parent) :
    context(::lua_newthread(owner), owner, parent),
    ref(luaL_ref(owner, LUA_REGISTRYINDEX))
{ }

thread::thread(thread&& ctx) :
    context(ctx._ctx, *ctx.owner, ctx.parent()),
    ref(ctx.ref)
{ }

thread::~thread()
{
    luaL_unref(this->_ctx, LUA_REGISTRYINDEX, this->ref);
}