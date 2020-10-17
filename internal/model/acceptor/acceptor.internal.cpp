#include "acceptor.internal.h"
using namespace fb::internal;

acceptor::acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay) : 
    fb::acceptor<session>(context, port, accept_delay)
{}

acceptor::~acceptor()
{}

fb::internal::session* acceptor::handle_accepted(fb::socket<fb::internal::session>& socket)
{
    return new fb::internal::session();
}

bool acceptor::handle_connected(fb::socket<fb::internal::session>& socket)
{
    auto& c = fb::console::get();
    c.puts("%s님이 접속했습니다.", socket.IP().c_str());
    return true;
}

bool acceptor::handle_disconnected(fb::socket<fb::internal::session>& socket)
{
    auto& c = fb::console::get();
    c.puts("%s님의 연결이 끊어졌습니다.", socket.IP().c_str());
    return false;
}