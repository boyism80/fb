#ifndef __FB_PROTOCOL_HANDLER_REGISTRY_H__
#define __FB_PROTOCOL_HANDLER_REGISTRY_H__

#include <unordered_map>
#include <functional>
#include <memory>
#include <chrono>
#include <async/task.h>
#include <fb/protocol/header.h>
#include <fb/model/datetime.h>

using namespace std::chrono_literals;

namespace fb {

// Forward declarations
template <typename T>
class socket;

template <typename T>
class acceptor;

/**
 * @brief      Protocol header concept that enforces inheritance from fb::protocol::header
 */
template <typename U>
concept ProtocolHeader = std::is_base_of_v<fb::protocol::header, U>;

/**
 * @brief      A rate-limited command executor for client protocol messages.
 *
 *             This class manages the execution of client protocol commands with rate limiting
 *             capabilities. It tracks the number of executions within a specified time window
 *             and enforces rate limits to prevent abuse.
 *
 * @tparam     T     The session type for socket connections
 */
template <typename T>
class protocol_handler_registry
{
public:
    using handle_func      = std::function<async::task<bool>(fb::socket<T>&, fb::protocol::header&)>;
    using deserialize_func = std::function<async::task<fb::protocol::header*>(fb::stream_reader<big_endian>&)>;

private:
    /**
     * @brief      Internal handler structure that manages protocol command handlers with rate limiting.
     */
    struct rate_limited_command
    {
    public:
        /// The actual handler function to execute
        const handle_func fn;
        /// Time window for rate limiting (default: 1s)
        const std::chrono::steady_clock::duration duration = 1s;
        /// Maximum transitions allowed per window (default: unlimited)
        const uint32_t limit = 0xFFFFFFFF;

        rate_limited_command() = default;

        /**
         * @brief      Constructs a handler with rate limiting parameters.
         */
        rate_limited_command(const handle_func&                         fn,
                             const std::chrono::steady_clock::duration& duration,
                             uint32_t                                   limit = 0xFFFFFFFF) :
            fn(fn),
            duration(duration),
            limit(limit)
        { }
    };

private:
    fb::acceptor<T>&                                  _owner;    ///< Reference to the owner acceptor
    std::unordered_map<uint8_t, rate_limited_command> _handlers; ///< Maps protocol command bytes to their handlers
    std::unordered_map<uint8_t, deserialize_func>
        _deserializers; ///< Maps protocol command bytes to deserialization functions

public:
    /**
     * @brief      Constructs a new protocol handler registry.
     *
     * @param[in]  owner  Reference to the owner acceptor instance
     */
    protocol_handler_registry(fb::acceptor<T>& owner) :
        _owner(owner)
    { }

    protocol_handler_registry(const protocol_handler_registry&)             = delete;
    protocol_handler_registry& operator= (const protocol_handler_registry&) = delete;

public:
    /**
     * @brief      Binds a member function as a protocol handler with explicit header.
     *
     * @tparam     Class     The class type containing the handler method.
     * @tparam     Request   The request type that the handler processes.
     *                       Must satisfy the ProtocolHeader concept.
     * @param[in]  fn        Pointer to the member function to bind.
     * @param[in]  header    The protocol command header byte.
     * @param[in]  duration  The time window for rate limiting (default: 1s).
     * @param[in]  limit     Maximum number of calls allowed within the duration window (default: 10).
     */
    template <typename Class, typename Request>
    requires ProtocolHeader<Request>
    void bind(async::task<bool> (Class::*fn)(fb::socket<T>&, const Request&),
              uint8_t                                    header,
              const std::chrono::steady_clock::duration& duration = 1s,
              uint32_t                                   limit    = 10)
    {
        // Add deserializer
        this->_deserializers.insert({header, [](auto& reader) -> async::task<fb::protocol::header*> {
                                         auto protocol = new Request();
                                         co_await protocol->deserialize(reader);
                                         co_return protocol;
                                     }});

        // Add handler with rate limiting
        this->_handlers.insert(
            {header,
             rate_limited_command(
                 [this, fn](fb::socket<T>& socket, fb::protocol::header& header) -> async::task<bool> {
                     auto* protocol = static_cast<Request*>(&header);
                     return (static_cast<Class&>(this->_owner).*fn)(socket, *protocol);
                 },
                 duration,
                 limit)});
    }

    /**
     * @brief      Binds a member function as a protocol handler using Request's header.
     *
     * @tparam     Class     The class type containing the handler method.
     * @tparam     Request   The request type that the handler processes.
     *                       Must satisfy the ProtocolHeader concept.
     * @param[in]  fn        Pointer to the member function to bind.
     * @param[in]  duration  The time window for rate limiting (default: 1s).
     * @param[in]  limit     Maximum number of calls allowed within the duration window (default: 10).
     */
    template <typename Class, typename Request>
    requires ProtocolHeader<Request>
    void bind(async::task<bool> (Class::*fn)(fb::socket<T>&, const Request&),
              const std::chrono::steady_clock::duration& duration = 1s,
              uint32_t                                   limit    = 10)
    {
        this->bind(fn, Request::header, duration, limit);
    }

    /**
     * @brief      Checks if a handler exists for the given command.
     */
    bool has_handler(uint8_t cmd) const
    {
        return this->_handlers.contains(cmd);
    }

    /**
     * @brief      Checks if a deserializer exists for the given command.
     */
    bool has_deserializer(uint8_t cmd) const
    {
        return this->_deserializers.contains(cmd);
    }

    /**
     * @brief      Gets the handler for a protocol command.
     */
    const rate_limited_command& get_handler(uint8_t cmd) const
    {
        return this->_handlers.at(cmd);
    }

    /**
     * @brief      Gets the deserializer for a protocol command.
     */
    const deserialize_func& get_deserializer(uint8_t cmd) const
    {
        return this->_deserializers.at(cmd);
    }
};

} // namespace fb

#endif // !__FB_PROTOCOL_HANDLER_REGISTRY_H__