#include <boost/asio/high_resolution_timer.hpp>
#include <context.h>
#include <format>

using namespace fb::login;

context::context(boost::asio::io_context& context, uint16_t port) :
    fb::acceptor<session>(context, port)
{
    for (auto& x : fb::config<>("forbidden"))
        this->_forbiddens.push_back(x.asString());

    // Register event handler
    this->bind(&context::handle_login);
    this->bind(&context::handle_agreement);
    this->bind(&context::handle_create_account);
    this->bind(&context::handle_account_complete);
    this->bind(&context::handle_change_password);
}

context::~context()
{ }

bool context::decrypt_policy(uint8_t cmd) const
{
    switch (cmd)
    {
    case 0x10:
        return false;

    default:
        return true;
    }
}

async::task<void> context::handle_start()
{
    this->bind_timer(&context::handle_heart_beat, 1s);

    co_await fb::acceptor<session>::handle_start();
}

async::task<void> context::handle_heart_beat()
{
    auto&& response =
        co_await this->post<fb::protocol::internal::request::Ping, fb::protocol::internal::response::Pong>(
            "internal",
            "/in-game/ping",
            fb::protocol::internal::request::Ping{this->id(),
                                                  this->name(),
                                                  this->service(),
                                                  fb::config<std::string>("ip"),
                                                  fb::config<uint16_t>("port")});
}

bool context::is_forbidden(const std::string& str) const
{
    return std::any_of(this->_forbiddens.cbegin(), this->_forbiddens.cend(), [str](const auto& x) {
        return x == str;
    });
}

void context::assert_account(const std::string& id, const std::string& pw) const
{
    auto cp949     = CP949(id);
    auto name_size = cp949.length();

    if (name_size < fb::config<int>("name_size:min") || name_size > fb::config<int>("name_size:max"))
        throw id_exception(message::account::INVALID_NAME);

    // Name must be full-hangul characters
    if (fb::config<bool>("allow other language") == false && assert_korean(cp949) == false)
        throw id_exception(message::account::INVALID_NAME);

    // Name cannot contains subcharacters in forbidden list
    if (this->is_forbidden(id))
        throw id_exception(message::account::INVALID_NAME);

    // Read character's password
    if (pw.length() < fb::config<int>("pw_size:min") || pw.length() > fb::config<int>("pw_size:max"))
        throw pw_exception(message::account::PASSWORD_SIZE);
}

session* context::handle_accepted(fb::socket<session>& socket)
{
    auto uptr = std::make_unique<session>();
    auto ptr  = uptr.get();
    this->_sessions.push_back(std::move(uptr));
    return ptr;
}

async::task<bool> context::handle_connected(fb::socket<session>& socket)
{
    fb::logger::info("{}님이 접속했습니다.", socket.IP());
    co_return true;
}

async::task<bool> context::handle_disconnected(fb::socket<session>& socket)
{
    fb::logger::info("{}님의 연결이 끊어졌습니다.", socket.IP());
    co_return false;
}

async::task<bool> context::handle_agreement(fb::socket<session>& socket, const request::agreement& request)
{
    try
    {
        if (cryptor::validate(request.enc_type, request.enc_key, request.enc_key_size) == false)
            throw std::exception();

        socket.crt(request.enc_type, request.enc_key);
        co_await socket.send(this->_agreement);
        co_return true;
    }
    catch (std::exception&)
    {
        co_return false;
    }
}

async::task<bool> context::handle_create_account(fb::socket<session>& socket, const request::account::create& request)
{
    // 여기는 task handler
    auto fd         = socket.fd();
    auto error      = std::string();
    auto error_code = 0x0E;

    try
    {
        auto name = std::string(request.id);
        auto pw   = std::string(request.pw);

        this->assert_account(name, pw);

        auto&& response1 = co_await this->post<internal::request::ReserveName, internal::response::ReserveName>(
            "internal",
            "/user/reserve-name",
            internal::request::ReserveName{name});

        if (this->sockets.contains(fd) == false)
            co_return false;

        if (response1.uid == -1)
            throw id_exception("이미 존재하는 이름입니다.");

        auto uid = response1.uid;
        std::srand(std::time(nullptr));

        auto&& response2 = co_await this->post<internal::request::InitCharacter, internal::response::InitCharacter>(
            "internal",
            "/user/init-ch",
            internal::request::InitCharacter{
                uid,
                name,
                pw,
                fb::config<uint32_t>("init:hp:base") + std::rand() % fb::config<uint32_t>("init:hp:range"), // hp
                fb::config<uint32_t>("init:mp:base") + std::rand() % fb::config<uint32_t>("init:mp:range"), // mp
                fb::config<uint16_t>("init:map"),                                                           // map
                fb::config<uint16_t>("init:position:x"), // position_x
                fb::config<uint16_t>("init:position:y"), // position_y
                fb::config<bool>("admin mode"),          // admin
            });

        // 여기서 새로운 promise handler
        if (this->sockets.contains(fd) == false)
            co_return false;

        if (response2.success == false)
            throw id_exception("이미 존재하는 이름입니다.");

        co_await this->send(socket, response::message("", 0x00));
        auto session  = socket.data();
        session->pk   = uid;
        session->name = name;
        co_return true;
    }
    catch (login_exception& e)
    {
        error      = e.what();
        error_code = e.type();
    }
    catch (std::exception& e)
    {
        error      = e.what();
        error_code = 0x0E;
    }

    if (this->sockets.contains(fd) == false)
        co_return false;

    co_await socket.send(response::message(error, error_code));
}

async::task<bool> context::handle_account_complete(fb::socket<session>&              socket,
                                                   const request::account::complete& request)
{
    auto fd         = socket.fd();
    auto error      = std::string();
    auto error_code = 0x0E;

    try
    {
        auto session = socket.data();
        if (session->pk == -1)
            throw std::exception();

        auto&& response = co_await this->post<internal::request::MakeCharacter, internal::response::MakeCharacter>(
            "internal",
            "/user/mk-ch",
            internal::request::MakeCharacter{session->pk, request.hair, request.sex, request.nation, request.creature});
        if (this->sockets.contains(fd) == false)
            co_return false;

        if (response.success == false)
            throw id_exception("이미 존재하는 이름입니다.");

        co_await socket.send(response::message(message::account::SUCCESS_REGISTER_ACCOUNT, 0x00));
        session->pk = -1;
        session->name.clear();
        co_return true;
    }
    catch (login_exception& e)
    {
        error      = e.what();
        error_code = e.type();
    }
    catch (std::exception& e)
    {
        co_return false;
    }

    if (this->sockets.contains(fd) == false)
        co_return false;

    co_await socket.send(response::message(error, error_code));
    co_return true;
}

async::task<bool> context::handle_login(fb::socket<session>& socket, const request::login& request)
{
    auto delay      = fb::config<uint32_t>("transfer delay");
    auto name       = std::string(request.id);
    auto pw         = std::string(request.pw);
    auto error      = std::string();
    auto error_code = 0x0E;
    co_await this->sleep(std::chrono::seconds(delay));

    auto fd = socket.fd();
    try
    {
        this->assert_account(name, pw);

        auto&& response = co_await this->get<internal::response::GetUid>("internal", std::format("/user/uid/{}", name));
        if (this->sockets.contains(fd) == false)
            co_return false;

        if (response.success == false)
            throw id_exception(message::account::NOT_FOUND_NAME);

        auto   uid       = response.uid;
        auto&& response2 = co_await this->post<internal::request::Authenticate, internal::response::Authenticate>(
            "internal",
            "/user/authenticate",
            internal::request::Authenticate{uid, pw});
        if (this->sockets.contains(fd) == false)
            co_return false;

        switch (response2.error_code)
        {
        case 1:
            throw id_exception(message::account::NOT_FOUND_NAME);

        case 2:
            throw pw_exception(message::account::INVALID_PASSWORD);
        }

        auto   map       = response2.map;
        auto&& response3 = co_await this->post<internal::request::Transfer, internal::response::Transfer>(
            "internal",
            "/in-game/transfer",
            internal::request::Transfer{fb::protocol::internal::Service ::Game, this->model.map[map].host, name, true});
        if (this->sockets.contains(fd) == false)
            co_return false;

        switch (static_cast<ERROR_CODE>(response3.error))
        {
        case ERROR_CODE::NONE:
            break;

        case ERROR_CODE::SERVER_NOT_READY:
            throw id_exception("비바람이 휘몰아치고 있습니다.");

        case ERROR_CODE::ALREADY_LOGIN:
            throw id_exception("이미 접속중입니다.");

        default:
            throw std::runtime_error(std::format("알 수 없는 에러가 발생했습니다. (에러코드 : {})", response3.error));
        }

        co_await socket.send(response::message("", 0x00));
        auto parameter = fb::stream();
        auto writer    = fb::stream_writer<big_endian>(parameter);
        writer.write<uint32_t>(uid);
        writer.write<std::string>(name);
        writer.write<uint8_t>(0);
        co_await this->transfer(socket, response3.ip, response3.port, internal::Service::Login, parameter);
        co_return true;
    }
    catch (login_exception& e)
    {
        error      = e.what();
        error_code = e.type();
    }
    catch (boost::system::error_code& e)
    {
        error      = std::format("({})", e.value());
        error_code = 0x0E;
    }
    catch (std::exception& e)
    {
        error      = e.what();
        error_code = 0x0E;
    }

    if (this->sockets.contains(fd) == false)
        co_return false;

    co_await socket.send(response::message(error, error_code));
    co_return true;
}

async::task<bool> context::handle_change_password(fb::socket<session>&               socket,
                                                  const request::account::change_pw& request)
{
    auto fd         = socket.fd();
    auto error      = std::string();
    auto error_code = 0x0E;
    try
    {
        // co_await this->_auth_service.change_pw(request.name, request.pw, request.new_pw, request.birthday);

        auto name     = std::string(request.name);
        auto pw       = std::string(request.pw);
        auto new_pw   = std::string(request.new_pw);
        auto birthday = request.birthday;

        auto delay = fb::config<uint32_t>("transfer delay");
        co_await this->sleep(std::chrono::seconds(delay));

        if (name.length() < fb::config("name_size:min").asInt() || name.length() > fb::config("name_size:max").asInt())
            throw id_exception(message::account::INVALID_NAME);

        // Name must be full-hangul characters
        if (fb::config<bool>("login:account option:allow other language") == false && assert_korean(name) == false)
            throw id_exception(message::account::INVALID_NAME);

        // Name cannot contains subcharacters in forbidden list
        if (this->is_forbidden(name))
            throw id_exception(message::account::INVALID_NAME);

        if (pw.length() < fb::config("pw_size:min").asInt() || pw.length() > fb::config("pw_size:max").asInt())
            throw pw_exception(message::account::PASSWORD_SIZE);

        if (new_pw.length() < fb::config("pw_size:min").asInt() || new_pw.length() > fb::config("pw_size:max").asInt())
            throw newpw_exception(message::account::PASSWORD_SIZE);

        // TODO : 너무 쉬운 비밀번호인지 체크
        if (pw == new_pw)
            throw newpw_exception(message::account::NEW_PW_EQUALIZATION);

        auto&& response = co_await this->get<internal::response::GetUid>("internal", std::format("/user/uid/{}", name));
        if (this->sockets.contains(fd) == false)
            co_return false;

        if (response.success == false)
            throw id_exception(message::account::NOT_FOUND_NAME);

        auto uid = response.uid;

        auto&& response2 = co_await this->post<internal::request::ChangePw, internal::response::ChangePw>(
            "internal",
            "/user/change-pw",
            internal::request::ChangePw{uid, pw, new_pw, birthday});

        if (this->sockets.contains(fd) == false)
            co_return false;

        switch (response2.error_code)
        {
        case 1: // id wrong
            throw id_exception(message::account::NOT_FOUND_NAME);

        case 2: // pw wrong
            throw pw_exception(message::account::INVALID_PASSWORD);

        case 3: // birthday wrong
            throw pw_exception(message::account::INVALID_BIRTHDAY);
        }

        co_await socket.send(response::message((message::account::SUCCESS_CHANGE_PASSWORD), 0x00));
        co_return true;
    }
    catch (login_exception& e)
    {
        error      = e.what();
        error_code = e.type();
    }
    catch (std::exception& e)
    {
        error      = e.what();
        error_code = 0x0E;
    }

    if (this->sockets.contains(fd) == false)
        co_return false;

    co_await socket.send(response::message(error, error_code));
    co_return true;
}