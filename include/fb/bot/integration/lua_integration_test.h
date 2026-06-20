#ifndef __BOT_INTEGRATION_LUA_INTEGRATION_TEST_H__
#define __BOT_INTEGRATION_LUA_INTEGRATION_TEST_H__

#include <fb/bot/integration/test_case.h>
#include <fb/bot/game_bot.h>
#include <fb/lua.h>
#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct lua_State;

namespace fb::bot::integration {

struct protocol_entry;

class lua_integration_test : public bot_integration_test, public fb::lua::luable
{
public:
    LUA_PROTOTYPE

    struct parallel_scenario_item
    {
        uint32_t bot_index;
        int      function_ref{LUA_NOREF};
    };

    struct scenario_item
    {
        int                                 function_ref{LUA_NOREF};
        std::vector<parallel_scenario_item> parallel;

        bool is_parallel() const
        {
            return this->parallel.empty() == false;
        }
    };

    struct suite_def
    {
        std::string                name;
        uint32_t                   bot_count{1};
        int                        on_initialize_ref{LUA_NOREF};
        int                        on_scenario_started_ref{LUA_NOREF};
        int                        on_scenario_finished_ref{LUA_NOREF};
        int                        on_parallel_scenario_started_ref{LUA_NOREF};
        int                        on_parallel_scenario_finished_ref{LUA_NOREF};
        int                        on_finished_ref{LUA_NOREF};
        std::vector<scenario_item> scenarios;
    };

private:
    struct hook_binding
    {
        int               lua_ref{LUA_NOREF};
        fb::lua::context* lua_ctx{nullptr};
    };

    std::unique_ptr<fb::lua::root>                         _lua_root;
    std::string                                            _script_path;
    suite_def                                              _suite;
    std::unordered_map<uint8_t, std::vector<hook_binding>> _hook_refs;

public:
    lua_integration_test(game_bot_controller& controller, std::filesystem::path script_path);
    ~lua_integration_test() override;

    static std::vector<std::filesystem::path> discover_scripts();
    static uint32_t                           peek_bot_count(const std::filesystem::path& script_path);

    void capture_suite(lua_State* L, int table_index);

    void push_ctx(fb::lua::context& ctx);
    void push_bot(fb::lua::context& ctx, std::shared_ptr<fb::bot::game_bot> bot);
    void register_opcode_hook(uint8_t opcode, int lua_ref, const protocol_entry* entry, fb::lua::context* lua_ctx);
    void unhook_opcode(uint8_t opcode);

    async::task<void> do_sleep(std::chrono::milliseconds duration);

    std::string name() const override;

    async::task<void> on_activated(game_bot_controller& controller) override;

protected:
    generator<scenario_t> on_generate_scenario() override;
    async::task<void>     on_initialize(game_bot_controller& controller) override;
    async::task<void>     on_scenario_started(uint32_t scenario_index) override;
    async::task<void>     on_scenario_finished(uint32_t scenario_index) override;
    async::task<void>     on_parallel_scenario_started(uint32_t id) override;
    async::task<void>     on_parallel_scenario_finished(uint32_t id) override;
    async::task<void>     on_finished() override;

private:
    void init_lua();
    void load_script();
    void release_lua_refs();
    void clear_all_hooks();

    async::task<bool> run_lua_function(int func_ref);
    async::task<void> run_lua_void(int func_ref, std::optional<uint32_t> extra_arg = std::nullopt);
    async::task<void> invoke_lua_hook(int                         lua_ref,
                                      fb::lua::context&           lua_ctx,
                                      fb::bot::game_bot&          bot,
                                      const fb::protocol::header& header,
                                      const protocol_entry*       entry);
};

} // namespace fb::bot::integration

#endif
