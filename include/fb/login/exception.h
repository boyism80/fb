#ifndef __FB_LOGIN_EXCEPTION_H__
#define __FB_LOGIN_EXCEPTION_H__

#include <fb/model/model.h>
#include <macro.h>
#include <stdexcept>
#include <string>
#include <string_view>

namespace fb::login {

class login_exception : public std::runtime_error
{
private:
    uint8_t _exc_type;

public:
    login_exception(uint8_t type, std::string_view what) :
        std::runtime_error(std::string(what)),
        _exc_type(type)
    { }

public:
    uint8_t type() const
    {
        return this->_exc_type;
    }
};

class id_exception : public login_exception
{
public:
    id_exception(std::string_view what) :
        login_exception(0x0E, what)
    { }
};

class pw_exception : public login_exception
{
public:
    pw_exception(std::string_view what) :
        login_exception(0x0F, what)
    { }
};

class newpw_exception : public login_exception
{
public:
    newpw_exception(std::string_view what) :
        login_exception(0x05, what)
    { }
};

class btd_exception : public login_exception
{
public:
    btd_exception() :
        login_exception(0x1F, _TEXT(MESSAGE_ACCOUNT_INVALID_BIRTHDAY))
    { }
};

} // namespace fb::login

#endif // !__FB_LOGIN_EXCEPTION_H__
