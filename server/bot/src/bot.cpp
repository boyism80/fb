#include <fb/bot/bot.h>

#include <fb/bot/container.h>
#include <fb/bot/controller.h>
#include <fb/thread.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iterator>
#include <utility>
#include <vector>

using namespace fb::bot;

base_bot::base_bot(fb::async_executor&                                                 executor,
                   base_bot_controller&                                                bot_controller,
                   std::function<async::task<void>(fb::socket<void*>&, fb::stream&)>&& on_receive,
                   std::function<async::task<void>(fb::socket<void*>&)>&&              on_closed,
                   uint32_t                                                            id) :
    fb::socket<void*>(executor, std::move(on_receive), std::move(on_closed)),
    _executor(executor),
    _bot_controller(bot_controller),
    id(id)
{ }

base_bot::~base_bot()
{ }

void base_bot::connect(const boost::asio::ip::tcp::endpoint& endpoint)
{
    try
    {
        this->async_connect(endpoint, [this, endpoint](const boost::system::error_code& error) {
            if (error)
            {
                fb::logger::fatal(error.message());
                this->connect(endpoint);
                return;
            }

            boost::asio::co_spawn(this->_executor.io_context, this->recv(), boost::asio::detached);

            auto builder = this->thread()->new_builder<void>();
            builder.func = [this](auto& thread) -> async::task<void> {
                co_await this->_bot_controller.on_bot_connected(*this);
            };

            builder.enqueue();
        });
    }
    catch (...)
    {
        fb::logger::fatal("connection failed");
        this->connect(endpoint);
    }
}

bool base_bot::on_encrypt(fb::stream& out)
{
    return this->encryption().encrypt(out) != static_cast<uint32_t>(-1);
}

bool base_bot::on_wrap(fb::stream& out)
{
    return this->encryption().wrap(out) != static_cast<uint32_t>(-1);
}

bool base_bot::accepts(const fb::protocol::header& packet) const
{
    return true;
}

async::task<size_t> base_bot::send(const fb::protocol::header& packet, bool encrypt, bool wrap)
{
    if (this->accepts(packet) == false)
        co_return 0;

    auto& stats = this->_bot_controller.container.stats;
    stats.count(stats.name(typeid(packet)), false);
    co_return co_await fb::socket<void*>::send(packet, encrypt, wrap);
}

async::task<bool> base_bot::changed(std::chrono::milliseconds within)
{
    if (this->_aborted)
        co_return false;

    auto change = std::make_shared<async::task_completion_source<bool>>();
    this->_changes.push_back(change);
    auto timer = std::shared_ptr<fb::timer>{};
    if (within > std::chrono::milliseconds{0})
    {
        timer = this->thread()->settimer(
            [change](auto&, auto) -> async::task<void> {
                auto failure = std::exception_ptr{};
                std::ignore  = change->try_set_value(false, failure);
                co_return;
            },
            within,
            fb::timer::repeat_type::once);
    }

    auto result = co_await change->task();
    if (timer != nullptr)
        timer->cancel();

    co_return result;
}

bool base_bot::process_hooks(uint8_t opcode, fb::protocol::header& header)
{
    this->assert_thread();

    auto matched = false;
    if (this->_hooks.contains(opcode))
    {
        auto& matched_hooks = this->_hooks.at(opcode);
        auto  i             = std::find_if(matched_hooks.begin(), matched_hooks.end(), [&header](const auto& hook) {
            return hook.condition(header);
        });

        if (i != matched_hooks.end())
        {
            auto callback = i->matched;
            matched_hooks.erase(i);
            callback(header);
            matched = true;
        }
    }

    this->wake();
    return matched;
}

async::task<void> base_bot::yield()
{
    auto turn    = std::make_shared<async::task_completion_source<bool>>();
    auto builder = this->thread()->new_builder<void>();
    builder.func = [turn](auto&) -> async::task<void> {
        auto failure = std::exception_ptr{};
        std::ignore  = turn->try_set_value(true, failure);
        co_return;
    };

    builder.enqueue();
    std::ignore = co_await turn->task();
}

void base_bot::wake(bool value)
{
    if (this->_changes.empty())
        return;

    auto changes = std::move(this->_changes);
    this->_changes.clear();
    auto thread  = this->thread();
    auto builder = thread->new_builder<void>();
    builder.func = [changes = std::move(changes), value](auto&) -> async::task<void> {
        for (auto& change : changes)
        {
            auto failure = std::exception_ptr{};
            std::ignore  = change->try_set_value(value, failure);
        }

        co_return;
    };

    builder.enqueue();
}

bool base_bot::remove_hook_by_context(uint8_t opcode, const void* context_ptr)
{
    this->assert_thread();

    if (this->_hooks.contains(opcode))
    {
        auto& hooks = this->_hooks.at(opcode);
        auto  i     = std::find_if(hooks.begin(), hooks.end(), [context_ptr](const auto& hook) {
            return hook.context_ptr == context_ptr;
        });

        if (i != hooks.end())
        {
            hooks.erase(i);
            return true;
        }
    }

    return false;
}

void base_bot::interrupt_hooks()
{
    this->assert_thread();

    auto released = std::vector<hook_params>{};
    for (auto& [_, entries] : this->_hooks)
    {
        auto split = std::stable_partition(entries.begin(), entries.end(), [](const auto& hook) {
            return hook.interruptible == false;
        });

        std::move(split, entries.end(), std::back_inserter(released));
        entries.erase(split, entries.end());
    }

    for (auto& hook : released)
    {
        if (hook.interrupted)
            hook.interrupted();
    }
}

void base_bot::interruptible(bool value)
{
    this->_interruptible = value;
}

bool base_bot::interruptible() const
{
    return this->_interruptible;
}

void base_bot::abort_hooks()
{
    this->assert_thread();

    this->_aborted = true;
    auto hooks     = std::move(this->_hooks);
    this->_hooks.clear();
    this->wake(false);
    for (auto& [_, entries] : hooks)
    {
        for (auto& hook : entries)
        {
            if (hook.aborted)
                hook.aborted();
        }
    }
}

bool base_bot::aborted() const
{
    return this->_aborted;
}

fb::thread* base_bot::thread() const
{
    if (this->_pinned_thread != nullptr)
        return this->_pinned_thread;

    return this->_executor.threads.modular(this->id);
}

void base_bot::pin_thread(fb::thread* t)
{
    this->_pinned_thread = t;
}

fb::thread* base_bot::pinned_thread() const
{
    return this->_pinned_thread;
}

const client_profile& base_bot::profile() const
{
    return this->_profile;
}

void base_bot::profile(const client_profile& value)
{
    this->_profile = value;
}

fb::protocol::CLIENT_VERSION base_bot::client_version() const
{
    return this->_profile.version;
}

fb::protocol::CLIENT_UI_MODE base_bot::ui_mode() const
{
    return this->_profile.ui_mode;
}

std::optional<uint32_t> base_bot::slot() const
{
    return this->_slot;
}

void base_bot::slot(std::optional<uint32_t> value)
{
    this->_slot = value;
}

void base_bot::carry_over(const base_bot& source)
{
    if (this->client_version() != source.client_version())
        fb::logger::fatal("bot transfer changed client version: bot_id={} source_bot_id={} source={} transferred={}",
                          this->id,
                          source.id,
                          fb::protocol::to_string(source.client_version()),
                          fb::protocol::to_string(this->client_version()));

    this->_profile.ui_mode = source.ui_mode();
    this->_slot            = source.slot();
}

void base_bot::add_unread(uint8_t opcode)
{
    auto lock = std::lock_guard(this->_unread_mutex);
    this->_unread_opcodes[opcode]++;
}

std::map<uint8_t, uint32_t> base_bot::unread_opcodes() const
{
    auto lock = std::lock_guard(this->_unread_mutex);
    return this->_unread_opcodes;
}
