#include <fb/abstract.h>

using namespace fb;

context::context(boost::asio::io_context& context, const std::string& name, uint32_t thread_count) :
    _boost_context(context),
    threads(context, thread_count)
{
    console::newline();
    console::puts(console::align_type::center, "The Kingdom of the wind [{}]", name);
    console::newline();
    console::puts(console::align_type::right, "https://github.com/boyism80/fb");
    console::puts(console::align_type::right, "made by cshyeon");
    console::newline();
}

context::operator boost::asio::io_context& () const
{
    return this->_boost_context;
}

acceptable::acceptable(boost::asio::io_context& ctx, const std::string& name, uint32_t thread_count, uint16_t port) : 
    fb::context(ctx, name, thread_count),
    boost::asio::ip::tcp::acceptor(ctx, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
    console::puts("Listen port : {}", port);
}