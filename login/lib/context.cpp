#include <fb/login/context.h>

fb::login::context::context(boost::asio::io_context& context, uint16_t port) : 
    fb::acceptor<fb::login::session>(context, port),
    _db(*this, 4),
    _auth_service(_db)
{
    // Register event handler
    this->bind<fb::protocol::login::request::login>                   (std::bind(&context::handle_login,               this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::login::request::agreement>               (std::bind(&context::handle_agreement,           this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::login::request::account::create>         (std::bind(&context::handle_create_account,      this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::login::request::account::complete>       (std::bind(&context::handle_account_complete,    this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::login::request::account::change_pw>      (std::bind(&context::handle_change_password,     this, std::placeholders::_1, std::placeholders::_2));

    this->bind<fb::protocol::internal::response::transfer>();
    this->bind<fb::protocol::internal::response::shutdown>            (std::bind(&context::handle_in_shutdown,         this, std::placeholders::_1, std::placeholders::_2));
}

fb::login::context::~context()
{ }

bool fb::login::context::is_decrypt(uint8_t cmd) const
{
    switch(cmd)
    {
    case 0x10:
        return false;

    default:
        return true;
    }
}

fb::login::session* fb::login::context::handle_accepted(fb::socket<fb::login::session>& socket)
{
    auto session = std::make_unique<fb::login::session>();
    auto ptr = session.get();
    this->_sessions.push_back(std::move(session));
    return ptr;
}

bool fb::login::context::handle_connected(fb::socket<fb::login::session>& socket)
{
    fb::logger::info("%s님이 접속했습니다.", socket.IP().c_str());
    return true;
}

bool fb::login::context::handle_disconnected(fb::socket<fb::login::session>& socket)
{
    fb::logger::info("%s님의 연결이 끊어졌습니다.", socket.IP().c_str());
    return false;
}

void fb::login::context::handle_internal_connected()
{
    fb::acceptor<fb::login::session>::handle_internal_connected();

    auto& config = fb::config::get();
    this->_internal->send(fb::protocol::internal::request::subscribe(config["id"].asString(), fb::protocol::internal::services::LOGIN, 0xFF));
}

fb::task<bool> fb::login::context::handle_in_shutdown(fb::internal::socket<>& socket, const fb::protocol::internal::response::shutdown& response)
{
    this->exit();
    co_return true;
}

fb::task<bool> fb::login::context::handle_agreement(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::agreement& request)
{
    try
    {
        if(cryptor::validate(request.enc_type, request.enc_key, request.enc_key_size) == false)
            throw std::exception();

        socket.crt(request.enc_type, request.enc_key);
        socket.send(this->_agreement);
        co_return true;
    }
    catch(std::exception&)
    {
        co_return false;
    }
}

fb::task<bool> fb::login::context::handle_create_account(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::account::create& request)
{
    auto fd = socket.fd();

    try
    {
        auto id = request.id;
        auto pw = request.pw;
        co_await this->_auth_service.create_account(id, pw);
        if (this->sockets.contains(fd) == false)
            co_return false;

        this->send(socket, fb::protocol::login::response::message("", 0x00));
        auto session = socket.data();
        session->created_id = id;
    }
    catch(login_exception& e)
    {
        if (this->sockets.contains(fd) == false)
            co_return false;

        socket.send(fb::protocol::login::response::message(e.what(), e.type()));
    }
    catch(std::exception& e)
    {
        if (this->sockets.contains(fd) == false)
            co_return false;

        socket.send(fb::protocol::login::response::message(e.what(), 0x0E));
    }
    co_return true;
}

fb::task<bool> fb::login::context::handle_account_complete(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::account::complete& request)
{
    try
    {
        auto session = socket.data();
        if(session->created_id.empty())
            throw std::exception();

        auto fd = socket.fd();
        co_await this->_auth_service.init_account(session->created_id, request.hair, request.sex, request.nation, request.creature);
        if (this->sockets.contains(fd) == false)
            co_return false;

        socket.send(fb::protocol::login::response::message(fb::login::message::account::SUCCESS_REGISTER_ACCOUNT, 0x00));
        session->created_id.clear();
        co_return true;
    }
    catch(login_exception&)
    {
        co_return true;
    }
    catch(std::exception&)
    {
        co_return false;
    }
}

fb::task<bool> fb::login::context::handle_login(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::login& request)
{
    auto delay = fb::config::get()["transfer delay"].asInt();
    co_await this->sleep(std::chrono::seconds(delay));

    auto fd = socket.fd();
    try
    {
        auto id = request.id;
        auto pw = request.pw;
        auto map = co_await this->_auth_service.login(id, pw);
        if (this->sockets.contains(fd) == false)
            co_return false;

        auto response = co_await this->request<fb::protocol::internal::response::transfer>(fb::protocol::internal::request::transfer(id, fb::protocol::internal::services::LOGIN, fb::protocol::internal::services::GAME, map, fd));
        if (this->sockets.contains(fd) == false)
            co_return false;

        if(response.code == fb::protocol::internal::response::transfer_code::CONNECTED)
            throw id_exception("이미 접속중입니다.");

        if(response.code != fb::protocol::internal::response::transfer_code::SUCCESS)
            throw id_exception("비바람이 휘몰아치고 있습니다.");

        socket.send(fb::protocol::login::response::message("", 0x00));
        fb::ostream         parameter;
        parameter.write(response.name);
        parameter.write_u8(0);
        this->transfer(socket, response.ip, response.port, fb::protocol::internal::services::LOGIN, parameter);
    }
    catch(login_exception& e)
    {
        if (this->sockets.contains(fd) == false)
            co_return false;

        socket.send(fb::protocol::login::response::message(e.what(), e.type()));
    }
    catch (boost::system::error_code& e)
    {
        if (this->sockets.contains(fd) == false)
            co_return false;

        auto sstream = std::stringstream();
        sstream << e.message() << "(" << e.value() << ")";
        socket.send(fb::protocol::login::response::message(sstream.str(), 0x0E));
    }
    catch (std::exception& e)
    {
        if (this->sockets.contains(fd) == false)
            co_return false;

        socket.send(fb::protocol::login::response::message(e.what(), 0x0E));
    }

    co_return true;
}

fb::task<bool> fb::login::context::handle_change_password(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::account::change_pw& request)
{
    auto delay = fb::config::get()["transfer delay"].asInt();
    co_await this->sleep(std::chrono::seconds(delay));

    auto fd = socket.fd();
    try
    {
        co_await this->_auth_service.change_pw(request.name, request.pw, request.new_pw, request.birthday);
        if (this->sockets.contains(fd) == false)
            co_return false;

        socket.send(fb::protocol::login::response::message((fb::login::message::account::SUCCESS_CHANGE_PASSWORD), 0x00));
    }
    catch(login_exception& e)
    {
        if (this->sockets.contains(fd) == false)
            co_return false;

        socket.send(fb::protocol::login::response::message(e.what(), e.type()));
    }
    catch(std::exception& e)
    {
        if (this->sockets.contains(fd) == false)
            co_return false;

        socket.send(fb::protocol::login::response::message(e.what(), 0x0E));
    }

    co_return true;
}
