#include <fb/login/context.h>
#include <fb/protocol/flatbuffer/fb.protocol.internal.request.h>
#include <fb/protocol/flatbuffer/fb.protocol.internal.response.h>
#include <boost/asio/high_resolution_timer.hpp>

fb::login::context::context(boost::asio::io_context& context, uint16_t port) : 
    fb::acceptor<fb::login::session>(context, port, std::thread::hardware_concurrency()),
    _db(*this, 4)
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
    fb::logger::info("%s님이 접속했습니다.", socket.IP().c_str());
    return true;
}

async::task<bool> fb::login::context::handle_disconnected(fb::socket<fb::login::session>& socket)
{
    fb::logger::info("%s님의 연결이 끊어졌습니다.", socket.IP().c_str());
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
        auto id = request.id;
        auto pw = request.pw;

        this->assert_account(id, pw);

        auto&& nameset_result = co_await this->_db.co_exec_f("CALL USP_NAME_SET('%s')", id.c_str()); // 여기서 task suspend, DB스레드에서 promise resume

        // 여기부터 promise handler
        if (this->sockets.contains(fd) == false)
            co_return false;

        auto success = nameset_result[0].get_value<bool>(0);
        if(success == false)
            throw id_exception("이미 존재하는 이름입니다.");

        auto pk = nameset_result[1].get_value<uint32_t>(0);
        auto& config = fb::config::get();
        std::srand(std::time(nullptr));
        auto hp = config["init"]["hp"]["base"].asInt() + std::rand() % config["init"]["hp"]["range"].asInt();
        auto mp = config["init"]["mp"]["base"].asInt() + std::rand() % config["init"]["mp"]["range"].asInt();
        auto map = config["init"]["map"].asInt();
        auto position_x = config["init"]["position"]["x"].asInt();
        auto position_y = config["init"]["position"]["y"].asInt();
        auto admin = config["admin mode"].asBool() ? 0 : 1;

        // 여기서 promise suspend
        auto&& init_result = co_await this->_db.co_exec_f(pk, "CALL USP_CHARACTER_INIT(%d, '%s', '%s', %d, %d, %d, %d, %d, %d)", pk, id.c_str(), this->sha256(pw).c_str(), hp, mp, map, position_x, position_y, admin);

        // 여기서 새로운 promise handler
        if (this->sockets.contains(fd) == false)
            co_return false;

        auto& init_row = init_result[0];
        if(init_row.get_value<bool>(0) == false)
            throw id_exception("이미 존재하는 이름입니다.");

        this->send(socket, fb::protocol::login::response::message("", 0x00));
        auto session = socket.data();
        session->pk = pk;
        session->name = id;
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
    try
    {
        auto session = socket.data();
        if(session->pk == -1)
            throw std::exception();

        auto fd = socket.fd();
        co_await this->_db.co_exec_f(session->pk, "CALL USP_CHARACTER_CREATE_FINISH(%d, %d, %d, %d, %d)", session->pk, request.hair, request.sex, request.nation, request.creature);
        if (this->sockets.contains(fd) == false)
            co_return false;

        socket.send(fb::protocol::login::response::message(fb::login::message::account::SUCCESS_REGISTER_ACCOUNT, 0x00));
        session->pk = -1;
        session->name.clear();
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

        auto&& name_result = co_await this->_db.co_exec_f("CALL USP_NAME_GET_ID('%s')", name.c_str());
        if (this->sockets.contains(fd) == false)
            co_return false;

        auto& name_row = name_result[0];
        if(name_row.count() == 0)
            throw id_exception(fb::login::message::account::NOT_FOUND_NAME);

        auto id = name_row.get_value<uint32_t>(0);
        auto&& auth_result = co_await this->_db.co_exec_f(id, "SELECT id, pw, map FROM user WHERE name='%s' LIMIT 1", name.c_str(), this->sha256(pw).c_str());
        if (this->sockets.contains(fd) == false)
            co_return false;

        auto& auth_row = auth_result[0];
        if(auth_row.count() == 0)
            throw id_exception(fb::login::message::account::NOT_FOUND_NAME);

        if(auth_row.get_value<std::string>(1) != this->sha256(pw))
            throw pw_exception(fb::login::message::account::INVALID_PASSWORD);

        auto map = auth_row.get_value<uint32_t>(2);
        auto&& response = co_await this->post_async<fb::protocol::internal::request::Login, fb::protocol::internal::response::Login>(
            "localhost:5126", "/access/login", 
            fb::protocol::internal::request::Login
            {
                auth_row.get_value<uint32_t>(0), 
                UTF8(name, PLATFORM::Windows),
                (uint16_t)map 
            });
        if (this->sockets.contains(fd) == false)
            co_return false;

        if (!response.success)
        {
            if(response.logon)
                throw id_exception("이미 접속중입니다.");
            else
                throw id_exception("비바람이 휘몰아치고 있습니다.");
        }

        socket.send(fb::protocol::login::response::message("", 0x00));
        fb::ostream         parameter;
        parameter.write_u32(id);
        parameter.write(name);
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

async::task<bool> fb::login::context::handle_change_password(fb::socket<fb::login::session>& socket, const fb::protocol::login::request::account::change_pw& request)
{
    auto delay = fb::config::get()["transfer delay"].asInt();
    co_await this->sleep(std::chrono::seconds(delay));

    auto fd = socket.fd();
    try
    {
        //co_await this->_auth_service.change_pw(request.name, request.pw, request.new_pw, request.birthday);

        auto name = request.name;
        auto pw = request.pw;
        auto new_pw = request.new_pw;
        auto birthday = request.birthday;

        const auto& config = fb::config::get();
        if(name.length() < config["name_size"]["min"].asInt() || name.length() > config["name_size"]["max"].asInt())
            throw id_exception(fb::login::message::account::INVALID_NAME);

        // Name must be full-hangul characters
        if(fb::config::get()["login"]["account option"]["allow other language"].asBool() == false && assert_korean(name.c_str()) == false)
            throw id_exception(fb::login::message::account::INVALID_NAME);

        // Name cannot contains subcharacters in forbidden list
        if(this->is_forbidden(name.c_str()))
            throw id_exception(fb::login::message::account::INVALID_NAME);

        if(pw.length() < config["pw_size"]["min"].asInt() || pw.length() > config["pw_size"]["max"].asInt())
            throw pw_exception(fb::login::message::account::PASSWORD_SIZE);

        if(new_pw.length() < config["pw_size"]["min"].asInt() || new_pw.length() > config["pw_size"]["max"].asInt())
            throw newpw_exception(fb::login::message::account::PASSWORD_SIZE);

        // TODO : 너무 쉬운 비밀번호인지 체크
        if(pw == new_pw)
            throw newpw_exception(fb::login::message::account::NEW_PW_EQUALIZATION);

        auto&& name_result = co_await this->_db.co_exec_f("CALL USP_NAME_GET_ID('%s')", name.c_str());
        if (this->sockets.contains(fd) == false)
            co_return false;

        auto& name_row = name_result[0];
        if(name_row.count() == 0)
            throw id_exception(fb::login::message::account::NOT_FOUND_NAME);

        auto id = name_row.get_value<uint32_t>(0);

        auto&& pw_result = co_await this->_db.co_exec_f(id, "CALL USP_CHANGE_PW(%d, '%s', '%s', %d)", id, this->sha256(pw).c_str(), this->sha256(new_pw).c_str(), birthday);
        if (this->sockets.contains(fd) == false)
            co_return false;

        auto& pw_row = pw_result[0];
        auto  result_code = pw_row.get_value<int>(0);
        switch(result_code)
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