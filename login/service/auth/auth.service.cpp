#include "auth.service.h"

fb::login::service::auth::auth()
{
    //
    //this->_gateway_data = this->make_gateway_stream(&this->_gateway_crc);

    const auto& config = fb::config();

    // Parse agreement
    for(auto x : config["login"]["account option"]["forbidden"])
        this->_forbiddens.push_back(x.asString());
}

fb::login::service::auth::~auth()
{
}

bool fb::login::service::auth::is_hangul(const char* str)
{
    auto                    size = strlen(str);
    if(size % 2 != 0)
        return false;

    std::regex re("^.+[가-힣]$");
    return std::regex_search(str, re);
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

void fb::login::service::auth::assert_client(fb::login::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd     = istream.read_u8();
    auto                    version = istream.read_u16();
    if(version != 0x0226)
        throw std::exception("클라이언트 버전이 맞지 않습니다.");

    auto                    national_key    = istream.read_u8();
    if(national_key != 0xD7)
        throw std::exception("국가가 올바르지 않습니다.");
}

void fb::login::service::auth::assert_account(const std::string& id, const std::string& pw) const
{
    if(id.length() < MIN_NAME_SIZE || id.length() > MAX_NAME_SIZE)
        throw id_exception(message::INVALID_NAME);

    // Name must be full-hangul characters
    if(fb::config()["login"]["account option"]["allow other language"].asBool() == false && this->is_hangul(id.c_str()) == false)
        throw id_exception(message::INVALID_NAME);

    // Name cannot contains subcharacters in forbidden list
    if(this->is_forbidden(id.c_str()))
        throw id_exception(message::INVALID_NAME);


    // Read character's password
    if(pw.length() < MIN_PASSWORD_SIZE || pw.length() > MAX_PASSWORD_SIZE)
        throw pw_exception(message::PASSWORD_SIZE);
}

void fb::login::service::auth::create_account(const std::string& id, const std::string& pw)
{
    this->assert_account(id, pw);
}

void fb::login::service::auth::init_account(const std::string& id, uint8_t hair, uint8_t sex, uint8_t nation, uint8_t creature)
{
    
}

void fb::login::service::auth::login(const std::string& id, const std::string& pw)
{
    this->assert_account(id, pw);
}

void fb::login::service::auth::change_pw(const std::string& id, const std::string& pw, const std::string& new_pw, uint32_t birthday)
{
    if(id.length() < MIN_NAME_SIZE || id.length() > MAX_NAME_SIZE)
        throw id_exception(message::INVALID_NAME);

    // Name must be full-hangul characters
    if(fb::config()["login"]["account option"]["allow other language"].asBool() == false && this->is_hangul(id.c_str()) == false)
        throw id_exception(message::INVALID_NAME);

    // Name cannot contains subcharacters in forbidden list
    if(this->is_forbidden(id.c_str()))
        throw id_exception(message::INVALID_NAME);


    // TODO : 존재하는 아이디인지 검사



    if(pw.length() < MIN_PASSWORD_SIZE || pw.length() > MAX_PASSWORD_SIZE)
        throw pw_exception(message::PASSWORD_SIZE);

    // TODO : 올바른 비밀번호인지 체크



    if(new_pw.length() < MIN_PASSWORD_SIZE || new_pw.length() > MAX_PASSWORD_SIZE)
        throw newpw_exception(message::PASSWORD_SIZE);
    // TODO : 너무 쉬운 비밀번호인지 체크

    if(pw == new_pw)
        throw newpw_exception(message::NEW_PW_EQUALIZATION);

    if(birthday < 100000 || birthday >= 1000000)
        throw btd_exception();
}