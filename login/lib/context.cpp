#include <boost/asio/high_resolution_timer.hpp>
#include <fb/login/context.h>
#include <format>

using namespace fb::login;

context::context(boost::asio::io_context& context, uint16_t port) :
    fb::acceptor<session>(context, "LOGIN", port),
    _redis(config<std::string>("redis:ip").c_str(), config<uint16_t>("redis:port"), config<uint32_t>("redis:pool"))
{
    for (auto& x : fb::config<>("forbidden"))
        this->_forbiddens.push_back(x.asString());

    // Register event handler
    this->handler.protocol.bind(&context::handle_login);
    this->handler.protocol.bind(&context::handle_agreement);
    this->handler.protocol.bind(&context::handle_create_account);
    this->handler.protocol.bind(&context::handle_complete);
    this->handler.protocol.bind(&context::handle_change_password);
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
    co_await fb::acceptor<session>::handle_start();

    this->bind_timer(&context::handle_heart_beat, 1s);
    this->handler.amqp.bind("fb.system", &context::handle_amqp_shutdown);
}

async::task<void> context::handle_heart_beat()
{
    auto root    = Json::Value{};
    root["Name"] = this->name();
    root["IP"]   = fb::config<std::string>("ip");
    root["Port"] = fb::config<uint16_t>("port");
    auto writer  = Json::FastWriter{};
    auto output  = writer.write(root);

    this->_redis.command<void>(std::format("SET heart-beat:Login:{} {}", this->id(), output));
    this->_redis.command<void>(std::format("EXPIRE heart-beat:Login:{} 5", this->id()));
    co_return;
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
        throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

    // Name must be full-hangul characters
    if (fb::config<bool>("allow other language") == false && assert_korean(cp949) == false)
        throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

    // Name cannot contains subcharacters in forbidden list
    if (this->is_forbidden(id))
        throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

    // Read character's password
    if (pw.length() < fb::config<int>("pw_size:min") || pw.length() > fb::config<int>("pw_size:max"))
        throw pw_exception(_TEXT(MESSAGE_ACCOUNT_PASSWORD_SIZE));
}

std::shared_ptr<session> context::handle_accepted(fb::socket<session>& socket)
{
    return std::make_shared<session>();
}

async::task<bool> context::handle_connected(fb::socket<session>& socket)
{
    co_return true;
}

async::task<bool> context::handle_disconnected(fb::socket<session>& socket)
{
    co_return false;
}

async::task<void> context::handle_amqp_shutdown(const internal_resp::Shutdown& response)
{
    this->exit();
    co_return;
}

async::task<bool> context::handle_agreement(fb::socket<session>& socket, const request::agreement& request)
{
    try
    {
        if (encryption::validate(request.enc_type, request.enc_key, request.enc_key_size) == false)
            throw std::exception();

        socket.encryption(request.enc_type, request.enc_key);
        socket.send(this->_agreement);
        co_return true;
    }
    catch (std::exception&)
    {
        co_return false;
    }
}

async::task<bool> context::handle_create_account(fb::socket<session>& socket, const request::create& request)
{
    // 여기는 task handler
    auto fd   = socket.fd();
    auto weak = socket.weak_from_this_as<fb::socket<session>>();

    try
    {
        auto name = std::string(request.id);
        auto pw   = std::string(request.pw);

        this->assert_account(name, pw);
        auto&& response1 = co_await this->http.post("internal", "/user/reserve-name", ReserveName{name});
        co_await this->threads.switching(weak);

        if (response1.success == false)
            throw id_exception("이미 존재하는 이름입니다.");

        auto        uid    = response1.uid;
        static auto device = std::random_device{};
        static auto gen    = std::mt19937{device()};
        static auto dist   = std::uniform_int_distribution<uint32_t>{0, 0xFFFFFFFF};

        auto   i         = dist(gen) % config<>("init:position").size();
        auto   init_x    = static_cast<uint16_t>(config<>("init:position")[i]["x"].asUInt());
        auto   init_y    = static_cast<uint16_t>(config<>("init:position")[i]["y"].asUInt());
        auto   admin     = fb::config<bool>("admin_mode");
        auto&& response2 = co_await this->http.post(
            "internal",
            "/user/init-ch",
            InitCharacter{
                uid,
                name,
                pw,
                fb::config<uint32_t>("init:hp:base") + dist(gen) % fb::config<uint32_t>("init:hp:range"), // hp
                fb::config<uint32_t>("init:mp:base") + dist(gen) % fb::config<uint32_t>("init:mp:range"), // mp
                fb::config<uint16_t>("init:map"),                                                         // map
                init_x,                                                                                   // position_x
                init_y,                                                                                   // position_y
                static_cast<uint8_t>(admin ? ROLE::ADMIN : ROLE::USER),                                   // admin
            });

        co_await this->threads.switching(weak);

        if (response2.success == false)
            throw id_exception("이미 존재하는 이름입니다.");

        this->send(socket, response::message("", 0x00));
        auto session  = socket.data();
        session->pk   = uid;
        session->name = name;
        co_return true;
    }
    catch (login_exception& e)
    {
        if (weak.expired() == false)
            socket.send(response::message(e.what(), e.type()));
    }
    catch (std::exception& e)
    {
        if (weak.expired() == false)
            socket.send(response::message(e.what(), 0x0E));
    }

    co_return true;
}

async::task<bool> context::handle_complete(fb::socket<session>& socket, const request::complete& request)
{
    auto fd   = socket.fd();
    auto weak = socket.weak_from_this_as<fb::socket<session>>();

    try
    {
        auto session = socket.data();
        if (session->pk == -1)
            throw std::exception();

        auto&& response = co_await this->http.post(
            "internal",
            "/user/mk-ch",
            MakeCharacter{session->pk, request.hair, request.sex, request.nation, request.creature});
        co_await this->threads.switching(weak);

        if (response.success == false)
            throw id_exception("이미 존재하는 이름입니다.");

        socket.send(response::message(_TEXT(MESSAGE_ACCOUNT_SUCCESS_REGISTER_ACCOUNT), 0x00));
        session->pk = -1;
        session->name.clear();
        co_return true;
    }
    catch (login_exception& e)
    {
        if (weak.expired() == false)
            socket.send(response::message(e.what(), e.type()));
    }
    catch (std::exception& e)
    {
        if (weak.expired() == false)
            socket.send(response::message(e.what(), 0x0E));
    }

    co_return true;
}

async::task<bool> context::handle_login(fb::socket<session>& socket, const request::login& request)
{
    auto weak  = socket.weak_from_this_as<fb::socket<session>>();
    auto delay = fb::config<uint32_t>("transfer delay");
    auto name  = std::string(request.id);
    auto pw    = std::string(request.pw);
    co_await this->sleep(std::chrono::seconds(delay));

    auto fd = socket.fd();
    try
    {
        this->assert_account(name, pw);

        auto&& response =
            co_await this->http.get<internal::response::GetUid>("internal", std::format("/user/uid/{}", name));
        co_await this->threads.switching(weak);

        if (response.success == false)
            throw id_exception(_TEXT(MESSAGE_ACCOUNT_NOT_FOUND_NAME));

        auto   uid       = response.uid;
        auto&& response2 = co_await this->http.post("internal", "/user/authenticate", Authenticate{uid, pw});
        co_await this->threads.switching(weak);

        switch (response2.error_code)
        {
        case 1:
            throw id_exception(_TEXT(MESSAGE_ACCOUNT_NOT_FOUND_NAME));

        case 2:
            throw pw_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_PASSWORD));
        }

        auto   map       = response2.map;
        auto&& response3 = co_await this->http.post(
            "internal",
            "/in-game/transfer",
            Transfer{fb::protocol::internal::Service ::Game, this->model.map[map].host, name, true});
        co_await this->threads.switching(weak);

        switch (static_cast<ERROR_CODE>(response3.error))
        {
        case ERROR_CODE::NONE:
            break;

        case ERROR_CODE::SERVER_NOT_READY:
            throw id_exception(_TEXT(MESSAGE_NOT_READY_GAME_SERVER));

        case ERROR_CODE::ALREADY_LOGIN:
            throw id_exception("이미 접속중입니다.");

        default:
            throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), response3.error));
        }

        socket.send(response::message("", 0x00));
        auto parameter = fb::stream();
        auto writer    = fb::stream_writer<big_endian>(parameter);
        writer.write<uint32_t>(uid);
        writer.write<std::string>(name);
        writer.write<uint8_t>(0);
        std::ignore = this->transfer(socket, response3.ip, response3.port, internal::Service::Login, parameter);
        co_return true;
    }
    catch (login_exception& e)
    {
        if (weak.expired() == false)
            socket.send(response::message(e.what(), e.type()));
    }
    catch (boost::system::error_code& e)
    {
        if (weak.expired() == false)
            socket.send(response::message(std::format("({})", e.value()), 0x0E));
    }
    catch (std::exception& e)
    {
        if (weak.expired() == false)
            socket.send(response::message(e.what(), 0x0E));
    }

    co_return true;
}

async::task<bool> context::handle_change_password(fb::socket<session>& socket, const request::update_pw& request)
{
    auto weak = socket.weak_from_this_as<fb::socket<session>>();
    try
    {
        // co_await this->_auth_service.change_pw(request.name, request.pw, request.new_pw, request.birthday);

        auto delay = fb::config<uint32_t>("transfer delay");
        co_await this->sleep(std::chrono::seconds(delay));

        if (request.name.length() < fb::config("name_size:min").asInt() ||
            request.name.length() > fb::config("name_size:max").asInt())
            throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

        // Name must be full-hangul characters
        if (fb::config<bool>("login:account option:allow other language") == false &&
            assert_korean(request.name) == false)
            throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

        // Name cannot contains subcharacters in forbidden list
        if (this->is_forbidden(request.name))
            throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

        if (request.pw.length() < fb::config("pw_size:min").asInt() ||
            request.pw.length() > fb::config("pw_size:max").asInt())
            throw pw_exception(_TEXT(MESSAGE_ACCOUNT_PASSWORD_SIZE));

        if (request.new_pw.length() < fb::config("pw_size:min").asInt() ||
            request.new_pw.length() > fb::config("pw_size:max").asInt())
            throw newpw_exception(_TEXT(MESSAGE_ACCOUNT_PASSWORD_SIZE));

        // TODO : 너무 쉬운 비밀번호인지 체크
        if (request.pw == request.new_pw)
            throw newpw_exception(_TEXT(MESSAGE_ACCOUNT_NEW_PW_EQUALIZATION));

        auto&& response =
            co_await this->http.get<internal::response::GetUid>("internal", std::format("/user/uid/{}", request.name));
        co_await this->threads.switching(weak);

        if (response.success == false)
            throw id_exception(_TEXT(MESSAGE_ACCOUNT_NOT_FOUND_NAME));

        auto uid = response.uid;

        auto&& response2 = co_await this->http.post("internal",
                                                    "/user/change-pw",
                                                    ChangePw{uid, request.pw, request.new_pw, request.birthday});

        co_await this->threads.switching(weak);

        switch (static_cast<ERROR_CODE>(response2.error_code))
        {
        case ERROR_CODE::NOT_FOUND_CHARACTER: // id wrong
            throw id_exception(_TEXT(MESSAGE_ACCOUNT_NOT_FOUND_NAME));

        case ERROR_CODE::PASSWORD_NOT_MATCHED: // pw wrong
            throw pw_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_PASSWORD));

        case ERROR_CODE::BIRTHDAY_NOT_MATCHED: // birthday wrong
            throw pw_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_BIRTHDAY));
        }

        socket.send(response::message((_TEXT(MESSAGE_ACCOUNT_SUCCESS_CHANGE_PASSWORD)), 0x00));
        co_return true;
    }
    catch (login_exception& e)
    {
        if (weak.expired() == false)
            socket.send(response::message(e.what(), e.type()));
    }
    catch (std::exception& e)
    {
        if (weak.expired() == false)
            socket.send(response::message(e.what(), 0x0E));
    }

    co_return true;
}

void context::handle_init_amqp(fb::amqp::socket& amqp)
{
    this->handler.amqp.declare_queue("amq.direct", "fb.system");
}