#include <fb/abstract.h>

using namespace fb;

context::context(boost::asio::io_context& context, const std::string& name, uint32_t thread_count) :
    io_context(context),
    threads(*this, thread_count)
{
    static auto flag = std::once_flag{};
    std::call_once(flag, [name] {
        auto mode = console::get_mode();
        console::set_mode(console::mode::plain);
        console::puts("");
        console::puts(console::align_type::center, "The Kingdom of the wind [{}]", name);
        console::puts("");
        console::puts(console::align_type::right, "https://github.com/boyism80/fb");
        console::puts(console::align_type::right, "made by cshyeon");
        console::puts("");
        console::set_mode(mode);
    });
}

context::operator boost::asio::io_context& () const
{
    return this->io_context;
}

void context::exit()
{
    if (this->_running == false)
        return;

    this->_running = false;
    for (auto& timer : this->_timers)
    {
        timer->cancel();
    }

    this->threads.exit();
    this->io_context.stop();
}

acceptable::acceptable(boost::asio::io_context& ctx, const std::string& name, uint32_t thread_count, uint16_t port) :
    fb::context(ctx, name, thread_count),
    boost::asio::ip::tcp::acceptor(ctx, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
    static auto flag = std::once_flag{};
    std::call_once(flag, [port] {
        console::puts("Listen port : {}", port);
    });
}