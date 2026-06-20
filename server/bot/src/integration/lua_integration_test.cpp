#include <fb/bot/integration/lua_integration_test.h>
#include <fb/bot/integration/lua_integration_model.h>
#include <fb/bot/builtin/integration_test.h>
#include <fb/bot/integration/lua_integration_protocol.h>
#include <fb/bot/integration/game_controller.h>
#include <fb/bot/integration/protocol_registry.h>
#include <fb/bot/container.h>
#include <fb/bot/game_bot.h>
#include <fb/config.h>
#include <fb/logger.h>
#include <fb/model/datetime.h>
#include <fb/protocol/header.h>

#include <fb/lua.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <cctype>
#include <format>
#include <functional>
#include <optional>
#include <typeinfo>

using namespace std::chrono_literals;
using namespace fb;
using namespace fb::bot::integration;

namespace {

constexpr const char* TEST_REGISTRY_KEY = "fb.bot.integration.test";

struct suite_capture
{
    lua_integration_test*           test{nullptr};
    lua_integration_test::suite_def suite;
};

struct discovery_context
{
    std::filesystem::path              base_dir;
    std::vector<std::filesystem::path> scripts;
};

std::optional<std::filesystem::path> find_integration_dir()
{
    constexpr std::array<const char*, 2> candidates = {
        "../../scripts/integration",
        "scripts/integration",
    };

    for (const auto& dir : candidates)
    {
        std::filesystem::path p(dir);
        if (std::filesystem::exists(p) && std::filesystem::is_directory(p))
            return p;
    }

    return std::nullopt;
}

std::filesystem::path resolve_test_path(const std::filesystem::path& base_dir, std::string_view name)
{
    auto filename = std::string(name);
    if (filename.ends_with(".lua"))
    {
        // use as-is
    }
    else if (filename.ends_with("_test"))
    {
        filename += ".lua";
    }
    else
    {
        filename += "_test.lua";
    }

    return base_dir / filename;
}

int lua_register_test(lua_State* L)
{
    auto* ctx = static_cast<discovery_context*>(lua_touserdata(L, lua_upvalueindex(1)));
    if (ctx == nullptr)
        return luaL_error(L, "register_test called outside integration test discovery");

    auto name = luaL_checkstring(L, 1);
    ctx->scripts.push_back(resolve_test_path(ctx->base_dir, name));
    return 0;
}

int lua_is_local_host(lua_State* L)
{
    auto local = fb::config<std::string_view>("ip") == "127.0.0.1";
    lua_pushboolean(L, local);
    return 1;
}

lua_integration_test::suite_def parse_suite_table(lua_State* L, int table_index);

int lua_test_suite(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);

    auto* capture = static_cast<suite_capture*>(lua_touserdata(L, lua_upvalueindex(1)));
    if (capture == nullptr)
        luaL_error(L, "test_suite called outside integration test load");

    if (capture->test != nullptr)
        capture->test->capture_suite(L, 1);
    else
        capture->suite = parse_suite_table(L, 1);

    return 0;
}

void append_package_path(lua_State* L, const char* path)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");
    auto current = lua_tostring(L, -1);
    auto merged  = std::format("{};{}", current != nullptr ? current : "", path);
    lua_pop(L, 1);
    lua_pushstring(L, merged.c_str());
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);
}

lua_integration_test::suite_def parse_suite_table(lua_State* L, int table_index)
{
    lua_integration_test::suite_def suite;
    const auto                      idx = lua_absindex(L, table_index);

    if (lua_getfield(L, idx, "name"); lua_isstring(L, -1))
        suite.name = lua_tostring(L, -1);
    lua_pop(L, 1);

    if (lua_getfield(L, idx, "bot_count"); lua_isinteger(L, -1))
        suite.bot_count = static_cast<uint32_t>(lua_tointeger(L, -1));
    lua_pop(L, 1);

    auto store_callback = [&](const char* key, int& ref) {
        if (lua_getfield(L, idx, key); lua_isfunction(L, -1))
        {
            lua_pushvalue(L, -1);
            ref = luaL_ref(L, LUA_REGISTRYINDEX);
        }
        lua_pop(L, 1);
    };

    store_callback("on_initialize", suite.on_initialize_ref);
    store_callback("on_scenario_started", suite.on_scenario_started_ref);
    store_callback("on_scenario_finished", suite.on_scenario_finished_ref);
    store_callback("on_parallel_scenario_started", suite.on_parallel_scenario_started_ref);
    store_callback("on_parallel_scenario_finished", suite.on_parallel_scenario_finished_ref);
    store_callback("on_finished", suite.on_finished_ref);

    if (lua_getfield(L, idx, "scenarios"); lua_istable(L, -1))
    {
        const auto scenarios_idx = lua_absindex(L, -1);
        for (int i = 1;; ++i)
        {
            lua_rawgeti(L, scenarios_idx, i);
            if (lua_isnil(L, -1))
            {
                lua_pop(L, 1);
                break;
            }

            if (lua_isfunction(L, -1))
            {
                lua_pushvalue(L, -1);
                auto ref = luaL_ref(L, LUA_REGISTRYINDEX);
                suite.scenarios.push_back({ref});
            }
            else if (lua_istable(L, -1))
            {
                if (lua_getfield(L, -1, "parallel"); lua_istable(L, -1))
                {
                    lua_integration_test::scenario_item item;
                    lua_pushnil(L);
                    while (lua_next(L, -2) != 0)
                    {
                        if (lua_isinteger(L, -2))
                        {
                            auto bot_index = static_cast<uint32_t>(lua_tointeger(L, -2));
                            if (lua_istable(L, -1))
                            {
                                for (int j = 1;; ++j)
                                {
                                    lua_rawgeti(L, -1, j);
                                    if (lua_isnil(L, -1))
                                    {
                                        lua_pop(L, 1);
                                        break;
                                    }

                                    if (lua_isfunction(L, -1))
                                    {
                                        lua_pushvalue(L, -1);
                                        auto ref = luaL_ref(L, LUA_REGISTRYINDEX);
                                        item.parallel.push_back({bot_index, ref});
                                    }
                                    lua_pop(L, 1);
                                }
                            }
                        }
                        lua_pop(L, 1);
                    }

                    if (item.parallel.empty() == false)
                        suite.scenarios.push_back(std::move(item));
                    else
                        fb::logger::warn("parallel scenario block has no functions");
                }
                lua_pop(L, 1);
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    return suite;
}

void release_suite_refs(lua_State* L, lua_integration_test::suite_def& suite)
{
    auto unref = [&](int ref) {
        if (ref != LUA_NOREF)
            luaL_unref(L, LUA_REGISTRYINDEX, ref);
    };

    unref(suite.on_initialize_ref);
    unref(suite.on_scenario_started_ref);
    unref(suite.on_scenario_finished_ref);
    unref(suite.on_parallel_scenario_started_ref);
    unref(suite.on_parallel_scenario_finished_ref);
    unref(suite.on_finished_ref);

    for (auto& scenario : suite.scenarios)
    {
        if (scenario.is_parallel())
        {
            for (auto& item : scenario.parallel)
                unref(item.function_ref);
        }
        else
        {
            unref(scenario.function_ref);
        }
    }
}

lua_integration_test::suite_def load_suite_metadata(lua_State* L, const std::filesystem::path& script_path)
{
    suite_capture capture;

    lua_pushlightuserdata(L, &capture);
    lua_pushcclosure(L, lua_test_suite, 1);
    lua_setglobal(L, "test_suite");

    if (luaL_dofile(L, script_path.string().c_str()) != LUA_OK)
    {
        auto* err = lua_tostring(L, -1);
        throw std::runtime_error(std::format("failed to load integration script '{}': {}",
                                             script_path.string(),
                                             err != nullptr ? err : "unknown error"));
    }

    return capture.suite;
}

} // namespace

namespace fb::bot::integration {

// clang-format off
IMPLEMENT_LUA_EXTENSION(lua_integration_test, "fb.integration.ctx")
    {"bot",       fb::bot::builtin::integration_test::builtin_bot},
    {"bot_count", fb::bot::builtin::integration_test::builtin_bot_count},
    {"sleep",     fb::bot::builtin::integration_test::builtin_sleep},
    {"hook",      fb::bot::builtin::integration_test::builtin_hook},
    {"unhook",    fb::bot::builtin::integration_test::builtin_unhook},
END_LUA_EXTENSION;
// clang-format on

uint32_t lua_integration_test::peek_bot_count(const std::filesystem::path& script_path)
{
    auto* L = luaL_newstate();
    luaL_openlibs(L);

    // Minimal dummy metatables so that require() in test scripts doesn't fail
    luaL_newmetatable(L, "fb.integration.ctx");
    lua_pop(L, 1);
    luaL_newmetatable(L, "fb.bot");
    lua_pop(L, 1);

    lua_protocol::register_metatables(L);
    lua_protocol::register_modules(L);
    append_package_path(L, "../../scripts/?.lua;../../scripts/?/init.lua;scripts/?.lua;scripts/?/init.lua");

    auto suite = load_suite_metadata(L, script_path);
    auto count = suite.bot_count;
    release_suite_refs(L, suite);
    lua_close(L);

    if (count == 0)
        return 1;

    return count;
}

std::vector<std::filesystem::path> lua_integration_test::discover_scripts()
{
    auto base_dir = find_integration_dir();
    if (base_dir.has_value() == false)
    {
        fb::logger::warn("integration test directory not found");
        return {};
    }

    auto* L = luaL_newstate();
    luaL_openlibs(L);

    discovery_context context{.base_dir = base_dir.value()};

    lua_pushlightuserdata(L, &context);
    lua_pushcclosure(L, lua_register_test, 1);
    lua_setglobal(L, "register_test");

    lua_pushcfunction(L, lua_is_local_host);
    lua_setglobal(L, "is_local_host");

    append_package_path(L, "../../scripts/?.lua;../../scripts/?/init.lua;scripts/?.lua;scripts/?/init.lua");

    auto registry_path = base_dir.value() / "registry.lua";
    if (luaL_dofile(L, registry_path.string().c_str()) != LUA_OK)
    {
        auto* err = lua_tostring(L, -1);
        lua_close(L);
        throw std::runtime_error(std::format("failed to load integration test registry '{}': {}",
                                             registry_path.string(),
                                             err != nullptr ? err : "unknown error"));
    }

    lua_close(L);
    return context.scripts;
}

lua_integration_test::lua_integration_test(game_bot_controller& controller, std::filesystem::path script_path) :
    bot_integration_test(controller, peek_bot_count(script_path)),
    _script_path(script_path.string())
{ }

lua_integration_test::~lua_integration_test()
{
    this->release_lua_refs();
}

void lua_integration_test::init_lua()
{
    auto& thread    = *this->controller.container.threads.at(0);
    this->_lua_root = std::make_unique<fb::lua::root>(this->controller.container, thread);

    auto& lua = *this->_lua_root;

    // Register type hierarchy
    lua.build<fb::lua::luable>();
    lua.build<fb::bot::game_bot, fb::lua::luable>();
    lua.build<lua_integration_test, fb::lua::luable>();

    // Register protocol metatables (request/response tokens — not luable types)
    lua_protocol::register_metatables((lua_State*)lua);

    // Register preloaded modules
    lua_protocol::register_modules(lua);

    // Set package search paths
    lua.package_path("../../scripts/?.lua");
    lua.package_path("../../scripts/?/init.lua");
    lua.package_path("scripts/?.lua");
    lua.package_path("scripts/?/init.lua");

    register_model_globals(lua);
    lua.build("log", fb::bot::builtin::integration_test::builtin_log);
}

void lua_integration_test::load_script()
{
    if (this->_lua_root == nullptr)
        return;

    auto  capture = suite_capture{.test = this};
    auto* L       = (lua_State*)*this->_lua_root;

    lua_pushlightuserdata(L, &capture);
    lua_pushcclosure(L, lua_test_suite, 1);
    lua_setglobal(L, "test_suite");

    this->_lua_root->execute(_script_path);
}

async::task<void> lua_integration_test::on_activated(game_bot_controller& controller)
{
    this->init_lua();
    this->load_script();

    co_await bot_integration_test::on_activated(controller);
}

void lua_integration_test::release_lua_refs()
{
    if (this->_lua_root == nullptr)
        return;

    auto* L = (lua_State*)*this->_lua_root;
    release_suite_refs(L, this->_suite);
    this->_hook_refs.clear();
}

void lua_integration_test::clear_all_hooks()
{
    if (this->_lua_root == nullptr)
        return;

    for (auto& [opcode, bindings] : this->_hook_refs)
    {
        for (auto& binding : bindings)
        {
            if (binding.lua_ctx == nullptr)
                continue;

            luaL_unref((lua_State*)*binding.lua_ctx, LUA_REGISTRYINDEX, binding.lua_ref);
        }

        this->controller.unhook_opcode(this, opcode);
    }
    this->_hook_refs.clear();
}

void lua_integration_test::push_ctx(fb::lua::context& ctx)
{
    ctx.pushobject(this);
}

void lua_integration_test::push_bot(fb::lua::context& ctx, std::shared_ptr<fb::bot::game_bot> bot)
{
    ctx.pushobject(bot);
}

async::task<void> lua_integration_test::do_sleep(std::chrono::milliseconds duration)
{
    co_await this->sleep(duration);
}

void lua_integration_test::capture_suite(lua_State* L, int table_index)
{
    this->_suite = parse_suite_table(L, table_index);
}

void lua_integration_test::register_opcode_hook(uint8_t               opcode,
                                                int                   lua_ref,
                                                const protocol_entry* entry,
                                                fb::lua::context*     lua_ctx)
{
    this->_hook_refs[opcode].push_back({lua_ref, lua_ctx});

    this->controller.hook_opcode(
        this,
        opcode,
        [this, lua_ref, entry, lua_ctx](game_bot& bot, const fb::protocol::header& header) -> async::task<void> {
            co_await this->invoke_lua_hook(lua_ref, *lua_ctx, bot, header, entry);
        });
}

void lua_integration_test::unhook_opcode(uint8_t opcode)
{
    auto it = this->_hook_refs.find(opcode);
    if (it != this->_hook_refs.end())
    {
        for (auto& binding : it->second)
        {
            if (binding.lua_ctx == nullptr)
                continue;

            luaL_unref((lua_State*)*binding.lua_ctx, LUA_REGISTRYINDEX, binding.lua_ref);
        }

        this->_hook_refs.erase(it);
    }

    this->controller.unhook_opcode(this, opcode);
}

async::task<void> lua_integration_test::invoke_lua_hook(int                         lua_ref,
                                                        fb::lua::context&           lua_ctx,
                                                        fb::bot::game_bot&          bot,
                                                        const fb::protocol::header& header,
                                                        const protocol_entry*       entry)
{
    auto* L = (lua_State*)lua_ctx;

    lua_rawgeti(L, LUA_REGISTRYINDEX, lua_ref);
    lua_ctx.pushobject(this);

    auto                               bots = this->get_test_bots();
    std::shared_ptr<fb::bot::game_bot> pushed_bot;
    for (auto& test_bot : bots)
    {
        if (test_bot.get() == &bot)
        {
            pushed_bot = test_bot;
            break;
        }
    }
    if (pushed_bot == nullptr)
        co_return;

    lua_ctx.pushobject(pushed_bot);

    if (entry->clone != nullptr)
    {
        auto cloned = entry->clone(header);
        entry->marshal_lua(L, *cloned);
    }
    else
    {
        entry->marshal_lua(L, header);
    }

    if (lua_pcall(L, 3, 0, 0) != LUA_OK)
    {
        auto err = lua_ctx.tostring(-1);
        fb::logger::fatal("{}: hook callback failed: {}", this->name(), err.empty() ? "unknown error" : err);
        lua_ctx.pop(1);
    }

    co_return;
}

async::task<bool> lua_integration_test::run_lua_function(int func_ref)
{
    auto  opts = fb::lua::call_options{.auto_release = false, .auto_resume_parent = false};
    auto* ctx  = this->_lua_root->pop(nullptr, opts);
    if (ctx == nullptr)
        co_return false;

    lua_rawgeti(*ctx, LUA_REGISTRYINDEX, func_ref);
    this->push_ctx(*ctx);

    bool result = false;
    try
    {
        co_await ctx->call(1);

        if (ctx->argc() >= 1)
            result = ctx->toboolean(-1);

        ctx->release();
    }
    catch (const std::exception& e)
    {
        auto* what = e.what();
        if (what != nullptr && what[0] != '\0')
            fb::logger::fatal("{}: {}", this->name(), what);
        else
            fb::logger::fatal("{}: {} (empty what())", this->name(), typeid(e).name());

        if (ctx != nullptr && lua_status(*ctx) != LUA_OK && ctx->argc() >= 1)
        {
            auto err = ctx->tostring(-1);
            if (err.empty() == false)
                fb::logger::fatal("{}: lua stack error: {}", this->name(), err);
        }
    }
    catch (...)
    {
        fb::logger::fatal("{}: non-std exception", this->name());
    }

    co_return result;
}

async::task<void> lua_integration_test::run_lua_void(int func_ref, std::optional<uint32_t> extra_arg)
{
    auto  opts = fb::lua::call_options{.auto_release = true, .auto_resume_parent = false};
    auto* ctx  = this->_lua_root->pop(nullptr, opts);
    if (ctx == nullptr)
        co_return;

    lua_rawgeti(*ctx, LUA_REGISTRYINDEX, func_ref);
    this->push_ctx(*ctx);

    auto nargs = 1;
    if (extra_arg.has_value())
    {
        ctx->pushinteger(static_cast<lua_Integer>(*extra_arg));
        nargs = 2;
    }

    try
    {
        co_await ctx->call(nargs);
    }
    catch (const std::exception& e)
    {
        auto* what = e.what();
        if (what != nullptr && what[0] != '\0')
            fb::logger::fatal("{}: {}", this->name(), what);
        else
            fb::logger::fatal("{}: {} (empty what())", this->name(), typeid(e).name());
    }
    catch (...)
    {
        fb::logger::fatal("{}: non-std exception", this->name());
    }
}

std::string lua_integration_test::name() const
{
    if (this->_suite.name.empty() == false)
        return this->_suite.name;

    return std::filesystem::path(this->_script_path).stem().string();
}

generator<bot_integration_test::scenario_t> lua_integration_test::on_generate_scenario()
{
    for (const auto& scenario : this->_suite.scenarios)
    {
        if (scenario.is_parallel())
        {
            std::vector<std::pair<uint32_t, scenario_t>> pairs;
            for (const auto& item : scenario.parallel)
            {
                auto ref = item.function_ref;
                auto idx = item.bot_index;
                pairs.emplace_back(idx, [this, ref]() -> async::task<bool> {
                    co_return co_await this->run_lua_function(ref);
                });
            }

            co_yield [this, pairs = std::move(pairs)]() -> async::task<bool> {
                co_return co_await this->parallel_scenarios(pairs);
            };
        }
        else
        {
            auto ref = scenario.function_ref;
            co_yield [this, ref]() -> async::task<bool> {
                co_return co_await this->run_lua_function(ref);
            };
        }
    }
}

async::task<void> lua_integration_test::on_initialize(game_bot_controller& controller)
{
    co_await bot_integration_test::on_initialize(controller);

    if (this->_suite.on_initialize_ref != LUA_NOREF)
        co_await this->run_lua_void(this->_suite.on_initialize_ref);
}

async::task<void> lua_integration_test::on_scenario_started(uint32_t scenario_index)
{
    if (this->_suite.on_scenario_started_ref != LUA_NOREF)
        co_await this->run_lua_void(this->_suite.on_scenario_started_ref, scenario_index);

    co_await bot_integration_test::on_scenario_started(scenario_index);
}

async::task<void> lua_integration_test::on_scenario_finished(uint32_t scenario_index)
{
    if (this->_suite.on_scenario_finished_ref != LUA_NOREF)
        co_await this->run_lua_void(this->_suite.on_scenario_finished_ref, scenario_index);

    co_await bot_integration_test::on_scenario_finished(scenario_index);
}

async::task<void> lua_integration_test::on_parallel_scenario_started(uint32_t id)
{
    if (this->_suite.on_parallel_scenario_started_ref != LUA_NOREF)
        co_await this->run_lua_void(this->_suite.on_parallel_scenario_started_ref, id);

    co_await bot_integration_test::on_parallel_scenario_started(id);
}

async::task<void> lua_integration_test::on_parallel_scenario_finished(uint32_t id)
{
    if (this->_suite.on_parallel_scenario_finished_ref != LUA_NOREF)
        co_await this->run_lua_void(this->_suite.on_parallel_scenario_finished_ref, id);

    co_await bot_integration_test::on_parallel_scenario_finished(id);
}

async::task<void> lua_integration_test::on_finished()
{
    this->clear_all_hooks();

    if (this->_suite.on_finished_ref != LUA_NOREF)
        co_await this->run_lua_void(this->_suite.on_finished_ref);

    co_await bot_integration_test::on_finished();
}

} // namespace fb::bot::integration

using namespace fb::bot;

int builtin::integration_test::builtin_bot(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto test = lua->touserdata<fb::bot::integration::lua_integration_test>(1);
    if (test == nullptr)
        return 0;

    auto index = static_cast<size_t>(lua->tointeger(2));
    auto bots  = test->get_test_bots();
    if (index >= bots.size())
        return luaL_error(L, "bot index out of range: %d", static_cast<int>(index));

    lua->pushobject(bots[index]);
    return 1;
}

int builtin::integration_test::builtin_bot_count(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto test = lua->touserdata<fb::bot::integration::lua_integration_test>(1);
    if (test == nullptr)
        return 0;

    lua->pushinteger(static_cast<lua_Integer>(test->get_test_bots().size()));
    return 1;
}

int builtin::integration_test::builtin_sleep(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto test = lua->touserdata<fb::bot::integration::lua_integration_test>(1);
    if (test == nullptr)
        return 0;

    auto ms = lua->tointeger(2);

    auto builder  = lua->new_co_builder();
    builder.yield = [test, ms]() -> async::task<void> {
        co_await test->do_sleep(std::chrono::milliseconds(ms));
    };
    builder.resume = []() -> async::task<int> {
        co_return 0;
    };
    return builder.run();
}

namespace {

bool log_level_equals(std::string_view level, std::string_view expected)
{
    if (level.size() != expected.size())
        return false;

    for (size_t i = 0; i < level.size(); ++i)
    {
        if (std::tolower(static_cast<unsigned char>(level[i])) != std::tolower(static_cast<unsigned char>(expected[i])))
            return false;
    }

    return true;
}

} // namespace

int builtin::integration_test::builtin_log(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    if (lua->argc() < 2 || lua->is_string(1) == false || lua->is_string(2) == false)
        return luaL_error(L, "log(level, message) requires two string arguments");

    auto level   = lua->tostring(1);
    auto message = lua->tostring(2);

    if (log_level_equals(level, "debug"))
        fb::logger::debug("{}", message);
    else if (log_level_equals(level, "info"))
        fb::logger::info("{}", message);
    else if (log_level_equals(level, "warn"))
        fb::logger::warn("{}", message);
    else if (log_level_equals(level, "fatal"))
        fb::logger::fatal("{}", message);
    else
        return luaL_error(L, "unknown log level: %s (expected debug, info, warn, fatal)", level.c_str());

    return 0;
}

int builtin::integration_test::builtin_hook(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto test = lua->touserdata<fb::bot::integration::lua_integration_test>(1);
    if (test == nullptr)
        return 0;

    if (lua->argc() < 3 || lua->is_string(2) == false || lua->is_function(3) == false)
        return luaL_error(L, "hook(name, function) requires a response name and callback function");

    auto name = lua->tostring(2);

    auto* entry =
        fb::bot::integration::protocol_registry::find(fb::bot::integration::protocol_direction::response, name.c_str());
    if (entry == nullptr)
        return luaL_error(L, "unknown response event: %s", name.c_str());

    lua_pushvalue(L, 3);
    auto ref = luaL_ref(L, LUA_REGISTRYINDEX);

    auto* lua_ctx = fb::lua::get(L);
    if (lua_ctx == nullptr)
        return luaL_error(L, "hook must be registered from an active lua coroutine");

    test->register_opcode_hook(entry->opcode, ref, entry, lua_ctx);
    return 0;
}

int builtin::integration_test::builtin_unhook(lua_State* L)
{
    auto lua = fb::lua::get(L);
    if (lua == nullptr)
        return 0;

    auto test = lua->touserdata<fb::bot::integration::lua_integration_test>(1);
    if (test == nullptr)
        return 0;

    if (lua->argc() < 2 || lua->is_string(2) == false)
        return luaL_error(L, "unhook(name) requires a response name");

    auto name = lua->tostring(2);

    auto* entry =
        fb::bot::integration::protocol_registry::find(fb::bot::integration::protocol_direction::response, name.c_str());
    if (entry == nullptr)
        return luaL_error(L, "unknown response event: %s", name.c_str());

    test->unhook_opcode(entry->opcode);
    return 0;
}
