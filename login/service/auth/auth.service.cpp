#include "auth.service.h"

fb::login::service::auth::auth()
{
    const auto& config = fb::config::get();

    this->_connection = new connection(
        config["database"]["ip"].asString(), 
        config["database"]["uid"].asString(), 
        config["database"]["pwd"].asString(), 
        config["database"]["name"].asString());

    for(auto x : config["forbidden"])
        this->_forbiddens.push_back(x.asString());
}

fb::login::service::auth::~auth()
{
    delete this->_connection;
}

// 참고자료
// https://gala04.tistory.com/entry/%EC%A0%9C%EB%AA%A9%EC%9D%84-%EC%9E%85%EB%A0%A5%ED%95%B4-%EC%A3%BC%EC%84%B8%EC%9A%94
bool fb::login::service::auth::is_hangul(const std::string& str)
{
    auto cp949 = CP949(str);
    auto len = cp949.length();
    if(len % 2 > 0)
        return false;

    auto raw = cp949.c_str();
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

bool fb::login::service::auth::is_forbidden(const char* str) const
{
    return std::any_of
    (
        this->_forbiddens.cbegin(), 
        this->_forbiddens.cend(), 
        [str](const std::string& x)
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

    uint8_t hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &ctx);
    
    std::stringstream sstream;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sstream << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];

    return sstream.str();
}

bool fb::login::service::auth::exists(const std::string& name) const
{
    return this->_connection->query
    (
        "SELECT COUNT(*) FROM user WHERE name='%s'",
        name.c_str()
    ).get_value<int>() > 0;
}

void fb::login::service::auth::assert_account(const std::string& id, const std::string& pw) const
{
    auto name_size = CP949(id).length();
    if(name_size < MIN_NAME_SIZE || name_size > MAX_NAME_SIZE)
        throw id_exception(CP949(fb::login::message::account::INVALID_NAME));

    // Name must be full-hangul characters
    if(fb::config::get()["allow other language"].asBool() == false && this->is_hangul(id) == false)
        throw id_exception(CP949(fb::login::message::account::INVALID_NAME));

    // Name cannot contains subcharacters in forbidden list
    if(this->is_forbidden(id.c_str()))
        throw id_exception(CP949(fb::login::message::account::INVALID_NAME));


    // Read character's password
    if(pw.length() < MIN_PASSWORD_SIZE || pw.length() > MAX_PASSWORD_SIZE)
        throw pw_exception(CP949(fb::login::message::account::PASSWORD_SIZE));
}

void fb::login::service::auth::create_account(const std::string& id, const std::string& pw)
{
    this->assert_account(id, pw);

    if(this->exists(id))
        throw id_exception(CP949(fb::login::message::account::ALREADY_EXISTS));

    auto& config = fb::config::get();

    std::srand(std::time(nullptr));
    auto hp = config["init"]["hp"]["base"].asInt() + std::rand() % config["init"]["hp"]["range"].asInt();
    auto mp = config["init"]["mp"]["base"].asInt() + std::rand() % config["init"]["mp"]["range"].asInt();
    this->_connection->exec
    (
        "INSERT INTO user(name, pw, hp, base_hp, mp, base_mp, map, position_x, position_y) VALUES('%s', '%s', %d, %d, %d, %d, %d, %d, %d)",
        id.c_str(), this->sha256(pw).c_str(), hp, hp, mp, mp, config["init"]["map"].asInt(), config["init"]["position"]["x"].asInt(), config["init"]["position"]["y"].asInt()
    );
    auto index = this->_connection->last_insert_id();

    auto result = this->_connection->query("SELECT id, name FROM user")
        .each([](int id, std::string name) 
            {
                return true;
            });
}

void fb::login::service::auth::init_account(const std::string& id, uint8_t hair, uint8_t sex, uint8_t nation, uint8_t creature)
{
    this->_connection->exec
    (
        "UPDATE user SET look=%d, sex=%d, nation=%d, creature=%d WHERE name='%s'",
        hair, sex, nation, creature, id.c_str()
    );
}

void fb::login::service::auth::login(const std::string& id, const std::string& pw)
{
    this->assert_account(id, pw);

    auto found = this->_connection->query
    (
        "SELECT pw FROM user WHERE name='%s' LIMIT 1",
        id.c_str(), this->sha256(pw).c_str()
    );

    if(found.count() == 0)
        throw id_exception(CP949(fb::login::message::account::NOT_FOUND_NAME));

    if(found.get_value<std::string>(0) != this->sha256(pw))
        throw pw_exception(CP949(fb::login::message::account::INVALID_PASSWORD));
}

void fb::login::service::auth::change_pw(const std::string& id, const std::string& pw, const std::string& new_pw, uint32_t birthday)
{
    if(id.length() < MIN_NAME_SIZE || id.length() > MAX_NAME_SIZE)
        throw id_exception(CP949(fb::login::message::account::INVALID_NAME));

    // Name must be full-hangul characters
    if(fb::config::get()["login"]["account option"]["allow other language"].asBool() == false && this->is_hangul(id.c_str()) == false)
        throw id_exception(CP949(fb::login::message::account::INVALID_NAME));

    // Name cannot contains subcharacters in forbidden list
    if(this->is_forbidden(id.c_str()))
        throw id_exception(CP949(fb::login::message::account::INVALID_NAME));

    if(pw.length() < MIN_PASSWORD_SIZE || pw.length() > MAX_PASSWORD_SIZE)
        throw pw_exception(CP949(fb::login::message::account::PASSWORD_SIZE));

    auto found = this->_connection->query
    (
        "SELECT pw, birth FROM user WHERE name='%s'",
        id.c_str()
    );

    if(found.count() == 0)
        throw id_exception(CP949(fb::login::message::account::NOT_FOUND_NAME));

    auto found_pw = found.get_value<std::string>(0);
    auto found_birth = found.get_value<uint32_t>(1);

    // TODO : 올바른 비밀번호인지 체크
    if(this->sha256(pw) != found_pw)
        throw pw_exception(CP949(fb::login::message::account::INVALID_PASSWORD));

    if(new_pw.length() < MIN_PASSWORD_SIZE || new_pw.length() > MAX_PASSWORD_SIZE)
        throw newpw_exception(CP949(fb::login::message::account::PASSWORD_SIZE));
    
    // TODO : 너무 쉬운 비밀번호인지 체크
    if(pw == new_pw)
        throw newpw_exception(CP949(fb::login::message::account::NEW_PW_EQUALIZATION));

    if(birthday != found_birth)
        throw btd_exception();

    this->_connection->exec
    (
        "UPDATE user SET pw='%s' WHERE name='%s'",
        this->sha256(new_pw).c_str(), id.c_str()
    );
}