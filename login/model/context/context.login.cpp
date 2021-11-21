#include "context.login.h"

fb::login::context::context(boost::asio::io_context& context, uint16_t port, std::chrono::seconds delay, const INTERNAL_CONNECTION& internal_connection) : 
    fb::acceptor<fb::login::session>(context, port, delay, internal_connection, fb::config::get()["thread"].isNull() ? 0xFF : fb::config::get()["thread"].asInt())
{
    // Register event handler
    this->bind<fb::protocol::login::request::login>                   (0x03, std::bind(&context::handle_login,               this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::login::request::agreement>               (0x10, std::bind(&context::handle_agreement,           this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::login::request::account::create>         (0x02, std::bind(&context::handle_create_account,      this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::login::request::account::complete>       (0x04, std::bind(&context::handle_account_complete,    this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::login::request::account::change_pw>      (0x26, std::bind(&context::handle_change_password,     this, std::placeholders::_1, std::placeholders::_2));

    this->bind<fb::protocol::internal::response::transfer>            (std::bind(&context::handle_in_transfer,               this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::internal::response::shutdown>            (std::bind(&context::handle_in_shutdown,               this, std::placeholders::_1, std::placeholders::_2));
}

fb::login::context::~context()
{ }

fb::login::session* fb::login::context::handle_accepted(fb::socket<fb::login::session>& socket)
{
    auto session = std::make_unique<fb::login::session>();
    auto ptr = session.get();
    this->_sessions.push_back(std::move(session));
    return ptr;
}

bool fb::login::context::handle_connected(fb::socket<fb::login::session>& socket)
{
    auto& c = fb::console::get();
    c.puts("%s님이 접속했습니다.", socket.IP().c_str());
    return true;
}

bool fb::login::context::handle_disconnected(fb::socket<fb::login::session>& socket)
{
    auto& c = fb::console::get();
    c.puts("%s님의 연결이 끊어졌습니다.", socket.IP().c_str());
    return false;
}

bool fb::login::context::handle_in_transfer(fb::internal::socket<>& socket, const fb::protocol::internal::response::transfer& response)
{
    auto client = this->sockets[response.fd];

    try
    {
        if(client == nullptr)
            return true;

        if(response.code == fb::protocol::internal::response::transfer_code::CONNECTED)
            throw id_exception("이미 접속중입니다.");

        if(response.code != fb::protocol::internal::response::transfer_code::SUCCESS)
            throw id_exception("비바람이 휘몰아치고 있습니다.");

        client->send(fb::protocol::login::response::message("", 0x00));
        fb::ostream         parameter;
        parameter.write(response.name);
        this->transfer(*client, response.ip, response.port, fb::protocol::internal::services::LOGIN, parameter);
    }
    catch(login_exception& e)
    {
        client->send(fb::protocol::login::response::message(e.what(), e.type()));
    }

    return true;
}

bool fb::login::context::handle_in_shutdown(fb::internal::socket<>& socket, const fb::protocol::internal::response::shutdown& response)
{
    this->exit();
    return true;
}

bool fb::login::context::handle_agreement(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::agreement& request)
{
    try
    {
        if(cryptor::validate(request.enc_type, request.enc_key, request.enc_key_size) == false)
            throw std::exception();

        socket.crt(request.enc_type, request.enc_key);
        socket.send(this->_agreement);
        return true;
    }
    catch(std::exception&)
    {
        return false;
    }
}

bool fb::login::context::handle_create_account(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::account::create& request)
{
    auto& id = request.id;
    this->_auth_service.create_account
    (
        request.id, request.pw,
        [this, &socket, id] (const auto& name)
        {
            this->send(socket, fb::protocol::login::response::message("", 0x00));

            // 일단 아이디 선점해야함
            auto session = socket.data();
            session->created_id = id;
        },
        [&socket] (const auto& name, const auto& e)
        {
            socket.send(fb::protocol::login::response::message(e.what(), e.type()));
        }
    );
    return true;
}

bool fb::login::context::handle_account_complete(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::account::complete& request)
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
    catch(login_exception&)
    {
        return true;
    }
    catch(std::exception&)
    {
        return false;
    }
}

bool fb::login::context::handle_login(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::login& request)
{
    const auto& id = request.id;
    const auto& pw = request.pw;
    auto delay = fb::config::get()["transfer delay"].asInt();

    this->threads().dispatch
    (
        [this, id, pw, &socket] ()
        {
            this->_auth_service.login
            (
                id, pw,
                [this, id, &socket] (auto map)
                {
                    this->_internal->send(fb::protocol::internal::request::transfer(id, fb::protocol::internal::services::LOGIN, fb::protocol::internal::services::GAME, map, socket.native_handle()));
                },
                [this, &socket] (const auto& e)
                {
                    socket.send(fb::protocol::login::response::message(e.what(), e.type()));
                }
            );
        },
        std::chrono::seconds(delay)
    );
    return true;
}

bool fb::login::context::handle_change_password(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::account::change_pw& request)
{
    const auto& id = request.name;
    const auto& pw = request.pw;
    const auto& new_pw = request.new_pw;
    const auto birthday = request.birthday;
    auto delay = fb::config::get()["transfer delay"].asInt();

    this->threads().dispatch
    (
        [this, &socket, id, pw, new_pw, birthday] ()
        {
            this->_auth_service.change_pw
            (
                id, pw, new_pw, birthday,
                [this, &socket] ()
                {
                    socket.send(fb::protocol::login::response::message((fb::login::message::account::SUCCESS_CHANGE_PASSWORD), 0x00));
                },
                [this, &socket] (const auto& e)
                {
                    socket.send(fb::protocol::login::response::message(e.what(), e.type()));
                }
            );
        },
        std::chrono::seconds(delay)
    );

    return true;
}
