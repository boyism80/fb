#ifndef FB_GAME_STATE_MACHINE_H
#define FB_GAME_STATE_MACHINE_H

#include <async/task.h>
#include <fb/lua.h>
#include <fb/timer.h>

#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fb {
class thread;
}

namespace fb::game {

class server;

class state_machine : public fb::lua::luable
{
public:
    LUA_PROTOTYPE

    class container;

    static constexpr auto SCRIPT_DIRECTORY = "scripts/state_machine";

private:
    struct schedule_entry
    {
        uint64_t                   version = 0;
        std::shared_ptr<fb::timer> timer;
    };

    struct pending_hook
    {
        std::string                 hook;
        std::vector<fb::lua::value> args;
    };

private:
    fb::game::server&                               _server;
    const std::string                               _name;
    const std::string                               _path;
    fb::thread&                                     _thread;
    mutable std::mutex                              _mutex;
    std::unordered_map<std::string, fb::lua::value> _properties;
    std::unordered_map<std::string, schedule_entry> _schedules;
    uint64_t                                        _next_version = 0;
    bool                                            _running      = false;
    std::deque<pending_hook>                        _pending;

private:
    async::task<void> run(const std::string& hook, const std::vector<fb::lua::value>& args);

public:
    state_machine(fb::game::server& server, std::string name, std::string path, fb::thread& thread);
    state_machine(const state_machine&) = delete;
    state_machine(state_machine&&)      = delete;

public:
    const std::string&            name() const;
    fb::thread&                   thread() const;
    bool                          on_thread() const;
    std::optional<fb::lua::value> property(const std::string& key) const;
    void                          property(const std::string& key, std::optional<fb::lua::value> value);
    void              schedule(const std::string& id, uint32_t interval_ms, const std::string& hook, bool repeat);
    void              cancel(const std::string& id);
    void              call(std::string hook, std::vector<fb::lua::value> args);
    async::task<void> invoke(std::string hook, std::vector<fb::lua::value> args);
};

class state_machine::container
{
private:
    fb::game::server&                                               _server;
    std::unordered_map<std::string, std::shared_ptr<state_machine>> _machines;

public:
    explicit container(fb::game::server& server);
    container(const container&) = delete;
    container(container&&)      = delete;

public:
    async::task<void>              load();
    std::shared_ptr<state_machine> find(std::string_view name) const;
};

} // namespace fb::game

#endif // FB_GAME_STATE_MACHINE_H
