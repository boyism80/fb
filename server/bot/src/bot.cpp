#include <fb/bot/bot.h>

#include <fb/bot/container.h>
#include <fb/bot/controller.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <utility>

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

bool base_bot::process_hooks(uint8_t opcode, fb::protocol::header& header)
{
    this->assert_thread();

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
            return true;
        }
    }

    return false;
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
