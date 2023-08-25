#include <fb/login/context.h>

fb::login::context::context(boost::asio::io_context& context, uint16_t port, std::chrono::seconds delay) : 
    fb::acceptor<fb::login::session>(context, port, delay),
    _db(4),
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

fb::task fb::login::context::create_account(fb::socket<fb::login::session>& socket, std::string id, std::string pw)
{
    try
    {
        co_await this->_auth_service.create_account(id, pw);
        this->send(socket, fb::protocol::login::response::message("", 0x00));
        auto session = socket.data();
        session->created_id = id;
    }
    catch(login_exception& e)
    {
        socket.send(fb::protocol::login::response::message(e.what(), e.type()));
    }
}

fb::task fb::login::context::login(fb::socket<fb::login::session>& socket, std::string id, std::string pw)
{
    auto fd = socket.fd();
    try
    {
        auto map = co_await this->_auth_service.login(id, pw);

        // 하나의 _internal 소켓에서 응답이 오기전에 request를 계속 호출하면서 발생하는 문제
        // 코루틴으로 하면 안될거같다..
        auto response = co_await this->_internal->request<fb::protocol::internal::response::transfer>(fb::protocol::internal::request::transfer(id, fb::protocol::internal::services::LOGIN, fb::protocol::internal::services::GAME, map, fd));

        auto client = this->sockets[fd];
        if(client == nullptr)
            co_return;

        if(response.code == fb::protocol::internal::response::transfer_code::CONNECTED)
            throw id_exception("이미 접속중입니다.");

        if(response.code != fb::protocol::internal::response::transfer_code::SUCCESS)
            throw id_exception("비바람이 휘몰아치고 있습니다.");

        client->send(fb::protocol::login::response::message("", 0x00));
        fb::ostream         parameter;
        parameter.write(response.name);
        parameter.write_u8(0);
        this->transfer(*client, response.ip, response.port, fb::protocol::internal::services::LOGIN, parameter);
    }
    catch(login_exception& e)
    {
        auto client = this->sockets[fd];
        if (client == nullptr)
            co_return;

        client->send(fb::protocol::login::response::message(e.what(), e.type()));
    }
    catch (std::exception& e)
    {
        auto client = this->sockets[fd];
        if (client == nullptr)
            co_return;

        client->send(fb::protocol::login::response::message("서버가 원활하지 않습니다.", 0x0E));
    }
}

fb::task fb::login::context::change_pw(fb::socket<fb::login::session>& socket, std::string id, std::string pw, std::string new_pw, uint32_t birthday)
{
    try
    {
        co_await this->_auth_service.change_pw(id, pw, new_pw, birthday);
        socket.send(fb::protocol::login::response::message((fb::login::message::account::SUCCESS_CHANGE_PASSWORD), 0x00));
    }
    catch(login_exception& e)
    {
        socket.send(fb::protocol::login::response::message(e.what(), e.type()));
    }
}

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
    this->create_account(socket, request.id, request.pw);
    return true;
}

bool fb::login::context::handle_account_complete(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::account::complete& request)
{
    try
    {
        auto session = socket.data();

        if(session->created_id.empty())
            throw std::exception();

        static auto fn = [this] (fb::socket<fb::login::session>& socket, uint8_t look, uint8_t sex, uint8_t nation, uint8_t creature) -> task
        {
            auto session = socket.data();
            co_await this->_auth_service.init_account(session->created_id, look, sex, nation, creature);
            socket.send(fb::protocol::login::response::message(fb::login::message::account::SUCCESS_REGISTER_ACCOUNT, 0x00));
            session->created_id.clear();
        };
        fn(socket, request.hair, request.sex, request.nation, request.creature);
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
    auto delay = fb::config::get()["transfer delay"].asInt();
    auto fn = [this, id = request.id, pw = request.pw, &socket]
    {
        this->login(socket, id, pw);
    };

    this->threads().dispatch(fn, std::chrono::seconds(delay));
    return true;
}

bool fb::login::context::handle_change_password(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::account::change_pw& request)
{
    auto delay = fb::config::get()["transfer delay"].asInt();

    auto fn = [this, &socket, id = request.name, pw = request.pw, new_pw = request.new_pw, birthday = request.birthday]
    {
        this->change_pw(socket, id, pw, new_pw, birthday);
    };

    this->threads().dispatch(fn, std::chrono::seconds(delay));

    return true;
}
