#include <model/acceptor/acceptor.login.h>
using namespace fb::login;

acceptor::acceptor(boost::asio::io_context& context, uint16_t port, uint8_t accept_delay) : 
    fb::acceptor<session>(context, port, accept_delay),
    _agreement(fb::config()["agreement"].asString())
{
    // Register event handler
    this->bind<fb::protocol::request::login::login>                   (0x03, std::bind(&acceptor::handle_login,               this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::request::login::agreement>               (0x10, std::bind(&acceptor::handle_agreement,           this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::request::login::account::create>         (0x02, std::bind(&acceptor::handle_create_account,      this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::request::login::account::complete>       (0x04, std::bind(&acceptor::handle_account_complete,    this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::request::login::account::change_pw>      (0x26, std::bind(&acceptor::handle_change_password,     this, std::placeholders::_1, std::placeholders::_2));
}

acceptor::~acceptor()
{}

fb::login::session* acceptor::handle_alloc_session(fb::socket* socket)
{
    return new login::session(socket);
}

bool acceptor::handle_connected(fb::login::session& session)
{
    return true;
}

bool acceptor::handle_disconnected(fb::login::session& session)
{
    std::cout << "disconnection request" << std::endl;
    return false;
}

bool acceptor::handle_agreement(fb::login::session& session, const fb::protocol::request::login::agreement& request)
{
    try
    {
        if(cryptor::validate(request.enc_type, request.enc_key, request.enc_key_size) == false)
            throw std::exception();

        session.crt(request.enc_type, request.enc_key);
        this->send(session, this->_agreement);
        return true;
    }
    catch(std::exception& e)
    {
        return false;
    }
}

bool acceptor::handle_create_account(fb::login::session& session, const fb::protocol::request::login::account::create& request)
{
    try
    {
        this->_auth_service.create_account(request.id, request.pw);
        this->send(session, response::login::message("", 0x00));

        // 일단 아이디 선점해야함
        session.created_id = request.id;
        return true;
    }
    catch(login_exception& e)
    {
        this->send(session, response::login::message(e.what(), e.type()));
        return true;
    }
    catch(std::exception& e)
    {
        return false;
    }
}

bool acceptor::handle_account_complete(fb::login::session& session, const fb::protocol::request::login::account::complete& request)
{
    try
    {
        if(session.created_id.empty())
            throw std::exception();

        this->_auth_service.init_account(session.created_id, request.hair, request.sex, request.nation, request.creature);
        this->send(session, response::login::message(message::SUCCESS_REGISTER_ACCOUNT, 0x00));
        session.created_id.clear();
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

bool acceptor::handle_login(fb::login::session& session, const fb::protocol::request::login::login& request)
{
    try
    {
        this->_auth_service.login(request.id, request.pw);
        this->send(session, response::login::message("", 0x00));

        fb::ostream         parameter;
        parameter.write(request.id);
        this->transfer(session, fb::config()["game"]["ip"].asString(), fb::config()["game"]["port"].asInt(), parameter);
        return true;
    }
    catch(login_exception& e)
    {
        this->send(session, response::login::message(e.what(), e.type()));
        return true;
    }
    catch(std::exception& e)
    {
        return false;
    }
}

bool acceptor::handle_change_password(fb::login::session& session, const fb::protocol::request::login::account::change_pw& request)
{
    try
    {
        this->_auth_service.change_pw(request.name, request.pw, request.new_pw, request.birthday);
        this->send(session, response::login::message(message::SUCCESS_CHANGE_PASSWORD, 0x00));
        return true;
    }
    catch(login_exception& e)
    {
        this->send(session, response::login::message(e.what(), e.type()));
        return true;
    }
    catch(std::exception& e)
    {
        return false;
    }
}
