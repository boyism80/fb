#ifndef __FB_LOGIN_H__
#define __FB_LOGIN_H__

/**
 * @file    server.h
 * @brief   Login server for user authentication and character management
 * @author  FB Development Team
 *
 * @details This file implements the login server that handles user authentication,
 *          account management, character creation, and game server connection routing
 *          in the FB 2D MMORPG distributed architecture. The login server serves as
 *          the authentication gateway between clients and game servers.
 *
 *          Key features:
 *          - Comprehensive user authentication with secure credential validation
 *          - Account creation and management with validation and security checks
 *          - Character creation, deletion, and selection with name reservation
 *          - Password change functionality with security validation
 *          - Game server connection routing and load balancing
 *          - Redis integration for session management and caching
 *          - AMQP messaging for inter-service communication
 *          - Database integration for persistent account and character data
 *          - Security features including forbidden name filtering and validation
 *          - Session management with proper authentication state tracking
 *          - Exception handling for various authentication and validation scenarios
 *          - Agreement and terms of service presentation to clients
 *
 * @note    The login server is a critical component in the distributed architecture,
 *          responsible for user authentication and character management before
 *          routing clients to appropriate game servers.
 */

#include <string>
#include <iostream>
#include <memory>
#include <fstream>
#include <json/json.h>
#include <regex>
#include <ctime>
#include <zlib.h>
#include <fb/socket.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/acceptor.h>
#include <fb/model/model.h>
#include <fb/login/session.h>
#include <fb/login/gateway.h>
#include <fb/login/protocol.h>
#include <fb/redis.h>

#define MAX_NXCLUB_SIZE 14

using namespace fb::protocol::login;
using namespace fb::model::enum_value;
using namespace fb::protocol::internal::request;

namespace internal      = fb::protocol::internal;
namespace internal_resp = fb::protocol::internal::response;

REGISTER_RESPONSE(fb::protocol::internal::request::ReserveName, fb::protocol::internal::response::ReserveName)
REGISTER_RESPONSE(fb::protocol::internal::request::InitCharacter, fb::protocol::internal::response::InitCharacter)
REGISTER_RESPONSE(fb::protocol::internal::request::MakeCharacter, fb::protocol::internal::response::MakeCharacter)
REGISTER_RESPONSE(fb::protocol::internal::request::Authenticate, fb::protocol::internal::response::Authenticate)
REGISTER_RESPONSE(fb::protocol::internal::request::ChangePw, fb::protocol::internal::response::ChangePw)
REGISTER_RESPONSE(fb::protocol::internal::request::Transfer, fb::protocol::internal::response::Transfer)

namespace fb { namespace login {

/**
 * @brief      Exception for signaling login errors.
 *
 *             Base exception class for all login-related errors. Contains
 *             an error type code that corresponds to specific error messages
 *             sent to the client.
 */
class login_exception : public std::runtime_error
{
private:
    uint8_t _exc_type;

public:
    /**
     * @brief      Constructs a new login exception.
     *
     * @param[in]  type  The error type code to send to the client.
     * @param[in]  what  The error message description.
     */
    login_exception(uint8_t type, const std::string& what) :
        std::runtime_error(what),
        _exc_type(type)
    { }

public:
    /**
     * @brief      Gets the error type code.
     *
     * @return     The error type code for client communication.
     */
    uint8_t type() const
    {
        return this->_exc_type;
    }
};

/**
 * @brief      Exception for signaling identifier (username) errors.
 *
 *             Thrown when there are issues with the username during
 *             account creation or login, such as invalid characters,
 *             forbidden names, or length violations.
 */
class id_exception : public login_exception
{
public:
    /**
     * @brief      Constructs a new identifier exception.
     *
     * @param[in]  what  The error message description.
     */
    id_exception(const std::string& what) :
        login_exception(0x0E, what)
    { }
};

/**
 * @brief      Exception for signaling password errors.
 *
 *             Thrown when there are issues with the password during
 *             account creation or login, such as invalid length or
 *             incorrect password.
 */
class pw_exception : public login_exception
{
public:
    /**
     * @brief      Constructs a new password exception.
     *
     * @param[in]  what  The error message description.
     */
    pw_exception(const std::string& what) :
        login_exception(0x0F, what)
    { }
};

/**
 * @brief      Exception for signaling new password errors.
 *
 *             Thrown when there are issues with new password validation
 *             during password change operations.
 */
class newpw_exception : public login_exception
{
public:
    /**
     * @brief      Constructs a new password exception.
     *
     * @param[in]  what  The error message description.
     */
    newpw_exception(const std::string& what) :
        login_exception(0x05, what)
    { }
};

/**
 * @brief      Exception for signaling birthday validation errors.
 *
 *             Thrown when birthday validation fails during account
 *             creation or verification processes.
 */
class btd_exception : public login_exception
{
public:
    /**
     * @brief      Constructs a new birthday exception with default message.
     */
    btd_exception() :
        login_exception(0x1F, _TEXT(MESSAGE_ACCOUNT_INVALID_BIRTHDAY))
    { }
};

/**
 * @brief      The main login server that handles client connections and authentication.
 *
 *             This class extends the acceptor to provide login server functionality.
 *             It manages client sessions, handles authentication requests, account creation,
 *             password changes, and communicates with the database and other services
 *             through Redis and AMQP messaging.
 */
class server : public fb::acceptor<fb::login::session>
{
private:
    fb::protocol::login::response::agreement _agreement = CP949(fb::config<std::string>("agreement"), PLATFORM::BOTH);
    std::vector<std::string>                 _forbiddens;
    std::vector<boost::asio::deadline_timer> _timers;
    fb::redis                                _redis;

    /**
     * @brief      Determines whether the specified string is forbidden.
     *
     * @param[in]  str   The string to check against the forbidden list.
     *
     * @return     True if the specified string is forbidden, False otherwise.
     */
    bool is_forbidden_impl(const std::string& str) const;

public:
    fb::model::model model;

public:
    /**
     * @brief      Constructs a new login server.
     *
     * @param      io_context   The boost::asio I/O context for network operations.
     * @param[in]  port         The port number to listen on for client connections.
     */
    server(boost::asio::io_context& io_context, uint16_t port);

    /**
     * @brief      Destroys the login server.
     */
    ~server();

    /**
     * @brief      Gets the agreement response for clients.
     *
     * @return     The agreement response containing terms of service.
     */
    const fb::protocol::login::response::agreement& agreement() const;

    /**
     * @brief      Determines whether the specified string is forbidden.
     *
     * @param[in]  str   The string to check against the forbidden list.
     *
     * @return     True if the specified string is forbidden, False otherwise.
     */
    bool is_forbidden(const std::string& str) const;

    /**
     * @brief      Validates account credentials and throws exceptions for invalid data.
     *
     * @param[in]  id    The username to validate.
     * @param[in]  pw    The password to validate.
     *
     * @throws     id_exception if username is invalid.
     * @throws     pw_exception if password is invalid.
     */
    void assert_account(const std::string& id, const std::string& pw) const;

    // override

protected:
    /**
     * @brief      Determines the decryption policy for incoming packets.
     *
     * @param[in]  cmd   The command byte of the incoming packet.
     *
     * @return     True if the packet should be decrypted, false otherwise.
     */
    bool decrypt_policy(uint8_t cmd) const override final;

    /**
     * @brief      Handles AMQP queue declaration for inter-service communication.
     *
     * @param      amqp  The AMQP socket for message queue operations.
     */
    void handle_init_amqp(fb::amqp::socket& amqp) override final;

    /**
     * @brief      Handles server startup initialization.
     *
     * @return     An async task that completes when startup is finished.
     */
    [[nodiscard]] async::task<void> handle_start() final;

    /**
     * @brief      Handles periodic heartbeat operations.
     *
     *             Sends heartbeat information to Redis to indicate server status
     *             and availability to other services.
     *
     * @return     An async task that completes when heartbeat is sent.
     */
    [[nodiscard]] async::task<void> handle_heart_beat();

    /**
     * @brief      Handles new client connections.
     *
     * @param      socket  The socket representing the new client connection.
     *
     * @return     A pointer to the created session object.
     */
    std::shared_ptr<fb::login::session> handle_accepted(fb::socket<fb::login::session>& socket) override final;

    /**
     * @brief      Handles client connection establishment.
     *
     * @param      socket  The socket representing the client connection.
     *
     * @return     An async task that returns true if connection should be maintained.
     */
    [[nodiscard]] async::task<bool> handle_connected(fb::socket<fb::login::session>& socket) final;

    /**
     * @brief      Handles client disconnection.
     *
     * @param      socket  The socket representing the disconnected client.
     *
     * @return     An async task that returns false to indicate disconnection.
     */
    [[nodiscard]] async::task<bool> handle_disconnected(fb::socket<fb::login::session>& socket) final;

    /**
     * @brief      Handles AMQP shutdown messages from other services.
     *
     * @param[in]  response  The shutdown response message.
     *
     * @return     An async task that completes when shutdown handling is finished.
     */
    [[nodiscard]] async::task<void> handle_amqp_shutdown(const internal_resp::Shutdown& response);

    // for heart-beat

protected:
    /**
     * @brief      Gets the service type identifier for this server.
     *
     * @return     The service type (Login) for heartbeat and identification.
     */
    fb::protocol::internal::Service service() const override final
    {
        return fb::protocol::internal::Service::Login;
    };

public:
};

}} // namespace fb::login

#endif // !__FB_LOGIN_H__
