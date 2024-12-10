#ifndef __FB_LOGIN_H__
#define __FB_LOGIN_H__

#include <string>
#include <iostream>
#include <memory>
#include <fstream>
#include <json/json.h>
#include <regex>
#include <ctime>
#include <zlib.h>
#include <fb/socket.h>
#include <session.h>
#include <gateway.h>
#include <fb/protocol/login.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/acceptor.h>
#include <fb/string.h>
#include <fb/model/model.h>

#define MAX_NXCLUB_SIZE 14

using namespace fb::protocol::login;
using namespace fb::model::enum_value;

namespace internal = fb::protocol::internal;

namespace fb { namespace login {

/**
 * @brief      Exception for signaling login errors.
 */
class login_exception : public std::runtime_error
{
private:
    uint8_t _exc_type;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  type  The type
     * @param[in]  what  The what
     */
    login_exception(uint8_t type, const std::string& what) :
        std::runtime_error(what),
        _exc_type(type)
    { }

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t type() const
    {
        return this->_exc_type;
    }
};

/**
 * @brief      Exception for signaling identifier errors.
 */
class id_exception : public login_exception
{
public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  what  The what
     */
    id_exception(const std::string& what) :
        login_exception(0x0E, what)
    { }
};

/**
 * @brief      Exception for signaling password errors.
 */
class pw_exception : public login_exception
{
public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  what  The what
     */
    pw_exception(const std::string& what) :
        login_exception(0x0F, what)
    { }
};

/**
 * @brief      Exception for signaling newpw errors.
 */
class newpw_exception : public login_exception
{
public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  what  The what
     */
    newpw_exception(const std::string& what) :
        login_exception(0x05, what)
    { }
};

/**
 * @brief      Exception for signaling btd errors.
 */
class btd_exception : public login_exception
{
public:
    /**
     * @brief      Constructs a new instance.
     */
    btd_exception() :
        login_exception(0x1F, fb::login::message::account::INVALID_BIRTHDAY)
    { }
};

/**
 * @brief      This class describes a context.
 */
class context : public fb::acceptor<fb::login::session>
{
public:
    using unique_session = std::unique_ptr<fb::login::session>;

private:
    fb::protocol::login::response::agreement _agreement = CP949(fb::config<std::string>("agreement"), PLATFORM::Both);
    std::vector<std::string>                 _forbiddens;
    std::vector<unique_session>              _sessions;
    std::vector<boost::asio::deadline_timer> _timers;

public:
    fb::model::model model;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  port     The port
     */
    context(boost::asio::io_context& context, uint16_t port);
    /**
     * @brief      Destroys the object.
     */
    ~context();

private:
    /**
     * @brief      Determines whether the specified string is forbidden.
     *
     * @param[in]  str   The string
     *
     * @return     True if the specified string is forbidden, False otherwise.
     */
    bool is_forbidden(const std::string& str) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  id    The identifier
     * @param[in]  pw    The password
     */
    void assert_account(const std::string& id, const std::string& pw) const;

    // override

protected:
    /**
     * @brief      { function_description }
     *
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool decrypt_policy(uint8_t) const final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_start() final;

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::login::session* handle_accepted(fb::socket<fb::login::session>&) final;
    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_connected(fb::socket<fb::login::session>&) final;
    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_disconnected(fb::socket<fb::login::session>&) final;

    // for heart-beat

protected:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::protocol::internal::Service service() const final
    {
        return fb::protocol::internal::Service::Login;
    };

public:
    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_agreement(fb::socket<fb::login::session>&,
                                                     const fb::protocol::login::request::agreement&);
    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_create_account(fb::socket<fb::login::session>&,
                                                          const fb::protocol::login::request::account::create&);
    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_account_complete(fb::socket<fb::login::session>&,
                                                            const fb::protocol::login::request::account::complete&);
    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_login(fb::socket<fb::login::session>&,
                                                 const fb::protocol::login::request::login&);
    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_change_password(fb::socket<fb::login::session>&,
                                                           const fb::protocol::login::request::account::change_pw&);
};

}} // namespace fb::login

#endif // !__FB_LOGIN_H__
