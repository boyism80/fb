#include <fb/bot/bot.h>
#include <fb/bot/container.h>
#include <fb/bot/controller.h>

using namespace fb::bot;

base_bot::base_bot(fb::context&                                                      context,
                   base_bot_controller&                                              bot_controller,
                   std::function<async::task<void>(fb::socket<void*>&, fb::stream&)> on_receive,
                   std::function<async::task<void>(fb::socket<void*>&)>              on_closed,
                   uint32_t                                                          id) :
    fb::socket<void*>(context, on_receive, on_closed),
    _context(context),
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

            boost::asio::co_spawn(this->_context.io_context, this->recv(), boost::asio::detached);

            std::ignore = this->thread()->dispatch([this](auto& thread) -> async::task<void> {
                co_await this->_bot_controller.on_bot_connected(*this);
            });
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
    return this->crt().encrypt(out);
}

bool base_bot::on_wrap(fb::stream& out)
{
    return this->crt().wrap(out);
}

bool base_bot::process_hooks(uint8_t cmd, fb::protocol::header& header)
{
    this->assert_thread();

    if (this->_hooks.contains(cmd))
    {
        auto& matched_hooks = this->_hooks.at(cmd);
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

fb::thread* base_bot::thread() const
{
    return this->_context.threads.modular(this->id);
}