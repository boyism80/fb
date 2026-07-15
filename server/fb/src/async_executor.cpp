#include <fb/async_executor.h>
#include <fb/execution_context.h>

using namespace fb;

async_executor::async_executor(boost::asio::io_context& context, std::string_view name, uint32_t thread_count) :
    io_context(context),
    threads(*this, thread_count)
{
    execution_context::install_propagation_hooks(*this);
    static auto flag = std::once_flag{};
    std::call_once(flag, [name] {
        auto name_str = std::string(name);
        auto mode     = console::get_mode();
        console::set_mode(console::mode::plain);
        console::puts("");
        console::puts(console::align_type::center, "The Kingdom of the wind [{}]", name_str);
        console::puts("");
        console::puts(console::align_type::right, "https://github.com/boyism80/fb");
        console::puts(console::align_type::right, "made by cshyeon");
        console::puts("");
        console::set_mode(mode);
    });
}

async_executor::operator boost::asio::io_context& () const
{
    return this->io_context;
}

void async_executor::exit()
{
    if (this->_running == false)
        return;

    this->_running = false;
    for (auto& timer : this->_timers)
    {
        timer->cancel();
    }

    // Stop IO before marking logic threads as exiting so in-flight socket close
    // handlers can still be resumed via thread queue while IO unwinds.
    this->io_context.stop();
    this->threads.exit();
}
