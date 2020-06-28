#include "module/acceptor/acceptor.h"

fb::base_acceptor::base_acceptor(boost::asio::io_context& context, uint16_t port) : 
    boost::asio::ip::tcp::acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{}

fb::base_acceptor::~base_acceptor()
{}