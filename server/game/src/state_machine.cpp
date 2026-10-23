#include <fb/game/state_machine.h>

#include <fb/game/server.h>
#include <fb/logger.h>
#include <fb/thread.h>

#include <chrono>
#include <exception>
#include <filesystem>
#include <format>
#include <functional>
#include <stdexcept>
#include <tuple>
#include <utility>

using namespace fb::game;

state_machine::state_machine(fb::game::server& server, std::string name, std::string path, fb::thread& thread) :
    _server(server),
    _name(std::move(name)),
    _path(std::move(path)),
    _thread(thread)
{ }

const std::string& state_machine::name() const
{
    return this->_name;
}

fb::thread& state_machine::thread() const
{
    return this->_thread;
}

bool state_machine::on_thread() const
{
    return this->_server.threads.current() == &this->_thread;
}

std::optional<fb::lua::value> state_machine::property(const std::string& key) const
{
    auto _     = std::lock_guard(this->_mutex);
    auto found = this->_properties.find(key);
    if (found == this->_properties.end())
        return std::nullopt;

    return found->second;
}

void state_machine::property(const std::string& key, std::optional<fb::lua::value> value)
{
    if (this->on_thread() == false)
        throw std::runtime_error(std::format("state machine {} sets {} only on its own thread", this->_name, key));

    auto _ = std::lock_guard(this->_mutex);
    if (value.has_value())
        this->_properties.insert_or_assign(key, std::move(value.value()));
    else
        this->_properties.erase(key);
}

void state_machine::schedule(const std::string& id, uint32_t interval_ms, const std::string& hook, bool repeat)
{
    if (this->on_thread() == false)
        throw std::runtime_error(std::format("state machine {} schedules {} only on its own thread", this->_name, id));

    this->cancel(id);

    auto version = ++this->_next_version;
    auto weak    = this->weak_from_this_as<state_machine>();
    auto timer   = this->_thread.settimer(
        [weak, id, hook, version, repeat](const fb::model::datetime&, std::thread::id) -> async::task<void> {
            auto self       = weak.lock();
            auto schedule   = id;
            auto hook_name  = hook;
            auto expected   = version;
            auto repeatable = repeat;
            if (self == nullptr)
                co_return;

            auto found = self->_schedules.find(schedule);
            if (found == self->_schedules.end() || found->second.version != expected)
                co_return;

            if (repeatable == false)
                self->_schedules.erase(found);
            else if (self->_running)
                co_return;

            co_await self->invoke(hook_name, {});
        },
        fb::model::timespan(std::chrono::milliseconds(interval_ms)),
        repeat ? fb::timer::repeat_type::repeat : fb::timer::repeat_type::once);

    this->_schedules.insert_or_assign(id, schedule_entry{version, timer});
}

void state_machine::cancel(const std::string& id)
{
    if (this->on_thread() == false)
        throw std::runtime_error(std::format("state machine {} cancels {} only on its own thread", this->_name, id));

    auto found = this->_schedules.find(id);
    if (found == this->_schedules.end())
        return;

    if (found->second.timer != nullptr)
        found->second.timer->cancel();
    this->_schedules.erase(found);
}

void state_machine::call(std::string hook, std::vector<fb::lua::value> args)
{
    auto self    = this->shared_from_this_as<state_machine>();
    auto builder = this->_thread.new_builder<void>();
    builder.func = [self, hook = std::move(hook), args = std::move(args)](auto&) -> async::task<void> {
        co_await self->invoke(hook, args);
    };
    builder.enqueue();
}

async::task<void> state_machine::invoke(std::string hook, std::vector<fb::lua::value> args)
{
    auto self = this->shared_from_this_as<state_machine>();
    if (self->_running)
    {
        self->_pending.push_back(pending_hook{std::move(hook), std::move(args)});
        co_return;
    }

    self->_running = true;
    co_await self->run(hook, args);
    while (self->_pending.empty() == false)
    {
        auto next = std::move(self->_pending.front());
        self->_pending.pop_front();
        co_await self->run(next.hook, next.args);
    }
    self->_running = false;
}

async::task<void> state_machine::run(const std::string& hook, const std::vector<fb::lua::value>& args)
{
    auto self = this->shared_from_this_as<state_machine>();
    try
    {
        auto lua = self->_server.lua.open(self->_path, hook);
        if (!lua)
            co_return;

        lua->pushobject(*self);
        for (const auto& arg : args)
        {
            arg.push(*lua);
        }

        std::ignore = co_await lua->call(static_cast<int>(args.size()) + 1);
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("state machine {} hook {} failed: {}", self->_name, hook, e.what());
    }
    catch (...)
    {
        fb::logger::warn("state machine {} hook {} failed", self->_name, hook);
    }
}

state_machine::container::container(fb::game::server& server) :
    _server(server)
{ }

async::task<void> state_machine::container::load()
{
    if (std::filesystem::exists(SCRIPT_DIRECTORY) == false)
        co_return;

    auto machines = std::vector<std::shared_ptr<state_machine>>{};
    for (const auto& entry : std::filesystem::directory_iterator(SCRIPT_DIRECTORY))
    {
        if (entry.is_regular_file() == false || entry.path().extension() != ".lua")
            continue;

        auto  name   = entry.path().stem().string();
        auto* thread = this->_server.threads.modular(static_cast<uint32_t>(std::hash<std::string>{}(name)));
        if (thread == nullptr)
            continue;

        auto path    = std::format("{}/{}.lua", SCRIPT_DIRECTORY, name);
        auto machine = std::make_shared<state_machine>(this->_server, name, path, *thread);
        this->_machines.insert_or_assign(name, machine);
        machines.push_back(machine);
    }

    for (auto& machine : machines)
    {
        auto builder = machine->thread().new_builder<void>();
        builder.func = [machine](auto&) -> async::task<void> {
            co_await machine->invoke("on_init", {});
        };
        co_await builder.dispatch();
    }
}

std::shared_ptr<state_machine> state_machine::container::find(std::string_view name) const
{
    auto found = this->_machines.find(std::string(name));
    if (found == this->_machines.end())
        return nullptr;

    return found->second;
}
