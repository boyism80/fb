#ifndef __BOT_H__
#define __BOT_H__

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

class bot_container;

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
    using handle_func    = std::function<async::task<void>(fb::protocol::header&)>; ///< Protocol handler function type
    using deserilze_func = std::function<async::task<fb::protocol::header*>(
        fb::stream_reader<big_endian>&)>;                                         ///< Deserialization function type
    using hook_func      = std::function<bool(const fb::protocol::header&)>;      ///< Hook condition function type
    using hook_container = std::unordered_map<uint8_t, std::vector<hook_params>>; ///< Container for protocol hooks

protected:
    bot_container&                              _owner;        ///< Reference to the bot container that manages this bot
    fb::crypto                                  _crypto;       ///< Cryptographic context for secure communication
    std::unordered_map<uint8_t, handle_func>    _handler;      ///< Maps protocol commands to handler functions
    std::unordered_map<uint8_t, deserilze_func> _deserializer; ///< Maps protocol commands to deserialization functions
    hook_container                              _hooks;        ///< Temporary hooks for request-response patterns

public:
    const uint32_t id; ///< Unique identifier for this bot instance

protected:
    /**
     * @brief      Constructs a new bot instance.
     *
     * @param      owner  The bot container that will manage this bot.
     * @param[in]  id     The unique identifier for this bot.
     */
    base_bot(bot_container& owner, uint32_t id);

public:
    /**
     * @brief      Virtual destructor for proper inheritance cleanup.
     */
    virtual ~base_bot();

public:
    /**
     * @brief      Handles incoming data from the server.
     *
     *             Processes received data streams, deserializes protocols, and
     *             dispatches them to appropriate handlers or hooks.
     *
     * @param      stream  The incoming data stream to process.
     *
     * @return     An async task that completes when processing is finished.
     */
    async::task<void> on_receive(fb::stream& stream);

    /**
     * @brief      Handles connection closure events.
     *
     *             Called when the connection to the server is closed, either
     *             gracefully or due to an error. Performs cleanup operations.
     *
     * @return     An async task that completes when cleanup is finished.
     */
    async::task<void> on_closed();

protected:
    /**
     * @brief      Called when the bot successfully connects to the server.
     *
     *             Override this method to implement bot-specific connection
     *             initialization logic, such as sending authentication requests
     *             or setting up initial state.
     *
     * @return     An async task that completes when connection setup is finished.
     */
    virtual async::task<void> on_connected();

    /**
     * @brief      Called when the bot disconnects from the server.
     *
     *             Override this method to implement bot-specific cleanup logic
     *             when the connection is lost or closed.
     *
     * @return     An async task that completes when disconnection cleanup is finished.
     */
    virtual async::task<void> on_disconnected();

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

    /**
     * @brief      Determines whether a command should be decrypted.
     *
     *             Override this method to specify which protocol commands
     *             require decryption based on the bot's security policy.
     *
     * @param[in]  cmd  The protocol command identifier.
     *
     * @return     True if the command should be decrypted, false otherwise.
     */
    virtual bool decrypt_policy(int cmd) const;

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
     * @brief      Called periodically to handle time-based bot actions.
     *
     *             Override this method to implement scheduled behaviors,
     *             such as periodic actions, timeouts, or state updates.
     *
     * @param[in]  now  The current date and time.
     *
     * @return     An async task that completes when timer processing is finished.
     */
    virtual async::task<void> on_timer(const fb::model::datetime& now)
    {
        co_return;
    }

    /**
     * @brief      Binds a response handler for a specific protocol type.
     *
     *             Registers a handler function that will be called whenever
     *             a protocol message of the specified type is received.
     *             Also sets up deserialization for the protocol type.
     *
     * @param[in]  fn   The handler function to bind.
     *
     * @tparam     ResponseType  The protocol response type to handle.
     */
    template <typename ResponseType>
    void bind(const std::function<async::task<void>(ResponseType&)>& fn)
    {
        this->_deserializer.insert({ResponseType::header, [](auto& reader) -> async::task<fb::protocol::header*> {
                                        auto protocol = new ResponseType();
                                        co_await protocol->deserialize(reader);
                                        co_return protocol;
                                    }});

        this->_handler.insert(
            {ResponseType::header, [this, fn](auto& header) -> async::task<void> {
                 if (this->_hooks.contains(ResponseType::header))
                 {
                     auto& matched_hooks = this->_hooks.at(ResponseType::header);
                     auto  i = std::find_if(matched_hooks.begin(), matched_hooks.end(), [&header](const auto& hook) {
                         return hook.condition(header);
                     });

                     if (i != matched_hooks.end())
                     {
                         auto callback = i->matched;
                         matched_hooks.erase(i);

                         callback(header);
                     }
                 }

                 auto protocol = static_cast<ResponseType&>(header);
                 co_await fn(protocol);
             }});
    }

    /**
     * @brief      Binds a member function as a response handler.
     *
     *             Convenience method for binding class member functions
     *             as protocol response handlers.
     *
     * @param[in]  fn   The member function to bind.
     *
     * @tparam     Class         The class type containing the member function.
     * @tparam     ResponseType  The protocol response type to handle.
     */
    template <typename Class, typename ResponseType>
    void bind(async::task<void> (Class::*fn)(const ResponseType&))
    {
        this->bind<ResponseType>(std::bind(fn, static_cast<Class*>(this), std::placeholders::_1));
    }

    /**
     * @brief      Sends a request and waits for a matching response.
     *
     *             Sends a protocol message to the server and sets up a temporary
     *             hook to capture the response that matches the given condition.
     *             This enables request-response patterns in bot communication.
     *
     * @param[in]  protocol   The protocol message to send.
     * @param[in]  condition  Function to determine if a response matches this request.
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
                                      bool                                                 encrypt = true,
                                      bool                                                 wrap    = true)
    {
        this->assert_thread();

        if (!this->_handler.contains(ResponseType::header))
        {
            this->bind<ResponseType>([](const ResponseType& resp) -> async::task<void> {
                co_return;
            });
        }

        auto promise = std::make_shared<async::task_completion_source<ResponseType>>();
        if (this->_hooks.contains(ResponseType::header) == false)
            this->_hooks.insert({ResponseType::header, {}});

        this->_hooks[ResponseType::header].push_back(hook_params{.condition =
                                                                     [promise, condition](const auto& header) {
                                                                         auto& protocol =
                                                                             static_cast<const ResponseType&>(header);
                                                                         return condition(protocol);
                                                                     },
                                                                 .matched =
                                                                     [promise](const auto& header) {
                                                                         auto& protocol =
                                                                             static_cast<const ResponseType&>(header);
                                                                         promise->set_value(protocol);
                                                                     }});
        this->send(protocol, encrypt, wrap);
        return promise->task();
    }

    /**
     * @brief      Sends a request and waits for any response of the specified type.
     *
     *             Convenience method that accepts any response of the specified
     *             type without additional filtering conditions.
     *
     * @param[in]  protocol  The protocol message to send.
     * @param[in]  encrypt   Whether to encrypt the outgoing message.
     * @param[in]  wrap      Whether to wrap the outgoing message.
     *
     * @tparam     ResponseType  The expected response protocol type.
     *
     * @return     An async task that completes with the response.
     */
    template <typename ResponseType>
    async::task<ResponseType> request(const fb::protocol::header& protocol, bool encrypt = true, bool wrap = true)
    {
        co_return co_await this->request<ResponseType>(
            protocol,
            [](auto& resp) -> bool {
                return true;
            },
            encrypt,
            wrap);
    }

public:
    /**
     * @brief      Gets the thread associated with this bot.
     *
     * @return     Pointer to the thread managing this bot's execution.
     */
    fb::thread* thread() const override final;
};

} // namespace fb::bot

#endif