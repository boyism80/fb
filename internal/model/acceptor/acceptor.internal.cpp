#include "acceptor.internal.h"

fb::internal::acceptor::acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay) : 
    fb::base::acceptor<fb::base::socket, fb::internal::session>(context, port, accept_delay)
{}

fb::internal::acceptor::~acceptor()
{}

void fb::internal::acceptor::handle_parse(fb::base::socket<fb::internal::session>& socket)
{}

fb::internal::session* fb::internal::acceptor::handle_accepted(fb::base::socket<fb::internal::session>& socket)
{
    return new fb::internal::session();
}

bool fb::internal::acceptor::handle_connected(fb::base::socket<fb::internal::session>& socket)
{
    auto& c = fb::console::get();
    c.puts("%s님이 접속했습니다.", socket.IP().c_str());
    return true;
}

bool fb::internal::acceptor::handle_disconnected(fb::base::socket<fb::internal::session>& socket)
{
    auto& c = fb::console::get();
    c.puts("%s님의 연결이 끊어졌습니다.", socket.IP().c_str());
    return false;
}