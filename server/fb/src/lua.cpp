#include <fb/async_executor.h>
#include <fb/execution_context.h>
#include <fb/lua.h>
#include <fb/thread_container.h>
#include <async/awaitable_then.h>
#include <async/propagation.h>
#include <string>
#include <string_view>
#include <vector>

using namespace fb::lua;

context* fb::lua::get(lua_State* ctx)
{
    if (ctx == nullptr)
        return nullptr;

    ::lua_getfield(ctx, LUA_REGISTRYINDEX, root::REGISTRY_KEY);
    auto root_ptr = static_cast<fb::lua::root*>(::lua_touserdata(ctx, -1));
    ::lua_pop(ctx, 1);

    if (root_ptr == nullptr)
        return nullptr;

    return root_ptr->get(ctx);
}

luable::luable()
{ }

luable::luable(uint32_t id)
{ }

luable::~luable()
{ }

context::context(fb::async_executor& executor, lua_State* ctx, fb::thread& initial_thread) :
    _ctx(ctx),
    executor(executor),
    _initial_thread(initial_thread)
{ }

context::context(fb::async_executor& executor, lua_State* ctx, context& owner, context* parent) :
    _ctx(ctx),
    executor(executor),
    _initial_thread(owner._initial_thread),
    _parent(parent),
    owner(&owner)
{ }

context& context::pushstring(std::string_view value)
{
    lua_pushstring(*this, UTF8(std::string(value), PLATFORM::WINDOWS).c_str());
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

context& context::pushjson(const Json::Value& json)
{
    if (json.isNull())
    {
        this->pushnil();
    }
    else if (json.isBool())
    {
        this->pushboolean(json.asBool());
    }
    else if (json.isInt() || json.isUInt() || json.isInt64() || json.isUInt64())
    {
        this->pushinteger(json.asInt64());
    }
    else if (json.isDouble() || json.isNumeric())
    {
        this->pushnumber(json.asDouble());
    }
    else if (json.isString())
    {
        this->pushstring(json.asString());
    }
    else if (json.isArray())
    {
        this->new_table();
        auto i = 0;
        for (const auto& item : json)
        {
            this->pushinteger(i + 1);
            this->pushjson(item);
            lua_settable(*this, -3);
            i++;
        }
    }
    else if (json.isObject())
    {
        this->new_table();
        for (auto it = json.begin(); it != json.end(); ++it)
        {
            this->pushstring(it.key().asString());
            this->pushjson(*it);
            lua_settable(*this, -3);
        }
    }
    else
    {
        this->pushnil();
    }
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

std::string fb::lua::context::basetable(std::string_view metaname)
{
    luaL_getmetatable(*this, std::string(metaname).c_str());
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

std::string context::tostring(int offset, std::string_view default_value)
{
    if (this->argc() < offset)
        return std::string(default_value);

    if (lua_type(*this, offset) != LUA_TSTRING)
        return std::string(default_value);

    auto x = lua_tostring(*this, offset);
    if (x == nullptr)
        return std::string();

    return CP949(x, PLATFORM::WINDOWS);
}

int context::tointeger(int offset, int default_value)
{
    if (this->argc() < offset)
        return default_value;
    else if (lua_type(*this, offset) != LUA_TNUMBER)
        return default_value;
    else
        return (int)lua_tointeger(*this, offset);
}
lua_Integer context::tonumber(int offset, lua_Integer default_value)
{
    if (this->argc() < offset)
        return default_value;
    else if (lua_type(*this, offset) != LUA_TNUMBER)
        return default_value;
    else
        return lua_tonumber(*this, offset);
}

bool context::toboolean(int offset, bool default_value)
{
    if (this->argc() < offset)
        return default_value;
    else if (lua_type(*this, offset) != LUA_TBOOLEAN)
        return default_value;
    else
        return lua_toboolean(*this, offset);
}

bool context::is_string(int offset)
{
    return lua_isstring(*this, offset);
}

bool context::is_obj(int offset)
{
    return lua_isuserdata(*this, offset);
}

bool context::is_function(int offset)
{
    if (this->argc() < offset)
        return false;
    else if (lua_type(*this, offset) != LUA_TFUNCTION)
        return false;
    else
        return true;
}

bool context::is_table(int offset)
{
    return lua_istable(*this, offset);
}

bool context::is_number(int offset)
{
    return lua_isnumber(*this, offset);
}

bool context::is_nil(int offset)
{
    return lua_isnil(*this, offset);
}

int context::rawgeti(int offset_t, int offset_e)
{
    return lua_rawgeti(*this, offset_t, offset_e);
}

int context::rawget(int table_index)
{
    return lua_rawget(*this, table_index);
}

void context::rawseti(int offset_t, int offset_e)
{
    lua_rawseti(*this, offset_t, offset_e);
}

void context::settable(int table_index)
{
    lua_settable(*this, table_index);
}

int context::rawlen(int offset_t)
{
    return (int)lua_rawlen(*this, offset_t);
}

void context::remove(int offset)
{
    lua_remove(*this, offset);
}
void context::new_table()
{
    lua_newtable(*this);
}

bool context::next(int offset)
{
    return lua_next(*this, offset) != 0;
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

async::task<bool> context::call(int argc, int* retc)
{
    this->_call_engaged = true;

    auto promise   = std::make_shared<async::task_completion_source<bool>>();
    this->_promise = promise;
    this->resume(argc, retc);
    return promise->task();
}

void fb::lua::context::options(call_options opts)
{
    this->_options = opts;
}

const call_options& fb::lua::context::options() const
{
    return this->_options;
}

bool fb::lua::context::call_engaged() const
{
    return this->_call_engaged;
}

void fb::lua::context::clear_call_engaged()
{
    this->_call_engaged = false;
}

void fb::lua::context::resume(int argc, int* n)
{
    if (this->_promise == nullptr)
        return;

    if (this->owner == nullptr)
    {
        this->_promise->set_exception(std::make_exception_ptr(std::runtime_error("this context is not lua thread")));
        return;
    }

    auto root  = static_cast<fb::lua::root*>(this->owner);
    auto state = lua_resume(*this, nullptr, argc);
    if (state == LUA_YIELD)
        return;

    if (state != LUA_OK)
    {
        // Any non-LUA_OK and non-LUA_YIELD means Lua errored.
        const char* raw = lua_tostring(*this, -1);
        auto        message =
            std::format("lua error message : {}", raw != nullptr ? std::string_view{raw} : std::string_view{});
        lua_pop(*this, 1);
        fb::logger::fatal(message);

        auto promise            = promise_type{this->_promise};
        auto parent             = this->_parent;
        auto auto_resume_parent = this->_options.auto_resume_parent;
        root->revoke(*this);

        if (parent != nullptr && lua_status(*parent) == LUA_YIELD && auto_resume_parent)
        {
            auto context = fb::execution_context::token();
            async::awaitable_then(parent->_initial_thread.switching(),
                                  [parent, context](async::awaitable_result<void> result) {
                                      try
                                      {
                                          result();
                                          fb::execution_context::pending(context);
                                          parent->resume(0);
                                      }
                                      catch (std::exception& e)
                                      {
                                          fb::logger::fatal("lua co_builder async completion error: {}", e.what());
                                      }
                                      catch (...)
                                      {
                                          fb::logger::fatal("lua co_builder async completion error: non-std exception");
                                      }
                                  });
        }
        promise->set_exception(std::make_exception_ptr(std::runtime_error(message)));
    }
    else // LUA_OK: coroutine finished successfully.
    {
        auto parent             = this->_parent;
        auto auto_resume_parent = this->_options.auto_resume_parent;
        if (parent != nullptr)
        {
            auto context = fb::execution_context::token();
            async::awaitable_then(parent->_initial_thread.switching(),
                                  [this, parent, n, context, auto_resume_parent](async::awaitable_result<void> result) {
                                      try
                                      {
                                          result();
                                          fb::execution_context::pending(context);

                                          auto argc = this->argc();
                                          if (n != nullptr)
                                              *n = argc;

                                          if (auto_resume_parent)
                                          {
                                              lua_xmove(*this, *parent, argc);
                                              if (lua_status(*parent) == LUA_YIELD)
                                                  parent->resume(argc);
                                          }
                                      }
                                      catch (std::exception& e)
                                      {
                                          fb::logger::fatal("lua co_builder async completion error: {}", e.what());
                                      }
                                      catch (...)
                                      {
                                          fb::logger::fatal("lua co_builder async completion error: non-std exception");
                                      }
                                  });
        }
        else if (n != nullptr)
            *n = this->argc();

        if (this->_options.auto_release)
            root->release(*this);

        this->_promise->set_value(true);
    }
}

int context::yield(int retc)
{
    return lua_yield(*this, retc);
}

void context::clear_loaded_modules()
{
    lua_getglobal(*this, "package");
    if (lua_istable(*this, -1) == false)
    {
        lua_pop(*this, 1);
        return;
    }

    lua_getfield(*this, -1, "loaded");
    if (lua_istable(*this, -1) == false)
    {
        lua_pop(*this, 2);
        return;
    }

    std::vector<std::string> keys;
    lua_pushnil(*this);
    while (lua_next(*this, -2) != 0)
    {
        if (lua_type(*this, -2) == LUA_TSTRING)
        {
            auto key = lua_tostring(*this, -2);
            if (key != nullptr && std::string_view(key).starts_with("lib."))
                keys.emplace_back(key);
        }
        lua_pop(*this, 1);
    }

    for (const auto& key : keys)
    {
        lua_pushnil(*this);
        lua_setfield(*this, -2, key.c_str());
    }

    lua_pop(*this, 2);
}

void context::release()
{
    auto root = static_cast<fb::lua::root*>(this->owner);

    auto status = lua_status(*this);
    switch (status)
    {
    case LUA_OK:
        root->release(*this);
        break;

    default:
        root->revoke(*this);
        break;
    }
}

async::task<std::optional<int>>
fb::lua::context::co_builder::run_pipeline(fb::async_executor&                                 executor,
                                           std::optional<std::weak_ptr<fb::thread_switchable>> weak,
                                           context*                                            lua_ptr,
                                           std::function<async::task<void>()>                  yield_fn,
                                           std::function<async::task<int>()>                   resume_fn)
{
    const auto abort_pipeline = [lua_ptr](const char* phase, const char* message) {
        if (message != nullptr)
            fb::logger::fatal("lua co_builder {} error: {}", phase, message);
        else
            fb::logger::fatal("lua co_builder {} error", phase);
        lua_ptr->release();
    };

    if (weak.has_value())
    {
        try
        {
            co_await executor.threads.switching(*weak);
        }
        catch (const std::exception& e)
        {
            abort_pipeline("thread switch", e.what());
            throw;
        }
        catch (...)
        {
            abort_pipeline("thread switch", nullptr);
            throw;
        }
    }

    if (yield_fn)
    {
        try
        {
            co_await yield_fn();
        }
        catch (const std::exception& e)
        {
            abort_pipeline("yield", e.what());
            throw;
        }
        catch (...)
        {
            abort_pipeline("yield", nullptr);
            throw;
        }
    }

    if (!resume_fn)
        co_return std::nullopt;

    try
    {
        co_await lua_ptr->_initial_thread.switching();
    }
    catch (const std::exception& e)
    {
        abort_pipeline("lua thread switch", e.what());
        throw;
    }
    catch (...)
    {
        abort_pipeline("lua thread switch", nullptr);
        throw;
    }

    try
    {
        co_return co_await resume_fn();
    }
    catch (const std::exception& e)
    {
        abort_pipeline("resume", e.what());
        throw;
    }
    catch (...)
    {
        abort_pipeline("resume", nullptr);
        throw;
    }
}

fb::lua::context::co_builder::co_builder(context& lua, fb::async_executor& executor) :
    _lua(lua),
    _executor(executor)
{ }

int fb::lua::context::co_builder::run()
{
    if (this->weak.has_value())
    {
        auto shared = this->weak->lock();
        if (shared == nullptr)
            return 0;
    }

    auto       yield_fn   = std::move(this->yield);
    auto       resume_fn  = std::move(this->resume);
    const auto has_resume = static_cast<bool>(resume_fn);

    int  sync_result = -1;
    auto lua_ptr     = &this->_lua;

    auto immediate = async::awaitable_then_immediate(
        run_pipeline(this->_executor, this->weak, lua_ptr, std::move(yield_fn), std::move(resume_fn)),
        [lua_ptr, &sync_result, has_resume](async::awaitable_result<std::optional<int>> result, bool immediate) {
            if (has_resume == false)
                return;

            try
            {
                auto n = result();
                if (n.has_value() == false)
                    return;

                if (immediate)
                    sync_result = *n;
                else
                    lua_ptr->resume(*n);
            }
            catch (const std::exception& e)
            {
                fb::logger::fatal("lua co_builder async completion error: {}", e.what());
            }
            catch (...)
            {
                fb::logger::fatal("lua co_builder async completion error: non-std exception");
            }
        });

    if (has_resume == false)
        return lua_ptr->yield(0);

    if (immediate)
        return sync_result;
    else
        return lua_ptr->yield(0);
}

fb::lua::context::co_builder fb::lua::context::new_co_builder()
{
    return co_builder(*this, this->executor);
}

root::root(fb::async_executor& executor, fb::thread& thread) :
    context(executor, ::luaL_newstate(), thread)
{
    luaL_openlibs(*this);
    ::lua_pushlightuserdata(*this, this);
    ::lua_setfield(*this, LUA_REGISTRYINDEX, root::REGISTRY_KEY);
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

bool root::dump(std::string_view path)
{
    if (path.empty())
        return true;

    auto path_str = std::string(path);

    if (auto cached = this->_bytecodes.find(path_str); cached != this->_bytecodes.end())
        return cached->second.empty() == false;

    if (luaL_loadfile(*this, path_str.c_str()) != LUA_OK)
    {
        context::pop(1); // pop error message
        this->_bytecodes[path_str] = std::vector<char>{};
        return false;
    }

    this->_bytecodes[path_str] = std::vector<char>();

    void*      params[] = {&this->_bytecodes[path_str]};
    const auto callback = [](lua_State* ctx, const void* bytes, size_t size, void* p) {
        auto params    = static_cast<void**>(p);
        auto bytecodes = static_cast<std::vector<char>*>(params[0]);

        for (size_t i = 0; i < size; i++)
        {
            bytecodes->push_back(static_cast<const char*>(bytes)[i]);
        }

        return 0;
    };

    ::lua_dump(*this, callback, params, 1);

    if (lua_pcall(*this, 0, LUA_MULTRET, 0) != LUA_OK)
    {
        context::pop(1);
        this->_bytecodes[path_str].clear();
        return false;
    }
    lua_settop(*this, 0);

    return true;
}

context* root::pop(context* parent, call_options options)
{
    if (this->idle.empty() == false)
    {
        auto& ctx = this->idle.begin()->second;
        auto  key = (lua_State*)*ctx;
        this->busy.insert({key, std::move(ctx)});
        this->idle.erase(key);
        this->busy[key]->parent(parent);
        this->busy[key]->options(options);
        return this->busy[key].get();
    }
    else if (this->idle.size() + this->busy.size() < DEFAULT_POOL_SIZE)
    {
        auto ptr = std::make_unique<fb::lua::thread>(this->executor, *this, parent, options);
        auto key = (lua_State*)*ptr.get();

        if (this->idle.contains(key) || this->busy.contains(key))
            return nullptr;

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

        if (lua_status(ctx) != LUA_OK)
            throw std::runtime_error("lua ctx's current state is not LUA_OK");

        lua_settop(ctx, 0);
        ctx.parent(nullptr);
        ctx.options(call_options{});
        ctx.clear_call_engaged();

        // Force garbage collection before moving to idle pool
        lua_gc(ctx, LUA_GCCOLLECT, 0);

        auto key = (lua_State*)ctx;
        this->idle.insert({key, std::move(this->busy[key])});
        this->busy.erase(key);
    };

    if (this->_initial_thread.id() != std::this_thread::get_id())
    {
        auto builder = this->_initial_thread.new_builder<void>();
        builder.func = [=, &ctx](auto&) -> async::task<void> {
            internal_func(ctx);
            co_return;
        };
        builder.enqueue();
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

    ctx.clear_call_engaged();
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

fb::lua::context_pool::context_pool(fb::async_executor& executor) :
    _executor(executor)
{
    for (auto& [id, thread] : executor.threads)
        this->_roots.insert({id, std::make_unique<root>(executor, *thread)});
}

fb::lua::context_pool::~context_pool()
{
    // std::unique_ptr handles cleanup automatically
}

context* fb::lua::context_pool::new_context(context* parent, call_options options)
{
    auto id = std::this_thread::get_id();
    if (this->_roots.contains(id) == false)
        return nullptr;

    auto* ctx = this->_roots[id]->pop(parent, options);
#if defined DEBUG || defined _DEBUG
    if (ctx != nullptr)
        ctx->clear_loaded_modules();
#endif
    return ctx;
}

context_guard fb::lua::context_pool::new_ctx_guard(context* parent, call_options options)
{
    return context_guard(this->new_context(parent, options));
}

context_guard fb::lua::context_pool::new_ctx_guard(std::string_view path,
                                                   std::string_view func,
                                                   context*         parent,
                                                   call_options     options)
{
    auto* ctx = this->new_context(parent, options);
    if (ctx == nullptr)
        return context_guard{};

    if (ctx->load(path) == false || ctx->func(func) == false)
    {
        ctx->release();
        return context_guard{};
    }

    return context_guard{ctx};
}

fb::lua::context_guard::context_guard(context* ctx) :
    _ctx(ctx)
{ }

fb::lua::context_guard::~context_guard()
{
    if (this->_ctx != nullptr && this->_ctx->call_engaged() == false)
        this->_ctx->release();
}

fb::lua::context_guard::context_guard(context_guard&& other) noexcept :
    _ctx(other._ctx)
{
    other._ctx = nullptr;
}

fb::lua::context_guard& fb::lua::context_guard::operator= (context_guard&& other) noexcept
{
    if (this != &other)
    {
        if (this->_ctx != nullptr && this->_ctx->call_engaged() == false)
            this->_ctx->release();

        this->_ctx = other._ctx;
        other._ctx = nullptr;
    }
    return *this;
}

async::task<void> fb::lua::context_pool::dump(std::string_view path)
{
    auto p = std::string{path};
    for (auto& [_, root] : this->_roots)
    {
        co_await root->switching();
        root->dump(p);
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

thread::thread(fb::async_executor& executor, context& owner, context* parent, call_options options) :
    context(executor, ::lua_newthread(owner), owner, parent),
    ref(luaL_ref(owner, LUA_REGISTRYINDEX))
{
    this->options(options);
    lua_checkstack(*this, 10000);
}

thread::thread(thread&& ctx) :
    context(ctx.executor, ctx._ctx, *ctx.owner, ctx.parent()),
    ref(ctx.ref)
{
    lua_checkstack(*this, 10000);
}

thread::~thread()
{
    luaL_unref(this->_ctx, LUA_REGISTRYINDEX, this->ref);
}