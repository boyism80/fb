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
     * @brief      Binds a handler to all commands with a default duration and limit.
     *
     * @tparam     HandlerType  The type of the handler to bind.
     * @param[in]  duration     The duration for rate limiting.
     * @param[in]  limit        The maximum number of executions per duration.
     */
    template <typename HandlerType>
    void bind(const std::chrono::steady_clock::duration& duration = 1s, uint32_t limit = 10)
    {
        using protocol_type = typename HandlerType::protocol_type;

        this->bind<HandlerType>(protocol_type::header, duration, limit);
    }

    /**
     * @brief      Binds a handler to a specific command with a default duration and limit.
     *
     * @tparam     HandlerType  The type of the handler to bind.
     * @param[in]  cmd          The command byte to bind the handler to.
     * @param[in]  duration     The duration for rate limiting.
     * @param[in]  limit        The maximum number of executions per duration.
     */
    template <typename HandlerType>
    void bind(uint8_t cmd, const std::chrono::steady_clock::duration& duration = 1s, uint32_t limit = 10)
    {
        using session_type  = typename HandlerType::session_type;
        using protocol_type = typename HandlerType::protocol_type;

        auto& server = static_cast<typename HandlerType::server_type&>(this->_owner);

        this->_deserializers.insert({cmd, [](auto& reader) -> async::task<fb::protocol::header*> {
                                         auto protocol = new typename HandlerType::protocol_type();
                                         co_await protocol->deserialize(reader);
                                         co_return protocol;
                                     }});

        this->_handlers.insert(
            {cmd,
             rate_limited_command(
                 [this, &server](fb::socket<T>& socket, fb::protocol::header& header) -> async::task<bool> {
                     auto* protocol = static_cast<typename HandlerType::protocol_type*>(&header);
                     auto& session  = static_cast<typename HandlerType::session_type&>(socket);
                     auto  handler  = HandlerType(server);
                     return handler.handle(session, *protocol);
                 },
                 duration,
                 limit)});
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