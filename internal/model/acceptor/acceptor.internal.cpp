#include "acceptor.internal.h"

fb::internal::acceptor::acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay) : 
    fb::base::acceptor<fb::internal::socket, fb::internal::session>(context, port, accept_delay)
{
    this->bind<fb::protocol::internal::request::subscribe>(std::bind(&acceptor::handle_subscribe, this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::internal::request::login>(std::bind(&acceptor::handle_login, this, std::placeholders::_1, std::placeholders::_2));
}

fb::internal::acceptor::~acceptor()
{
    
}

void fb::internal::acceptor::handle_parse(fb::internal::socket<fb::internal::session>& socket)
{
    fb::internal::parse<fb::internal::session, std::map<uint8_t, fb::internal::acceptor::handler>>(socket, this->_handler_dict);
}

fb::internal::session* fb::internal::acceptor::handle_accepted(fb::internal::socket<fb::internal::session>& socket)
{
    return new fb::internal::session();
}

bool fb::internal::acceptor::handle_connected(fb::internal::socket<fb::internal::session>& socket)
{
    return true;
}

bool fb::internal::acceptor::handle_disconnected(fb::internal::socket<fb::internal::session>& socket)
{
    auto& c = fb::console::get();
    
    for(auto x : socket.data()->subscribed)
    {
        this->_subscribers.erase(x);
        c.puts("%s 서버와 연결이 끊어졌습니다.", x.c_str());
    }

    return false;
}

bool fb::internal::acceptor::handle_subscribe(fb::internal::socket<fb::internal::session>& socket, const fb::protocol::internal::request::subscribe& request)
{
    if(this->_subscribers[request.name] != nullptr)
        return false;

    this->_subscribers[request.name] = &socket;

    auto session = socket.data();
    session->subscribed.push_back(request.name);

    auto& c = fb::console::get();
    c.puts("%s 서버가 연결되었습니다.", request.name.c_str());
    return true;
}

bool fb::internal::acceptor::handle_login(fb::internal::socket<fb::internal::session>& socket, const fb::protocol::internal::request::login& request)
{
    auto found = this->_users.find(request.name);
    if(found == this->_users.end())
    {
        this->send(socket, fb::protocol::internal::response::login(request.name, true, "192.168.0.100", 2004, request.fd));
        this->_users[request.name] = new fb::internal::user("game1");
    }
    else
    {
        this->send(socket, fb::protocol::internal::response::login(request.name, false, "", 0, request.fd));
        delete this->_users[request.name];
        this->_users[request.name] = nullptr;
    }
    return true;
}
