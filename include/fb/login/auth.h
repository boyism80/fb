#ifndef __AUTH_SERVICE_H
#define __AUTH_SERVICE_H

#include <fb/core/db.h>
#include <string>
#include <iostream>
#include <memory>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <regex>
#include <ctime>
#include <openssl/sha.h>
#include <fb/core/stream.h>
#include <fb/core/thread.h>
#include <fb/core/config.h>
#include <fb/login/session.h>
#include <fb/login/query.h>
#include <fb/core/encoding.h>
#include <fb/core/string.h>
#include <fb/game/query.h>

#define MAX_NXCLUB_SIZE     14


class login_exception : public std::runtime_error
{
private:
    uint8_t                 _exc_type;

public:
    login_exception(uint8_t type, const std::string& what) : std::runtime_error(what), _exc_type(type) { }

public:
    uint8_t                 type() const { return this->_exc_type; }
};

class id_exception : public login_exception
{
public:
    id_exception(const std::string& what) : login_exception(0x0E, what) { }
};

class pw_exception : public login_exception
{
public:
    pw_exception(const std::string& what) : login_exception(0x0F, what) { }
};

class newpw_exception : public login_exception
{
public:
    newpw_exception(const std::string& what) : login_exception(0x05, what) { }
};

class btd_exception : public login_exception
{
public:
    btd_exception() : login_exception(0x1F, fb::login::message::account::INVALID_BIRTHDAY) { }
};


namespace fb { namespace login { namespace service { 

class auth
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

    fb::task                    __create_account(fb::awaitable<void>& awaitable, std::string id, std::string pw);
    fb::task                    __login(fb::awaitable<uint32_t>& awaitable, std::string id, std::string pw);
    fb::task                    __change_pw(fb::awaitable<void>& awaitable, std::string id, std::string pw, std::string new_pw, uint32_t birthday);

public:
    auto                        exists(const std::string& name);

    void                        assert_account(const std::string& id, const std::string& pw) const;
    fb::awaitable<void>         create_account(const std::string& id, const std::string& pw);
    void                        init_account(const std::string& id, uint8_t hair, uint8_t sex, uint8_t nation, uint8_t creature);
    fb::awaitable<uint32_t>     login(const std::string& id, const std::string& pw);
    fb::awaitable<void>         change_pw(const std::string& id, const std::string& pw, const std::string& new_pw, uint32_t birthday);
};

} } }
#endif // !__AUTH_SERVICE_H
