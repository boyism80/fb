#include <fb/abstract.h>

using namespace fb;

context::context(boost::asio::io_context& context, const std::string& name, uint16_t port) :
    boost::asio::ip::tcp::acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    _boost_context(context),
    threads(context)
{
    console::newline();
    console::puts(console::align_type::center, "The Kingdom of the wind [{}]", name);
    console::newline();
    console::puts(console::align_type::right, "https://github.com/boyism80/fb");
    console::puts(console::align_type::right, "made by cshyeon");
    console::newline();
    console::puts("Listen port : {}", port);
}

context::operator boost::asio::io_context& () const
{
    return this->_boost_context;
}