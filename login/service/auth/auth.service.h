#ifndef __AUTH_SERVICE_H
#define __AUTH_SERVICE_H

#include <string>
#include <iostream>
#include <memory>
#include <fstream>
#include <json/json.h>
#include <regex>
#include <ctime>
#include <locale>
#include <codecvt>
#include <module/stream/stream.h>
#include <module/config/config.h>
#include <model/session/session.login.h>
#include <module/service/service.h>
#include <model/gateway/gateway.h>
#include <mysql+++/mysql+++.h>
#include <openssl/sha.h>
using namespace daotk::mysql;

#define MIN_NAME_SIZE       4   // sizeof(wchar_t) * 2
#define MAX_NAME_SIZE       12  // sizeof(wchar_t) * 6
#define MIN_PASSWORD_SIZE   4
#define MAX_PASSWORD_SIZE   8
#define MAX_NXCLUB_SIZE     14

#pragma region message
namespace message
{
    static const char*      INVALID_NAME                = "이름이 길거나 적합하지 않습니다.";
    static const char*      NOT_FOUND_NAME              = "존재하지 않는 이름입니다.";
    static const char*      PASSWORD_SIZE               = "암호는 4자 이상 8자 이하";
    static const char*      INVALID_PASSWORD            = "비밀번호가 올바르지 않습니다.";
    static const char*      SIMPLE_PASSWORD             = "암호가 단순합니다.";
    static const char*      SUCCESS_REGISTER_ACCOUNT    = "등록완료, 이어하기를 선택하세요.";
    static const char*      SUCCESS_CHANGE_PASSWORD     = "변경됐다리";
    static const char*      INVALID_BIRTHDAY            = "생년월일이 올바르지 않습니다.";
    static const char*      NEW_PW_EQUALIZATION         = "기존 암호화 동일합니다.";
    static const char*      ALREADY_EXISTS              = "이미 존재하는 이름입니다.";
}
#pragma endregion

#pragma region exceptions

class login_exception : public std::exception
{
private:
    uint8_t                 _exc_type;

public:
    login_exception(uint8_t exc_type, const char* make_message_stream) : std::exception(make_message_stream), _exc_type(exc_type) {}

public:
    uint8_t                 exc_type() const { return this->_exc_type; }
};

class id_exception : public login_exception
{
public:
    id_exception(const char* make_message_stream) : login_exception(0x0E, make_message_stream) {}
};

class pw_exception : public login_exception
{
public:
    pw_exception(const char* make_message_stream) : login_exception(0x0F, make_message_stream) {}
};

class newpw_exception : public login_exception
{
public:
    newpw_exception(const char* make_message_stream) : login_exception(0x05, make_message_stream) {}
};

class btd_exception : public login_exception
{
public:
    btd_exception() : login_exception(0x1F, message::INVALID_BIRTHDAY) {}
};

#pragma endregion

namespace fb { namespace login { namespace service { 

class auth : public service
{
private:
    connection*                 _connection;
    std::vector<std::string>    _forbiddens;

public:
    auth();
    ~auth();

private:
    static bool                 is_hangul(const std::string& str);
    bool                        is_forbidden(const char* str) const;
    std::string                 sha256(const std::string& data) const;

public:
    bool                        exists(const std::string& name) const;

    void                        assert_client(fb::login::session& session);
    void                        assert_account(const std::string& id, const std::string& pw) const;
    void                        create_account(const std::string& id, const std::string& pw);
    void                        init_account(const std::string& id, uint8_t hair, uint8_t sex, uint8_t nation, uint8_t creature);
    void                        login(const std::string& id, const std::string& pw);
    void                        change_pw(const std::string& id, const std::string& pw, const std::string& new_pw, uint32_t birthday);
};

} } }
#endif // !__AUTH_SERVICE_H
