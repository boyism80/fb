#include "acceptor.login.h"

fb::login::acceptor::acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay, const INTERNAL_CONNECTION& internal_connection) : 
    fb::acceptor<fb::login::session>(context, port, accept_delay, internal_connection),
    _agreement(CP949(fb::config::get()["agreement"].asString(), PLATFORM::Both))
{
    // Register event handler
    this->bind<fb::protocol::login::request::login>                   (0x03, std::bind(&acceptor::handle_login,               this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::login::request::agreement>               (0x10, std::bind(&acceptor::handle_agreement,           this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::login::request::account::create>         (0x02, std::bind(&acceptor::handle_create_account,      this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::login::request::account::complete>       (0x04, std::bind(&acceptor::handle_account_complete,    this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::login::request::account::change_pw>      (0x26, std::bind(&acceptor::handle_change_password,     this, std::placeholders::_1, std::placeholders::_2));

    this->bind<fb::protocol::internal::response::login>               (std::bind(&acceptor::handle_private_login,             this, std::placeholders::_1, std::placeholders::_2));
}

fb::login::acceptor::~acceptor()
{}

fb::login::session* fb::login::acceptor::handle_accepted(fb::socket<fb::login::session>& socket)
{
    return new fb::login::session();
}

bool fb::login::acceptor::handle_connected(fb::socket<fb::login::session>& socket)
{
    auto& c = fb::console::get();
    c.puts("%s님이 접속했습니다.", socket.IP().c_str());
    return true;
}

bool fb::login::acceptor::handle_disconnected(fb::socket<fb::login::session>& socket)
{
    auto& c = fb::console::get();
    c.puts("%s님의 연결이 끊어졌습니다.", socket.IP().c_str());
    return false;
}

bool fb::login::acceptor::handle_private_login(fb::internal::socket<>& socket, const fb::protocol::internal::response::login& response)
{
    auto client = this->sockets[response.fd];

    try
    {
        if(client == nullptr)
            return true;

        if(response.status == false)
            throw id_exception("이미 접속중입니다.");

        client->send(fb::protocol::login::response::message("", 0x00));
        fb::ostream         parameter;
        parameter.write(response.name);
        this->transfer(*client, fb::config::get()["game"]["ip"].asString(), fb::config::get()["game"]["port"].asInt(), parameter);
    }
    catch(login_exception& e)
    {
        client->send(fb::protocol::login::response::message(e.what(), e.type()));
    }

    return true;
}

bool fb::login::acceptor::handle_agreement(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::agreement& request)
{
    try
    {
        if(cryptor::validate(request.enc_type, request.enc_key, request.enc_key_size) == false)
            throw std::exception();

        socket.crt(request.enc_type, request.enc_key);
        socket.send(this->_agreement);
        return true;
    }
    catch(std::exception& e)
    {
        return false;
    }
}

bool fb::login::acceptor::handle_create_account(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::account::create& request)
{
    try
    {
        this->_auth_service.create_account(request.id, request.pw);
        this->send(socket, fb::protocol::login::response::message("", 0x00));

        // 일단 아이디 선점해야함

        auto session = socket.data();
        session->created_id = request.id;
        return true;
    }
    catch(login_exception& e)
    {
        socket.send(fb::protocol::login::response::message(e.what(), e.type()));
        return true;
    }
    catch(std::exception& e)
    {
        return false;
    }
}

bool fb::login::acceptor::handle_account_complete(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::account::complete& request)
{
    try
    {
        auto session = socket.data();

        if(session->created_id.empty())
            throw std::exception();

        this->_auth_service.init_account(session->created_id, request.hair, request.sex, request.nation, request.creature);
        socket.send(fb::protocol::login::response::message(fb::login::message::account::SUCCESS_REGISTER_ACCOUNT, 0x00));
        session->created_id.clear();
        return true;
    }
    catch(login_exception& e)
    {
        return true;
    }
    catch(std::exception& e)
    {
        return false;
    }
}

bool fb::login::acceptor::handle_login(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::login& request)
{
    try
    {
        this->_auth_service.login(request.id, request.pw);
        this->_internal->send(fb::protocol::internal::request::login(request.id, socket.native_handle()));
        return true;
    }
    catch(login_exception& e)
    {
        socket.send(fb::protocol::login::response::message(e.what(), e.type()));
        return true;
    }
    catch(std::exception& e)
    {
        return false;
    }
}

bool fb::login::acceptor::handle_change_password(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::account::change_pw& request)
{
    try
    {
        this->_auth_service.change_pw(request.name, request.pw, request.new_pw, request.birthday);
        socket.send(fb::protocol::login::response::message((fb::login::message::account::SUCCESS_CHANGE_PASSWORD), 0x00));
        return true;
    }
    catch(login_exception& e)
    {
        socket.send(fb::protocol::login::response::message(e.what(), e.type()));
        return true;
    }
    catch(std::exception& e)
    {
        return false;
    }
}
