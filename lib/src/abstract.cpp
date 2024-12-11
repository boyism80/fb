#include <fb/abstract.h>

using namespace fb;

context::context(boost::asio::io_context& context, uint16_t port) : 
    boost::asio::ip::tcp::acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    _boost_context(context),
    threads(context)
{}

context::operator boost::asio::io_context& () const
{
    return this->_boost_context;
}