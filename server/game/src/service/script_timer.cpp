#include <fb/game/service/script_timer.h>
#include <fb/game/server.h>
#include <fb/game/map.h>
#include <fb/logger.h>
#include <fb/lua.h>
#include <chrono>
#include <vector>

using namespace fb::game;
using namespace std::chrono_literals;

service::script_timer::script_timer(fb::game::server& server) :
    server(server)
{ }

void service::script_timer::erase_unlocked(uint64_t id)
{
    auto it = this->_by_id.find(id);
    if (it == this->_by_id.end())
        return;

    this->_by_key.erase(key_t{it->second.map_id, it->second.name});
    this->_by_id.erase(it);
}

void service::script_timer::erase(uint64_t id)
{
    auto _ = std::lock_guard(this->_mutex);
    this->erase_unlocked(id);
}

bool service::script_timer::cancel(uint64_t id)
{
    std::shared_ptr<fb::timer> timer;
    {
        auto _  = std::lock_guard(this->_mutex);
        auto it = this->_by_id.find(id);
        if (it == this->_by_id.end())
            return false;

        timer = it->second.timer;
        this->erase_unlocked(id);
    }

    if (timer != nullptr)
        timer->cancel();
    return true;
}

bool service::script_timer::cancel(uint32_t map_id, std::string_view name)
{
    if (name.empty())
        return false;

    std::shared_ptr<fb::timer> timer;
    {
        auto _   = std::lock_guard(this->_mutex);
        auto key = key_t{map_id, std::string(name)};
        auto it  = this->_by_key.find(key);
        if (it == this->_by_key.end())
            return false;

        auto id_it = this->_by_id.find(it->second);
        if (id_it != this->_by_id.end())
            timer = id_it->second.timer;

        this->erase_unlocked(it->second);
    }

    if (timer != nullptr)
        timer->cancel();
    return true;
}

void service::script_timer::cancel_all(uint32_t map_id)
{
    auto timers = std::vector<std::shared_ptr<fb::timer>>{};
    {
        auto _        = std::lock_guard(this->_mutex);
        auto to_erase = std::vector<uint64_t>{};
        for (auto& [id, entry] : this->_by_id)
        {
            if (entry.map_id != map_id)
                continue;
            timers.push_back(entry.timer);
            to_erase.push_back(id);
        }
        for (auto id : to_erase)
            this->erase_unlocked(id);
    }

    for (auto& timer : timers)
    {
        if (timer != nullptr)
            timer->cancel();
    }
}

async::task<std::optional<uint64_t>> service::script_timer::create_or_replace(fb::game::map& map,
                                                                              std::string    name,
                                                                              uint32_t       interval_ms,
                                                                              std::string    path,
                                                                              std::string    func,
                                                                              bool           repeat)
{
    if (name.empty() || path.empty() || func.empty() || interval_ms == 0)
        co_return std::nullopt;

    auto* thread = map.thread();
    if (thread == nullptr)
        co_return std::nullopt;

    auto map_id = map.id;
    this->cancel(map_id, name);

    auto at_limit = false;
    {
        auto _   = std::lock_guard(this->_mutex);
        at_limit = this->_by_id.size() >= MAX_ACTIVE;
    }
    if (at_limit)
    {
        fb::logger::warn("script_timer: max active timers ({}) reached", MAX_ACTIVE);
        co_return std::nullopt;
    }

    co_await thread->switching();

    if (this->server.maps.find(map_id) == nullptr)
        co_return std::nullopt;

    auto id       = uint64_t{0};
    auto duration = fb::model::timespan(std::chrono::milliseconds(interval_ms));
    auto type     = repeat ? fb::timer::repeat_type::repeat : fb::timer::repeat_type::once;

    {
        auto _ = std::lock_guard(this->_mutex);
        id     = this->_next_id++;
    }

    auto* server = &this->server;
    auto  timer  = thread->settimer(
        [server, id, map_id, path, func, repeat](const fb::model::datetime&, std::thread::id) -> async::task<void> {
            // Copy captures into the coroutine frame before any suspend. The
            // lambda object itself may be destroyed while lua->call is yielded.
            auto* srv         = server;
            auto  timer_id    = id;
            auto  mid         = map_id;
            auto  script_path = path;
            auto  script_func = func;
            auto  once        = (repeat == false);
            auto& timers      = srv->script_timers;

            if (srv->maps.find(mid) == nullptr)
            {
                timers.cancel(timer_id);
                co_return;
            }

            auto lua = srv->lua.open(script_path, script_func);
            if (lua)
            {
                std::ignore = co_await lua->call(0);
            }
            else
            {
                fb::logger::warn("script_timer: failed to open {}:{}", script_path, script_func);
            }

            if (once)
                timers.cancel(timer_id);
            co_return;
        },
        duration,
        type);

    {
        auto _       = std::lock_guard(this->_mutex);
        auto entry   = entry_t{};
        entry.id     = id;
        entry.map_id = map_id;
        entry.name   = name;
        entry.path   = std::move(path);
        entry.func   = std::move(func);
        entry.repeat = repeat;
        entry.timer  = timer;

        this->_by_id[id]                   = std::move(entry);
        this->_by_key[key_t{map_id, name}] = id;
    }

    co_return id;
}
