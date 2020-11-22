#ifndef __AUTH_SERVICE_H
#define __AUTH_SERVICE_H

#include "module/db/db.h"
#include <string>
#include <iostream>
#include <memory>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <regex>
#include <ctime>
#include <openssl/sha.h>
#include "module/stream/stream.h"
#include "module/config/config.h"
#include "model/session/session.login.h"
#include "module/service/service.h"
#include "model/gateway/gateway.h"
#include "module/encoding/encoding.h"
#include "module/string/string.h"

#define MIN_NAME_SIZE       4   // sizeof(wchar_t) * 2
#define MAX_NAME_SIZE       12  // sizeof(wchar_t) * 6
#define MIN_PASSWORD_SIZE   4
#define MAX_PASSWORD_SIZE   8
#define MAX_NXCLUB_SIZE     14

#pragma region exceptions

class login_exception : public std::runtime_error
{
private:
    uint8_t                 _exc_type;

public:
    login_exception(uint8_t type, const std::string& what) : std::runtime_error(what), _exc_type(type) {}

public:
    uint8_t                 type() const { return this->_exc_type; }
};

class id_exception : public login_exception
{
public:
    id_exception(const std::string& what) : login_exception(0x0E, what) {}
};

class pw_exception : public login_exception
{
public:
    pw_exception(const std::string& what) : login_exception(0x0F, what) {}
};

class newpw_exception : public login_exception
{
public:
    newpw_exception(const std::string& what) : login_exception(0x05, what) {}
};

class btd_exception : public login_exception
{
public:
    btd_exception() : login_exception(0x1F, fb::login::message::account::INVALID_BIRTHDAY) {}
};

#pragma endregion

namespace fb { namespace login { namespace service { 

class auth : public service
{
private:
    std::vector<std::string>    _forbiddens;

public:
    auth();
    ~auth();

private:
    static bool                 is_hangul(const std::string& str);
    bool                        is_forbidden(const std::string& str) const;
    std::string                 sha256(const std::string& data) const;

public:
    bool                        exists(const std::string& name) const;

    void                        assert_account(const std::string& id, const std::string& pw) const;
    void                        create_account(const std::string& id, const std::string& pw);
    void                        init_account(const std::string& id, uint8_t hair, uint8_t sex, uint8_t nation, uint8_t creature);
    uint32_t                    login(const std::string& id, const std::string& pw);
    void                        change_pw(const std::string& id, const std::string& pw, const std::string& new_pw, uint32_t birthday);
};

} } }
#endif // !__AUTH_SERVICE_H
