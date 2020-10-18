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
    static constexpr uint8_t base_size = sizeof(uint16_t);
    auto& in_stream = socket.in_stream();

    while(true)
    {
        try
        {
            if(in_stream.readable_size() < base_size)
                throw std::exception();

            auto size = in_stream.read_u16();
            if(size > in_stream.capacity())
                throw std::exception();

            if(in_stream.readable_size() < size)
                throw std::exception();

            auto cmd = in_stream.read_8();
            auto found = this->_handler_dict.find(cmd);
            if(found == this->_handler_dict.end())
                throw std::exception();

            found->second(socket);

            in_stream.reset();
            in_stream.shift(base_size + size);
            in_stream.flush();
        }
        catch(...)
        {
            break;
        }
    }

    in_stream.reset();
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
    this->_users[request.name] = true;
    return true;
}
