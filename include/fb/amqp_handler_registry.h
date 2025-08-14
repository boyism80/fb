#ifndef __FB_AMQP_HANDLER_REGISTRY_H__
#define __FB_AMQP_HANDLER_REGISTRY_H__

#include <unordered_map>
#include <functional>
#include <string>
#include <async/task.h>
#include <fb/amqp.h>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

namespace fb {

template <typename T>
class acceptor;

/**
 * @brief      A registry for AMQP message handlers.
 *
 *             This class manages AMQP connections, message handlers, and the AMQP event loop.
 *             It provides functionality for:
 *             - AMQP connection management and initialization
 *             - Message handler registration and routing
 *             - Queue declaration and binding
 *             - Automatic reconnection handling
 *             - Thread-safe message processing
 *
 * @tparam     T     The session type for socket connections
 */
template <typename T>
class amqp_handler_registry
{
public:
    using handler_func       = std::function<async::task<void>(const uint8_t*)>;
    using initialize_handler = std::function<void(fb::amqp::socket&)>; ///< Type for AMQP initialization handler

private:
    fb::acceptor<T>& _owner; ///< Reference to the owner acceptor
    std::unordered_map<std::string, std::unordered_map<uint32_t, handler_func>>
                                      _handlers; ///< Maps exchange+routing_key to handlers
    std::unique_ptr<fb::amqp::socket> _amqp;     ///< AMQP connection for inter-service communication

public:
    initialize_handler on_initialize; ///< Handler called after AMQP connection is established

public:
    /**
     * @brief      Constructs a new AMQP handler registry.
     *
     * @param[in]  owner  Reference to the owner acceptor instance
     */
    amqp_handler_registry(fb::acceptor<T>& owner) :
        _owner(owner),
        on_initialize(nullptr)
    { }

    amqp_handler_registry(const amqp_handler_registry&)             = delete;
    amqp_handler_registry& operator= (const amqp_handler_registry&) = delete;

public:
    /**
     * @brief      Main AMQP thread loop that manages inter-service message queue communication.
     *
     *             This method runs in a dedicated thread and handles the complete lifecycle of
     *             AMQP connections with automatic reconnection on failures. The loop:
     *             1. Establishes AMQP connection using configuration
     *             2. Calls the initialization handler if set
     *             3. Processes incoming messages until connection failure
     *             4. Automatically reconnects on errors
     */
    void thread_loop()
    {
        auto timeout = timeval{5, 0}; // 5 second timeout for select operations
        while (this->_owner.running())
        {
            try
            {
                // Create new AMQP connection
                this->_amqp = std::make_unique<fb::amqp::socket>();
                this->_amqp->connect(fb::config<std::string>("amqp:ip"),
                                     fb::config<uint16_t>("amqp:port"),
                                     fb::config<std::string>("amqp:uid"),
                                     fb::config<std::string>("amqp:pwd"),
                                     "/");

                // Call initialization handler if set
                if (this->on_initialize)
                    this->on_initialize(*this->_amqp.get());
            }
            catch (std::exception& e)
            {
                fb::logger::fatal(e.what());
                std::this_thread::sleep_for(1s); // Wait before retry
                continue;
            }

            // Message processing loop
            while (this->_owner.running())
            {
                try
                {
                    // Check for incoming messages with timeout
                    if (this->_amqp->select(&timeout) == false)
                        continue; // No messages, continue polling
                }
                catch (std::exception&)
                {
                    break; // Connection error, reconnect
                }
            }
        }
    }

    /**
     * @brief      Gets the underlying AMQP socket.
     *
     * @return     Reference to the AMQP socket.
     */
    fb::amqp::socket& socket()
    {
        return *this->_amqp;
    }

    /**
     * @brief      Binds an AMQP handler function to a route and response type.
     *
     * @param[in]  route  The AMQP route to bind to.
     * @param[in]  fn     The member function to handle the AMQP message.
     *
     * @tparam     Class         The class containing the handler function.
     * @tparam     ResponseType  The type of the response to handle.
     */
    template <typename HandlerType>
    void bind(const std::string& route)
    {
        using message_type = typename HandlerType::message_type;

        if (!this->_handlers.contains(route))
            this->_handlers.insert({route, std::unordered_map<uint32_t, handler_func>()});

        auto cmd = static_cast<uint32_t>(message_type::FlatBufferProtocolType);
        this->_handlers[route].insert({cmd, [this](const uint8_t* ptr) -> async::task<void> {
                                           auto protocol = std::make_shared<message_type>();
                                           protocol->Deserialize(ptr);

                                           auto& server = static_cast<typename HandlerType::server_type&>(this->_owner);
                                           auto  handler = std::make_shared<HandlerType>(server);
                                           co_await handler->handle(*protocol);
                                       }});
    }

    /**
     * @brief      Declares a new AMQP queue and binds it to an exchange.
     *
     * @param[in]  exchange  The exchange to bind the queue to
     * @param[in]  key       The routing key for the binding
     */
    void declare_queue(const std::string& exchange, const std::string& key)
    {
        auto& queue = this->_amqp->declare_queue();
        queue.bind(exchange, key);

        auto& route = queue.route();
        if (this->_handlers.contains(route))
        {
            for (auto& [cmd, fn] : this->_handlers.at(route))
            {
                queue.handler(cmd, fn);
            }
        }
    }

    /**
     * @brief      Gets the handler for a specific exchange and routing key.
     *
     * @param[in]  exchange  The AMQP exchange name
     * @param[in]  key       The routing key
     * @return     The handler function if found
     */
    handler_func get_handler(const std::string& exchange, uint32_t key) const
    {
        if (!this->_handlers.contains(exchange))
            return nullptr;

        const auto& exchange_handlers = this->_handlers.at(exchange);
        if (!exchange_handlers.contains(key))
            return nullptr;

        return exchange_handlers.at(key);
    }

    /**
     * @brief      Checks if a handler exists for the given exchange and routing key.
     *
     * @param[in]  exchange  The AMQP exchange name
     * @param[in]  key       The routing key
     * @return     True if a handler exists, false otherwise
     */
    bool has_handler(const std::string& exchange, uint32_t key) const
    {
        if (!this->_handlers.contains(exchange))
            return false;

        const auto& exchange_handlers = this->_handlers.at(exchange);
        return exchange_handlers.contains(key);
    }
};

} // namespace fb

#endif // !__FB_AMQP_HANDLER_REGISTRY_H__