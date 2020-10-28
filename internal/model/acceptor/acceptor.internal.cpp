#include "acceptor.internal.h"

fb::internal::acceptor::acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay) : 
    fb::base::acceptor<fb::internal::socket, fb::internal::session>(context, port, accept_delay)
{
    this->bind<fb::protocol::internal::request::subscribe>(std::bind(&acceptor::handle_subscribe, this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::internal::request::transfer>(std::bind(&acceptor::handle_transfer, this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::internal::request::login>(std::bind(&acceptor::handle_login, this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::internal::request::logout>(std::bind(&acceptor::handle_logout, this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::internal::request::whisper>(std::bind(&acceptor::handle_whisper, this, std::placeholders::_1, std::placeholders::_2));
}

fb::internal::acceptor::~acceptor()
{
    for(auto pair : this->_users)
        delete pair.second;
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
    auto name = socket.data()->name;
    this->_subscribers.erase(name);
    c.puts("%s 서버와 연결이 끊어졌습니다.", name.c_str());

    return false;
}

bool fb::internal::acceptor::handle_subscribe(fb::internal::socket<fb::internal::session>& socket, const fb::protocol::internal::request::subscribe& request)
{
    if(this->_subscribers[request.name] != nullptr)
        return false;

    this->_subscribers[request.name] = &socket;

    auto session = socket.data();
    session->name = request.name;

    auto& c = fb::console::get();
    c.puts("%s 서버가 연결되었습니다.", request.name.c_str());
    return true;
}

bool fb::internal::acceptor::handle_transfer(fb::internal::socket<fb::internal::session>& socket, const fb::protocol::internal::request::transfer& request)
{
    try
    {
        auto game_id = fb::internal::table::hosts[request.map];
        if(game_id == nullptr)
            throw fb::internal::transfer_error(fb::protocol::internal::response::NOT_READY);

        auto subscriber = this->_subscribers[*game_id];
        if(subscriber == nullptr)
            throw fb::internal::transfer_error(fb::protocol::internal::response::NOT_READY);

        auto request_id = socket.data()->name;
        if(request_id == "login" || request_id == "gateway")
        {
            auto found = this->_users.find(request.name);
            if(found != this->_users.end())
                throw fb::internal::transfer_error(fb::protocol::internal::response::CONNECTED);
        }

        auto& config = fb::config::get();
        auto session = subscriber->data();
        this->send(socket, fb::protocol::internal::response::transfer(request.name, fb::protocol::internal::response::transfer_code::SUCCESS, request.map, request.x, request.y, config["hosts"][session->name]["ip"].asString(), config["hosts"][session->name]["port"].asInt(), request.fd));
    }
    catch(fb::internal::transfer_error& e)
    {
        this->send(socket, fb::protocol::internal::response::transfer(request.name, e.code,  request.map, request.x, request.y, "", 0, request.fd));
    }
    catch(std::exception& e)
    {
        this->send(socket, fb::protocol::internal::response::transfer(request.name, fb::protocol::internal::response::transfer_code::UNKNOWN,  request.map, request.x, request.y, "", 0, request.fd));
    }
    
    return true;
}

bool fb::internal::acceptor::handle_login(fb::internal::socket<fb::internal::session>& socket, const fb::protocol::internal::request::login& request)
{
    try
    {
        auto game_id = fb::internal::table::hosts[request.map];
        if(game_id == nullptr)
            return true;

        auto subscriber = this->_subscribers[*game_id];
        if(subscriber == nullptr)
            return true;

        auto found = this->_users.find(request.name);
        if(found != this->_users.end())
        {
            delete found->second;
            this->_users.erase(found);
        }

        this->_users[request.name] = new fb::internal::user(*game_id);
    }
    catch(std::exception& e)
    {
        // TODO
    }

    return true;
}

bool fb::internal::acceptor::handle_logout(fb::internal::socket<fb::internal::session>& socket, const fb::protocol::internal::request::logout& request)
{
    if(this->_users[request.name] != nullptr)
        delete this->_users[request.name];

    this->_users.erase(request.name);
    return true;
}

bool fb::internal::acceptor::handle_whisper(fb::internal::socket<fb::internal::session>& socket, const fb::protocol::internal::request::whisper& request)
{
    try
    {
        auto found = this->_users.find(request.to);
        if(found == this->_users.end())
            throw std::exception();

        auto subscriber = this->_subscribers[found->second->game_id];
        if(subscriber == nullptr)
            throw std::exception();

        this->send(socket, fb::protocol::internal::response::whisper(true, request.from, request.to, request.message));

        std::stringstream sstream;
        sstream << request.from << "\" " << request.message;
        subscriber->send(fb::protocol::internal::response::message(request.to, sstream.str(), 0x00));
    }
    catch(std::exception& e)
    {
        this->send(socket, fb::protocol::internal::response::whisper(false, request.from, request.to, request.message));
    }
    return true;
}