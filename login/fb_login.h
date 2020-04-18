#ifndef __FB_LOGIN_H__
#define __FB_LOGIN_H__

#include <string>
#include <iostream>
#include <memory>
#include <fstream>
#include "socket.h"
#include "acceptor.h"
#include "json/json.h"

#define MIN_NAME_SIZE       4   // sizeof(wchar_t) * 2
#define MAX_NAME_SIZE       12  // sizeof(wchar_t) * 6
#define MIN_PASSWORD_SIZE   4
#define MAX_PASSWORD_SIZE   8
#define MAX_NXCLUB_SIZE     14

namespace fb { namespace login {

#pragma region message
namespace message
{
    static const char*      INVALID_NAME                = "이름이 길거나 적합하지 않습니다.";
    static const char*      PASSWORD_SIZE               = "암호는 4자 이상 8자 이하";
    static const char*      SIMPLE_PASSWORD             = "암호가 단순합니다.";
    static const char*      SUCCESS_REGISTER_ACCOUNT    = "등록완료, 이어하기를 선택하세요.";
    static const char*      SUCCESS_CHANGE_PASSWORD     = "변경됐다리";
    static const char*      INVALID_BIRTHDAY            = "생년월일이 올바르지 않습니다.";
    static const char*      NEW_PW_EQUALIZATION         = "기존 암호화 동일합니다.";
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


class session : public crtsocket
{
public:
    session(SOCKET fd);
    ~session();
};


class acceptor : public fb_acceptor<session>
{
private:
    typedef struct _gateway_form
    {
    public:
        std::string             name;
        std::string             desc;
        uint32_t                ip;
        uint16_t                port;

    public:
        _gateway_form(const char* name, const char* desc, uint32_t ip, uint16_t port) : name(name), desc(desc), ip(ip), port(port) { }
        _gateway_form(const char* name, const char* desc, const char* ip, uint16_t port) : _gateway_form(name, desc, inet_addr(ip), port) { }
        ~_gateway_form() {}
    } gateway_form;

public:
    typedef std::vector<std::unique_ptr<gateway_form>>  multi_server_list;

private:
    Json::Value                 _config;

    uint32_t                    _gateway_crc;
    multi_server_list           _gateway_list;
    ostream                     _gateway_data;
    ostream                     _agreement_data;
    std::vector<std::string>    _forbidden_names;

public:
    acceptor(uint16_t port);
    ~acceptor();

    // override
public:
    login::session*             handle_allocate_session(SOCKET fd);

private:
    uint32_t                    compress(const uint8_t * source, uint32_t size, uint8_t * dest) const;
    fb::ostream                 make_gateway_stream(uint32_t* crc) const;
    fb::ostream                 make_agreement_stream() const;
    const ostream               make_message_stream(int type, const char* msg) const;

private:
    static bool                 is_hangul(const char* str);
    bool                        is_forbidden(const char* str);

public:
    bool                        handle_connected(fb::login::session& session);
    bool                        handle_disconnected(fb::login::session& session);

public:
    bool                        handle_check_version(fb::login::session& session);
    bool                        handle_gateway_list(fb::login::session& session);
    bool                        handle_agreement(fb::login::session& session);
    bool                        handle_check_id(fb::login::session& session);
    bool                        handle_create_account(fb::login::session& session);
    bool                        handle_login(fb::login::session& session);
    bool                        handle_change_password(fb::login::session& session);
};

} }

#endif // !__FB_LOGIN_H__
