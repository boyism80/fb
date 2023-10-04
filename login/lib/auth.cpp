#include <fb/login/auth.h>

fb::login::service::auth::auth(fb::db::context& db) : _db(db)
{
    const auto& config = fb::config::get();

    for(auto x : config["forbidden"])
        this->_forbiddens.push_back(x.asString());
}

fb::login::service::auth::~auth()
{
    
}

// 참고자료
// https://gala04.tistory.com/entry/%EC%A0%9C%EB%AA%A9%EC%9D%84-%EC%9E%85%EB%A0%A5%ED%95%B4-%EC%A3%BC%EC%84%B8%EC%9A%94
bool fb::login::service::auth::is_hangul(const std::string& str)
{
    auto len = str.length();
    if(len % 2 > 0)
        return false;

    auto raw = str.c_str();
    for(int i = 0; i < len; i += 2)
    {
        uint8_t e1 = raw[i+0];
        uint8_t e2 = raw[i+1];

        if(isascii(e1))
            return false;

        if(e1 < 0xB0 || e1 > 0xC8)
            return false;

        if(e2 < 0xA1 || e2 > 0xFE)
            return false;
    }

    return true;
}

bool fb::login::service::auth::is_forbidden(const std::string& str) const
{
    return std::any_of
    (
        this->_forbiddens.cbegin(), 
        this->_forbiddens.cend(), 
        [str](const auto& x)
        {
            return x == str;
        }
    );
}

std::string fb::login::service::auth::sha256(const std::string& data) const
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

fb::task fb::login::service::auth::__exists(fb::awaitable<bool>& awaitable, std::string name)
{
    try
    {
        auto results = co_await this->_db.co_exec_f(name, "CALL USP_CHARACTER_EXISTS('%s')", name.c_str());
        auto exists = results[0].get_value<int>() > 0;
        awaitable.result = &exists;
    }
    catch(std::exception& e)
    {
        awaitable.error = std::make_unique<std::runtime_error>(e.what());
    }

    awaitable.handler.resume();
}

auto fb::login::service::auth::exists(const std::string& name)
{
    auto await_callback = [this, name] (auto& awaitable)
    {
        this->__exists(awaitable, name);
    };

    return fb::awaitable<bool>(await_callback);
}

void fb::login::service::auth::assert_account(const std::string& id, const std::string& pw) const
{
    const auto& config = fb::config::get();
    auto cp949 = CP949(id);
    auto name_size = cp949.length();
    if(name_size < config["name_size"]["min"].asInt() || name_size > config["name_size"]["max"].asInt())
        throw id_exception(fb::login::message::account::INVALID_NAME);

    // Name must be full-hangul characters
    if(config["allow other language"].asBool() == false && this->is_hangul(cp949) == false)
        throw id_exception(fb::login::message::account::INVALID_NAME);

    // Name cannot contains subcharacters in forbidden list
    if(this->is_forbidden(id))
        throw id_exception(fb::login::message::account::INVALID_NAME);


    // Read character's password
    if(pw.length() < config["pw_size"]["min"].asInt() || pw.length() > config["pw_size"]["max"].asInt())
        throw pw_exception(fb::login::message::account::PASSWORD_SIZE);
}

fb::task fb::login::service::auth::__create_account(fb::awaitable<void>& awaitable, std::string id, std::string pw)
{
    try
    {
        this->assert_account(id, pw);

        auto& config = fb::config::get();
        std::srand(std::time(nullptr));
        auto hp = config["init"]["hp"]["base"].asInt() + std::rand() % config["init"]["hp"]["range"].asInt();
        auto mp = config["init"]["mp"]["base"].asInt() + std::rand() % config["init"]["mp"]["range"].asInt();
        auto map = config["init"]["map"].asInt();
        auto position_x = config["init"]["position"]["x"].asInt();
        auto position_y = config["init"]["position"]["y"].asInt();
        auto admin = config["admin mode"].asBool() ? 0 : 1;

        auto results = co_await this->_db.co_exec_f(id, "CALL USP_CHARACTER_INIT('%s', '%s', %d, %d, %d, %d, %d, %d)", id.c_str(), this->sha256(pw).c_str(), hp, mp, map, position_x, position_y, admin);
        auto& result = results[0];

        auto success = result.get_value<bool>(0);
        if(success == false)
            throw std::exception();
    }
    catch(login_exception& e)
    {
        awaitable.error = std::make_unique<login_exception>(e.type(), e.what());
    }
    catch(std::exception& e)
    {
        awaitable.error = std::make_unique<login_exception>(0x0E, e.what());
    }

    awaitable.handler.resume();
}

fb::awaitable<void> fb::login::service::auth::create_account(const std::string& id, const std::string& pw)
{
    auto await_callback = [this, id, pw] (auto& awaitable)
    {
        this->__create_account(awaitable, id, pw);
    };

    return fb::awaitable<void>(await_callback);
}

fb::task fb::login::service::auth::__init_account(fb::awaitable<void>& awaitable, std::string id, uint8_t hair, uint8_t sex, uint8_t nation, uint8_t creature)
{
    try
    {
        co_await this->_db.co_exec_f(id, "CALL USP_CHARACTER_CREATE_FINISH('%s', %d, %d, %d, %d)", id.c_str(), hair, sex, nation, creature);
    }
    catch(std::exception& e)
    {
        awaitable.error = std::make_unique<std::runtime_error>(e.what());
    }
    awaitable.handler.resume();
}

fb::awaitable<void> fb::login::service::auth::init_account(const std::string& id, uint8_t hair, uint8_t sex, uint8_t nation, uint8_t creature)
{
    auto await_callback = [this, id, hair, sex, nation, creature] (auto& awaitable)
    {
        this->__init_account(awaitable, id, hair, sex, nation, creature);
    };

    return fb::awaitable<void>(await_callback);
}

fb::task fb::login::service::auth::__login(fb::awaitable<uint32_t, login_exception>& awaitable, std::string id, std::string pw)
{
    try
    {
        this->assert_account(id, pw);
        auto  results = co_await this->_db.co_exec_f(id, "SELECT pw, map FROM user WHERE name='%s' LIMIT 1", id.c_str(), this->sha256(pw).c_str());
        auto& result = results[0];
        if(result.count() == 0)
            throw id_exception(fb::login::message::account::NOT_FOUND_NAME);

        if(result.get_value<std::string>(0) != this->sha256(pw))
            throw pw_exception(fb::login::message::account::INVALID_PASSWORD);

        auto value = result.get_value<uint32_t>(1);
        awaitable.result = &value;
    }
    catch(id_exception& e)
    {
        awaitable.error = std::make_unique<id_exception>(e.what());
    }
    catch(pw_exception& e)
    {
        awaitable.error = std::make_unique<pw_exception>(e.what());
    }
    catch(login_exception& e)
    {
        awaitable.error = std::make_unique<login_exception>(e.type(), e.what());
    }
    catch(std::exception& e)
    {
        awaitable.error = std::make_unique<login_exception>(0x0E, e.what());
    }
    awaitable.handler.resume();
}

fb::awaitable<uint32_t, login_exception> fb::login::service::auth::login(const std::string& id, const std::string& pw)
{
    auto await_callback = [this, id, pw] (auto& awaitable)
    {
        this->__login(awaitable, id, pw);
    };

    return fb::awaitable<uint32_t, login_exception>(await_callback);
}

fb::task fb::login::service::auth::__change_pw(fb::awaitable<void>& awaitable, std::string id, std::string pw, std::string new_pw, uint32_t birthday)
{
    try
    {
        const auto& config = fb::config::get();
        if(id.length() < config["name_size"]["min"].asInt() || id.length() > config["name_size"]["max"].asInt())
            throw id_exception(fb::login::message::account::INVALID_NAME);

        // Name must be full-hangul characters
        if(fb::config::get()["login"]["account option"]["allow other language"].asBool() == false && this->is_hangul(id.c_str()) == false)
            throw id_exception(fb::login::message::account::INVALID_NAME);

        // Name cannot contains subcharacters in forbidden list
        if(this->is_forbidden(id.c_str()))
            throw id_exception(fb::login::message::account::INVALID_NAME);

        if(pw.length() < config["pw_size"]["min"].asInt() || pw.length() > config["pw_size"]["max"].asInt())
            throw pw_exception(fb::login::message::account::PASSWORD_SIZE);

        auto results = co_await this->_db.co_exec_f(id, "SELECT pw, birth FROM user WHERE name='%s'", id.c_str());
        auto& result = results[0];

        if(result.count() == 0)
            throw id_exception(fb::login::message::account::NOT_FOUND_NAME);

        auto found_pw = result.get_value<std::string>(0);
        auto found_birth = result.get_value<uint32_t>(1);

        // TODO : 올바른 비밀번호인지 체크
        if(this->sha256(pw) != found_pw)
            throw pw_exception(fb::login::message::account::INVALID_PASSWORD);

        if(new_pw.length() < config["name_size"]["min"].asInt() || new_pw.length() > config["name_size"]["max"].asInt())
            throw newpw_exception(fb::login::message::account::PASSWORD_SIZE);

        // TODO : 너무 쉬운 비밀번호인지 체크
        if(pw == new_pw)
            throw newpw_exception(fb::login::message::account::NEW_PW_EQUALIZATION);

        if(birthday != found_birth)
            throw btd_exception();

        co_await this->_db.co_exec_f(id, "UPDATE user SET pw='%s' WHERE name='%s'", this->sha256(new_pw).c_str(), id.c_str());
    }
    catch(login_exception& e)
    {
        awaitable.error = std::make_unique<login_exception>(e.type(), e.what());
    }
    catch(std::exception& e)
    {
        awaitable.error = std::make_unique<login_exception>(0x0E, e.what());
    }

    awaitable.handler.resume();
}

fb::awaitable<void> fb::login::service::auth::change_pw(const std::string& id, const std::string& pw, const std::string& new_pw, uint32_t birthday)
{
    // TODO: 프로시저 호출 방식으로 수정
    auto await_callback = [this, id, pw, new_pw, birthday] (auto& awaitable)
    {
        this->__change_pw(awaitable, id, pw, new_pw, birthday);
    };

    return fb::awaitable<void>(await_callback);
}