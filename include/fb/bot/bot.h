#ifndef __BOT_H__
#define __BOT_H__

/**
 * @file    bot.h
 * @brief   Automated game client bot system for testing and load simulation
 * @author  FB Development Team
 *
 * @details This file implements the core bot system that provides automated game client
 *          functionality for the FB 2D MMORPG server. Bots can simulate real player
 *          behavior for testing, load testing, debugging, and development purposes.
 *          The system supports multiple bot types with configurable behaviors and
 *          protocol handling capabilities.
 *
 *          Key features:
 *          - Base bot class with extensible architecture for different bot types
 *          - Protocol handling system with automatic message routing and processing
 *          - Request-response pattern support with temporary hooks and callbacks
 *          - Cryptographic integration for secure server communication
 *          - Configurable bot behaviors with timer-based actions and events
 *          - Connection management with automatic reconnection and error handling
 *          - Template-based protocol binding for type-safe message handling
 *          - Hook system for intercepting and processing specific protocol messages
 *          - Load testing capabilities with concurrent bot management
 *          - Debugging and development support with detailed logging and monitoring
 *          - Integration with game server protocol stack for realistic simulation
 *          - Support for complex gameplay scenarios and automated testing workflows
 *
 * @note    The bot system is essential for server testing and development,
 *          providing realistic load simulation and automated testing capabilities
 *          that help ensure server stability and performance under various conditions.
 */

#include <random>
#include <any>
#include <fb/socket.h>
#include <async/awaitable_get.h>
#include <boost/endian/conversion.hpp>
#include <fb/model/model.h>
#include <fb/bot/hook_params.h>

using namespace std::chrono_literals;
using namespace fb::model;

namespace fb::bot {

// Forward declarations
class bot_container;
class base_bot_controller;
template <typename BotType> class bot_controller;

/**
 * @brief      Base class for automated game client bots.
 *
 *             This class provides the foundation for creating automated game clients
 *             that can connect to game servers and perform automated actions. It extends
 *             the socket functionality with protocol handling, message hooks, and
 *             request-response patterns. Bots can be used for testing, load simulation,
 *             or automated gameplay scenarios.
 */
class base_bot : public fb::socket<void*>
{
private:
    using hook_func      = std::function<bool(const fb::protocol::header&)>;      ///< Hook condition function type
    using hook_container = std::unordered_map<uint8_t, std::vector<hook_params>>; ///< Container for protocol hooks

protected:
    hook_container       _hooks;          ///< Temporary hooks for request-response patterns
    fb::context&         _context;        ///< Reference to the context for socket operations
    base_bot_controller& _bot_controller; ///< Reference to the bot_controller managing this bot

public:
    const uint32_t id; ///< Unique identifier for this bot instance

protected:
    /**
     * @brief      Constructs a new bot instance.
     *
     * @param      context     The context for socket operations.
     * @param      bot_controller  The bot_controller managing this bot.
     * @param[in]  on_receive  Callback function for received messages.
     * @param[in]  on_closed   Callback function for connection closure.
     * @param[in]  id          The unique identifier for this bot.
     */
    base_bot(fb::context&                                                      context,
             base_bot_controller&                                              bot_controller,
             std::function<async::task<void>(fb::socket<void*>&, fb::stream&)> on_receive,
             std::function<async::task<void>(fb::socket<void*>&)>              on_closed,
             uint32_t                                                          id);

public:
    /**
     * @brief      Virtual destructor for proper inheritance cleanup.
     */
    virtual ~base_bot();

protected:
    /**
     * @brief      Applies encryption to outgoing data streams.
     *
     *             Override this method to implement bot-specific encryption
     *             logic for securing communication with the server.
     *
     * @param      out  The output stream to encrypt.
     *
     * @return     True if encryption was applied successfully, false otherwise.
     */
    virtual bool on_encrypt(fb::stream& out);

    /**
     * @brief      Applies protocol wrapping to outgoing data streams.
     *
     *             Override this method to implement bot-specific protocol
     *             wrapping, such as adding headers or formatting data.
     *
     * @param      out  The output stream to wrap.
     *
     * @return     True if wrapping was applied successfully, false otherwise.
     */
    virtual bool on_wrap(fb::stream& out);

public:
    /**
     * @brief      Initiates a connection to the specified server endpoint.
     *
     *             Establishes a TCP connection to the given endpoint and begins
     *             the bot's communication session with the server.
     *
     * @param[in]  endpoint  The server endpoint to connect to.
     */
    void connect(const boost::asio::ip::tcp::endpoint& endpoint);

    /**
     * @brief      Processes temporary hooks for the given protocol message.
     *
     *             Checks if there are any temporary hooks registered for the
     *             protocol command type. If a matching hook is found based on
     *             its condition function, the hook's callback is executed and
     *             the hook is removed from the collection.
     *
     * @param[in]  cmd     The protocol command identifier.
     * @param      header  The protocol message to check against hooks.
     *
     * @return     True if a hook was found and processed, false otherwise.
     */
    bool process_hooks(uint8_t cmd, fb::protocol::header& header);

    /**
     * @brief      Removes a hook by its context pointer.
     *
     *             Removes the first hook found with the matching context pointer
     *             from the specified command's hook collection. Used primarily
     *             for timeout scenarios where specific hooks need to be cleaned up.
     *
     * @param[in]  cmd          The protocol command identifier.
     * @param[in]  context_ptr  The context pointer to match against.
     *
     * @return     True if a hook was found and removed, false otherwise.
     */
    bool remove_hook_by_context(uint8_t cmd, const void* context_ptr);

public:
    /**
     * @brief      Gets the thread associated with this bot.
     *
     * @return     Pointer to the thread managing this bot's execution.
     */
    virtual fb::thread* thread() const;
};

/**
 * @brief      Template bot class with bot_controller-specific functionality.
 *
 *             This template class extends base_bot with bot_controller-specific
 *             functionality, providing type-safe access to the bot_controller
 *             and implementing request-response patterns.
 *
 * @tparam     BotType  The specific bot type (CRTP pattern).
 */
template <typename BotType>
class bot : public base_bot
{
public:
    /**
     * @brief      Request context for managing timed requests with automatic cleanup.
     *
     *             RAII-style structure that manages the lifecycle of a request with timeout.
     *             Handles mutual dependencies between timer and hook callbacks safely.
     */
    template <typename ResponseType>
    struct request_context
    {
        std::shared_ptr<async::task_completion_source<ResponseType>> promise;
        std::shared_ptr<fb::timer>                                   timer;
        std::weak_ptr<BotType>                                       bot_weak;
        uint8_t                                                      hook_cmd;
        std::atomic<bool>                                            completed{false};
        const void*                                                  context_ptr; ///< Self-reference for hook removal

        /**
         * @brief      Constructs a new request context.
         *
         * @param[in]  bot  The bot instance (converted to weak_ptr for safe access).
         * @param[in]  cmd  The protocol command for hook management.
         */
        request_context(std::shared_ptr<BotType> bot, uint8_t cmd) :
            promise(std::make_shared<async::task_completion_source<ResponseType>>()),
            bot_weak(bot),
            hook_cmd(cmd),
            context_ptr(this)
        { }

        /**
         * @brief      Completes the request successfully.
         *
         *             Thread-safe completion that cancels timer and resolves promise.
         *             Uses atomic flag to prevent double completion.
         *
         * @param[in]  response  The response to return.
         */
        void complete_success(const ResponseType& response)
        {
            if (completed.exchange(true))
                return; // Already completed

            if (timer)
                timer->cancel();
            promise->set_value(response);
        }

        /**
         * @brief      Completes the request with timeout.
         *
         *             Thread-safe timeout completion that removes hooks and rejects promise.
         *             Uses atomic flag to prevent double completion.
         */
        void complete_timeout()
        {
            if (completed.exchange(true))
                return; // Already completed

            // Remove hook from bot (if bot still exists)
            if (auto bot = bot_weak.lock())
            {
                bot->remove_hook_by_context(hook_cmd, context_ptr);
            }
            promise->set_exception(std::make_exception_ptr(std::runtime_error("request timeout")));
        }

        /**
         * @brief      Gets the task for awaiting the result.
         *
         * @return     The task that will complete with the response or timeout.
         */
        async::task<ResponseType> task()
        {
            return promise->task();
        }
    };

public:
    bot_controller<BotType>& controller; ///< Reference to the bot bot_controller for this bot type

protected:
    /**
     * @brief      Constructs a new bot instance with specific bot_controller.
     *
     * @param      bot_controller  The bot_controller that will manage this bot.
     * @param[in]  id         The unique identifier for this bot.
     */
    bot(bot_controller<BotType>& controller, uint32_t id);

public:
    /**
     * @brief      Sends a request and waits for a matching response.
     *
     *             Sends a protocol message to the server and sets up a temporary
     *             hook to capture the response that matches the given condition.
     *             This enables request-response patterns in bot communication.
     *
     * @param[in]  protocol   The protocol message to send.
     * @param[in]  condition  Function to determine if a response matches this request.
     * @param[in]  timeout    The timeout duration for the request.
     * @param[in]  encrypt    Whether to encrypt the outgoing message.
     * @param[in]  wrap       Whether to wrap the outgoing message.
     *
     * @tparam     ResponseType  The expected response protocol type.
     *
     * @return     An async task that completes with the matching response.
     */
    template <typename ResponseType>
    async::task<ResponseType> request(const fb::protocol::header&                          protocol,
                                      const std::function<bool(const ResponseType& resp)>& condition,
                                      const fb::model::timespan&                           timeout = 0s,
                                      bool                                                 encrypt = true,
                                      bool                                                 wrap    = true);

    /**
     * @brief      Sends a request and waits for any response of the specified type.
     *
     *             Convenience method that accepts any response of the specified
     *             type without additional filtering conditions.
     *
     * @param[in]  protocol  The protocol message to send.
     * @param[in]  timeout   The timeout duration for the request.
     * @param[in]  encrypt   Whether to encrypt the outgoing message.
     * @param[in]  wrap      Whether to wrap the outgoing message.
     *
     * @tparam     ResponseType  The expected response protocol type.
     *
     * @return     An async task that completes with the response.
     */
    template <typename ResponseType>
    async::task<ResponseType> request(const fb::protocol::header& protocol,
                                      const fb::model::timespan&  timeout = 0s,
                                      bool                        encrypt = true,
                                      bool                        wrap    = true);
};

} // namespace fb::bot

#endif