#include <context.h>
#include <format>
#include <boost/asio/high_resolution_timer.hpp>

fb::login::context::context(boost::asio::io_context& context, uint16_t port) : 
    fb::acceptor<fb::login::session>(context, port)
{
    const auto& config = fb::config::get();
    for(auto& x : config["forbidden"])
        this->_forbiddens.push_back(x.asString());
    
    // Register event handler
    this->bind<fb::protocol::login::request::login>                   (std::bind(&context::handle_login,               this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::login::request::agreement>               (std::bind(&context::handle_agreement,           this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::login::request::account::create>         (std::bind(&context::handle_create_account,      this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::login::request::account::complete>       (std::bind(&context::handle_account_complete,    this, std::placeholders::_1, std::placeholders::_2));
    this->bind<fb::protocol::login::request::account::change_pw>      (std::bind(&context::handle_change_password,     this, std::placeholders::_1, std::placeholders::_2));
}

fb::login::context::~context()
{ }

bool fb::login::context::decrypt_policy(uint8_t cmd) const
{
    switch(cmd)
    {
    case 0x10:
        return false;

    default:
        return true;
    }
}

bool fb::login::context::is_forbidden(const std::string& str) const
{
    return std::any_of(this->_forbiddens.cbegin(), this->_forbiddens.cend(), 
        [str](const auto& x)
        {
            return x == str;
        });
}

std::string fb::login::context::sha256(const std::string& data) const
{
    auto buffer = new char[data.size()];
    memcpy(buffer, data.c_str(), data.size());

    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, buffer, data.size());
    delete[] buffer;

    uint8_t hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &ctx);
    
    std::stringstream sstream;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sstream << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];

    return sstream.str();
}

void fb::login::context::assert_account(const std::string& id, const std::string& pw) const
{
    const auto& config = fb::config::get();
    auto cp949 = CP949(id);
    auto name_size = cp949.length();
    if(name_size < config["name_size"]["min"].asInt() || name_size > config["name_size"]["max"].asInt())
        throw id_exception(fb::login::message::account::INVALID_NAME);

    // Name must be full-hangul characters
    if(config["allow other language"].asBool() == false && assert_korean(cp949) == false)
        throw id_exception(fb::login::message::account::INVALID_NAME);

    // Name cannot contains subcharacters in forbidden list
    if(this->is_forbidden(id))
        throw id_exception(fb::login::message::account::INVALID_NAME);


    // Read character's password
    if(pw.length() < config["pw_size"]["min"].asInt() || pw.length() > config["pw_size"]["max"].asInt())
        throw pw_exception(fb::login::message::account::PASSWORD_SIZE);
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
    fb::logger::info("{}님이 접속했습니다.", socket.IP());
    return true;
}

async::task<bool> fb::login::context::handle_disconnected(fb::socket<fb::login::session>& socket)
{
    fb::logger::info("{}님의 연결이 끊어졌습니다.", socket.IP());
    co_return false;
}

async::task<bool> fb::login::context::handle_agreement(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::agreement& request)
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

async::task<bool> fb::login::context::handle_create_account(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::account::create& request)
{
    // 여기는 task handler
    auto fd = socket.fd();

    try
    {
        auto name = std::string(request.id);
        auto pw = std::string(request.pw);

        this->assert_account(name, pw);

        auto&& response1 = co_await this->post<fb::protocol::db::request::ReserveName, fb::protocol::db::response::ReserveName>("db", "/user/reserve-name", fb::protocol::db::request::ReserveName
            {
                UTF8(name, PLATFORM::Windows)
            });

        if (this->sockets.contains(fd) == false)
            co_return false;

        if(response1.uid == -1)
            throw id_exception("이미 존재하는 이름입니다.");

        auto uid = response1.uid;
        auto& config = fb::config::get();
        std::srand(std::time(nullptr));

        auto&& response2 = co_await this->post<fb::protocol::db::request::InitCharacter, fb::protocol::db::response::InitCharacter>("db", "/user/init-ch", fb::protocol::db::request::InitCharacter
        {
            uid,
            UTF8(name, PLATFORM::Windows),
            pw,
            config["init"]["hp"]["base"].asUInt() + std::rand() % config["init"]["hp"]["range"].asUInt(), // hp
            config["init"]["mp"]["base"].asUInt() + std::rand() % config["init"]["mp"]["range"].asUInt(), // mp
            (uint16_t)config["init"]["map"].asUInt(), // map
            (uint16_t)config["init"]["position"]["x"].asUInt(), // position_x
            (uint16_t)config["init"]["position"]["y"].asUInt(), // position_y
            config["admin mode"].asBool(), // admin
        });

        // 여기서 새로운 promise handler
        if (this->sockets.contains(fd) == false)
            co_return false;

        if(response2.success == false)
            throw id_exception("이미 존재하는 이름입니다.");

        this->send(socket, fb::protocol::login::response::message("", 0x00));
        auto session = socket.data();
        session->pk = uid;
        session->name = name;
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

async::task<bool> fb::login::context::handle_account_complete(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::account::complete& request)
{
    auto fd = socket.fd();

    try
    {
        auto session = socket.data();
        if(session->pk == -1)
            throw std::exception();

        auto&& response = co_await this->post<fb::protocol::db::request::MakeCharacter, fb::protocol::db::response::MakeCharacter>("db", "/user/mk-ch", fb::protocol::db::request::MakeCharacter
        {
            session->pk,
            request.hair,
            request.sex,
            request.nation,
            request.creature
        });
        if (this->sockets.contains(fd) == false)
            co_return false;

        if(response.success == false)
            throw id_exception("이미 존재하는 이름입니다.");

        socket.send(fb::protocol::login::response::message(fb::login::message::account::SUCCESS_REGISTER_ACCOUNT, 0x00));
        session->pk = -1;
        session->name.clear();
        co_return true;
    }
    catch(login_exception& e)
    {
        if (this->sockets.contains(fd) == false)
            co_return false;

        socket.send(fb::protocol::login::response::message(e.what(), e.type()));
        co_return true;
    }
    catch(std::exception&)
    {
        co_return false;
    }
}

async::task<bool> fb::login::context::handle_login(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::login& request)
{
    auto delay = fb::config::get()["transfer delay"].asInt();
    auto name = std::string(request.id);
    auto pw = std::string(request.pw);
    co_await this->sleep(std::chrono::seconds(delay));

    auto fd = socket.fd();
    try
    {
        this->assert_account(name, pw);

        auto&& response = co_await this->get<fb::protocol::db::response::GetUid>("db", std::format("/user/uid/{}", name));
        if (this->sockets.contains(fd) == false)
            co_return false;

        if(response.success == false)
            throw id_exception(fb::login::message::account::NOT_FOUND_NAME);

        auto uid = response.uid;
        auto&& response2 = co_await this->post<fb::protocol::db::request::Authenticate, fb::protocol::db::response::Authenticate>("db", "/user/authenticate", fb::protocol::db::request::Authenticate
            {
                uid, pw
            });
        if (this->sockets.contains(fd) == false)
            co_return false;

        switch (response2.error_code)
        {
        case 1:
            throw id_exception(fb::login::message::account::NOT_FOUND_NAME);

        case 2:
            throw pw_exception(fb::login::message::account::INVALID_PASSWORD);
        }

        auto map = response2.map;
        auto&& response3 = co_await this->post<fb::protocol::internal::request::Login, fb::protocol::internal::response::Login>(
            "internal",
            "/in-game/login", 
            fb::protocol::internal::request::Login
            {
                uid, 
                UTF8(name, PLATFORM::Windows),
                (uint16_t)map 
            });
        if (this->sockets.contains(fd) == false)
            co_return false;

        if (!response3.success)
        {
            if(response3.logon)
                throw id_exception("이미 접속중입니다.");
            else
                throw id_exception("비바람이 휘몰아치고 있습니다.");
        }

        socket.send(fb::protocol::login::response::message("", 0x00));
        fb::ostream         parameter;
        parameter.write_u32(uid);
        parameter.write(name);
        parameter.write_u8(0);
        this->transfer(socket, response3.ip, response3.port, fb::protocol::internal::services::LOGIN, parameter);
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

async::task<bool> fb::login::context::handle_change_password(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::account::change_pw& request)
{
    auto fd = socket.fd();
    try
    {
        //co_await this->_auth_service.change_pw(request.name, request.pw, request.new_pw, request.birthday);

        auto name = std::string(request.name);
        auto pw = std::string(request.pw);
        auto new_pw = std::string(request.new_pw);
        auto birthday = request.birthday;

        auto delay = fb::config::get()["transfer delay"].asInt();
        co_await this->sleep(std::chrono::seconds(delay));

        const auto& config = fb::config::get();
        if(name.length() < config["name_size"]["min"].asInt() || name.length() > config["name_size"]["max"].asInt())
            throw id_exception(fb::login::message::account::INVALID_NAME);

        // Name must be full-hangul characters
        if(fb::config::get()["login"]["account option"]["allow other language"].asBool() == false && assert_korean(name) == false)
            throw id_exception(fb::login::message::account::INVALID_NAME);

        // Name cannot contains subcharacters in forbidden list
        if(this->is_forbidden(name))
            throw id_exception(fb::login::message::account::INVALID_NAME);

        if(pw.length() < config["pw_size"]["min"].asInt() || pw.length() > config["pw_size"]["max"].asInt())
            throw pw_exception(fb::login::message::account::PASSWORD_SIZE);

        if(new_pw.length() < config["pw_size"]["min"].asInt() || new_pw.length() > config["pw_size"]["max"].asInt())
            throw newpw_exception(fb::login::message::account::PASSWORD_SIZE);

        // TODO : 너무 쉬운 비밀번호인지 체크
        if(pw == new_pw)
            throw newpw_exception(fb::login::message::account::NEW_PW_EQUALIZATION);

        auto&& response = co_await this->get<fb::protocol::db::response::GetUid>("db", std::format("/user/uid/{}", name));
        if (this->sockets.contains(fd) == false)
            co_return false;

        if (response.success == false)
            throw id_exception(fb::login::message::account::NOT_FOUND_NAME);

        auto uid = response.uid;

        auto&& response2 = co_await this->post<fb::protocol::db::request::ChangePw, fb::protocol::db::response::ChangePw>("db", "/user/change-pw", fb::protocol::db::request::ChangePw
            {
                uid,
                this->sha256(pw),
                this->sha256(new_pw),
                birthday
            });

        if (this->sockets.contains(fd) == false)
            co_return false;

        switch(response2.result)
        {
        case -1: // id wrong
            throw id_exception(fb::login::message::account::NOT_FOUND_NAME);

        case -2: // pw wrong
            throw pw_exception(fb::login::message::account::INVALID_PASSWORD);

        case -3: // birthday wrong
            throw pw_exception(fb::login::message::account::INVALID_BIRTHDAY);
        }

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