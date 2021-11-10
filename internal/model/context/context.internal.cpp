#include "context.internal.h"

fb::internal::context::context(boost::asio::io_context& context, uint16_t port, std::chrono::seconds delay) : 
    fb::base::acceptor<fb::internal::socket, fb::internal::session>(context, port, delay, fb::config::get()["thread"].isNull() ? 0xFF : fb::config::get()["thread"].asInt())
{
    this->bind<fb::protocol::internal::request::subscribe>(std::bind(&context::handle_subscribe, this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::internal::request::transfer>(std::bind(&context::handle_transfer, this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::internal::request::login>(std::bind(&context::handle_login, this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::internal::request::logout>(std::bind(&context::handle_logout, this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::internal::request::whisper>(std::bind(&context::handle_whisper, this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::internal::request::shutdown>(std::bind(&context::handle_shutdown, this, std::placeholders::_1, std::placeholders::_2));
}

fb::internal::context::~context()
{ }

fb::internal::context::service* fb::internal::context::get(fb::protocol::internal::services type, uint8_t group)
{
    switch(type)
    {
    case fb::protocol::internal::services::GATEWAY:
        return this->_gateway;

    case fb::protocol::internal::services::LOGIN:
        return this->_login;

    case fb::protocol::internal::services::GAME:
        return this->_games[group];

    default:
        return nullptr;
    }
}

bool fb::internal::context::handle_parse(fb::internal::socket<fb::internal::session>& socket, const std::function<bool(fb::internal::socket<fb::internal::session>&)>& callback)
{
    return fb::internal::parse<fb::internal::session, std::map<uint8_t, fb::internal::context::handler>>(socket, this->_handler_dict);
}

fb::internal::session* fb::internal::context::handle_accepted(fb::internal::socket<fb::internal::session>& socket)
{
    auto session = new fb::internal::session();
    this->_sessions.push_back(unique_session(session));
    return session;
}

bool fb::internal::context::handle_connected(fb::internal::socket<fb::internal::session>& socket)
{
    return true;
}

bool fb::internal::context::handle_disconnected(fb::internal::socket<fb::internal::session>& socket)
{
    auto& c = fb::console::get();
    auto subscriber = socket.data();
    c.puts("%s 서버와 연결이 끊어졌습니다.", subscriber->name.c_str());

    switch(subscriber->service)
    {
    case fb::protocol::internal::services::GATEWAY:
    {
        this->_gateway = nullptr;
    } break;

    case fb::protocol::internal::services::LOGIN:
    {
        this->_login = nullptr;
    } break;

    case fb::protocol::internal::services::GAME:
    {
        this->_games.erase(subscriber->group);
    } break;
    }
    
    return false;
}

bool fb::internal::context::handle_subscribe(fb::internal::socket<fb::internal::session>& socket, const fb::protocol::internal::request::subscribe& request)
{
    auto session = socket.data();
    session->name = request.name;
    session->service = request.service;
    session->group = request.group;

    switch(request.service)
    {
    case fb::protocol::internal::services::GATEWAY:
    {
        if(this->_gateway != nullptr)
            return false;

        this->_gateway = &socket;
    } break;

    case fb::protocol::internal::services::LOGIN:
    {
        if(this->_login != nullptr)
            return false;

        this->_login = &socket;
    } break;

    case fb::protocol::internal::services::GAME:
    {
        if(this->_games[request.group] != nullptr)
            return false;

        this->_games[request.group] = &socket;
    } break;

    default:
        return false;
    }

    auto& c = fb::console::get();
    c.puts("%s 서버가 연결되었습니다.", request.name.c_str());
    return true;
}

bool fb::internal::context::handle_transfer(fb::internal::socket<fb::internal::session>& socket, const fb::protocol::internal::request::transfer& request)
{
    auto group = std::optional<uint8_t>(0xFF);

    try
    {
        switch(request.to)
        {
        case fb::protocol::internal::services::GAME:
        {
            group = fb::internal::table::hosts[request.map];
            if(group.has_value() == false)
                throw fb::internal::transfer_error(fb::protocol::internal::response::transfer_code::UNKNOWN);

            if(this->_games[*group] == nullptr)
                throw fb::internal::transfer_error(fb::protocol::internal::response::transfer_code::NOT_READY);

            if(request.from == fb::protocol::internal::services::LOGIN)
            {
                auto found = this->_users.find(request.name);
                if(found != this->_users.end())
                    throw fb::internal::transfer_error(fb::protocol::internal::response::transfer_code::CONNECTED);
            }
        } break;

        default:
            return false;
        }

        auto& config = fb::config::get();
        auto session = this->get(request.to, group.value())->data();
        this->send(socket, fb::protocol::internal::response::transfer(request.name, fb::protocol::internal::response::transfer_code::SUCCESS, request.map, request.x, request.y, config["hosts"][session->name]["ip"].asString(), config["hosts"][session->name]["port"].asInt(), request.fd, request.from));
    }
    catch(fb::internal::transfer_error& e)
    {
        if(e.code == fb::protocol::internal::response::transfer_code::CONNECTED)
        {
            this->_users.erase(request.name);

            // send disconnection message
            auto game_id = fb::internal::table::hosts[request.map];
            auto subscriber = this->get(request.to, group.value());
            subscriber->send(fb::protocol::internal::response::logout(request.name));
        }

        this->send(socket, fb::protocol::internal::response::transfer(request.name, e.code,  request.map, request.x, request.y, "", 0, request.fd, request.from));
    }
    catch(std::exception& e)
    {
        this->send(socket, fb::protocol::internal::response::transfer(request.name, fb::protocol::internal::response::transfer_code::UNKNOWN,  request.map, request.x, request.y, "", 0, request.fd, request.from));
    }
    
    return true;
}

bool fb::internal::context::handle_login(fb::internal::socket<fb::internal::session>& socket, const fb::protocol::internal::request::login& request)
{
    try
    {
        auto group = fb::internal::table::hosts[request.map];
        if(group.has_value() == false)
            return true;

        auto subscriber = this->get(fb::protocol::internal::services::GAME, *group);
        if(subscriber == nullptr)
            return true;

        auto found = this->_users.find(request.name);
        if(found != this->_users.end())
        {
            this->_users.erase(found);
        }

        // TODO: unique_ptr로 관리
        this->_users.insert(std::make_pair(request.name, std::make_unique<fb::internal::user>(*group)));
    }
    catch(std::exception& e)
    {
        // TODO
    }

    return true;
}

bool fb::internal::context::handle_logout(fb::internal::socket<fb::internal::session>& socket, const fb::protocol::internal::request::logout& request)
{
    auto found = this->_users.find(request.name);
    if(found != this->_users.end())
        this->_users.erase(found);
    
    return true;
}

bool fb::internal::context::handle_whisper(fb::internal::socket<fb::internal::session>& socket, const fb::protocol::internal::request::whisper& request)
{
    try
    {
        auto found = this->_users.find(request.to);
        if(found == this->_users.end())
            throw std::exception();

        auto subscriber = this->_games[found->second->group];
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

bool fb::internal::context::handle_shutdown(fb::internal::socket<fb::internal::session>& socket, const fb::protocol::internal::request::shutdown& request)
{
    if(this->_gateway != nullptr)
        this->send(*this->_gateway, fb::protocol::internal::response::shutdown());

    if(this->_login != nullptr)
        this->send(*this->_login, fb::protocol::internal::response::shutdown());

    for(auto& x : this->_games)
        this->send(*x.second, fb::protocol::internal::response::shutdown());

    this->exit();
    return true;
}