#ifndef __FB_PROTOCOL_HANDLER_REGISTRY_H__
#define __FB_PROTOCOL_HANDLER_REGISTRY_H__

#include <unordered_map>
#include <functional>
#include <memory>
#include <chrono>
#include <async/task.h>
#include <fb/protocol/header.h>
#include <fb/model/datetime.h>

namespace fb {

using namespace std::chrono_literals;

template <typename T>
class socket;

template <typename T>
class acceptor;

template <typename U>
concept ProtocolHeader = std::is_base_of_v<fb::protocol::header, U>;

template <typename T>
class protocol_handler_registry
{
public:
    using handle_func = std::function<async::task<bool>(fb::socket<T>&, fb::protocol::header&)>;
    using deserialize_func =
        std::function<async::task<std::shared_ptr<fb::protocol::header>>(fb::stream_reader<big_endian>&)>;

private:
    struct rate_limited_command
    {
    public:
        const handle_func                         fn;
        const std::chrono::steady_clock::duration duration = 1s;
        const uint32_t                            limit    = 0xFFFFFFFF;

        rate_limited_command() = default;

        rate_limited_command(const handle_func&                         fn,
                             const std::chrono::steady_clock::duration& duration,
                             uint32_t                                   limit = 0xFFFFFFFF) :
            fn(fn),
            duration(duration),
            limit(limit)
        { }
    };

private:
    fb::acceptor<T>&                                  _owner;
    std::unordered_map<uint8_t, rate_limited_command> _handlers;
    std::unordered_map<uint8_t, deserialize_func>     _deserializers;

public:
    protocol_handler_registry(fb::acceptor<T>& owner) :
        _owner(owner)
    { }

    protocol_handler_registry(const protocol_handler_registry&)             = delete;
    protocol_handler_registry& operator= (const protocol_handler_registry&) = delete;

public:
    template <typename HandlerType> void bind()
    {
        using protocol_type = typename HandlerType::protocol_type;

        this->bind<HandlerType>(protocol_type::opcode);
    }

    template <typename HandlerType> void bind(uint8_t opcode)
    {
        using session_type  = typename HandlerType::session_type;
        using protocol_type = typename HandlerType::protocol_type;

        auto& server = static_cast<typename HandlerType::server_type&>(this->_owner);

        auto duration = std::chrono::milliseconds(HandlerType::duration_ms);
        auto limit    = HandlerType::limit;

        this->_deserializers.insert({opcode, [](auto& reader) -> async::task<std::shared_ptr<fb::protocol::header>> {
                                         auto protocol = std::make_shared<typename HandlerType::protocol_type>();
                                         co_await protocol->deserialize(reader);
                                         co_return std::static_pointer_cast<fb::protocol::header>(protocol);
                                     }});

        this->_handlers.insert(
            {opcode,
             rate_limited_command(
                 [this, &server](fb::socket<T>& socket, fb::protocol::header& header) -> async::task<bool> {
                     auto* protocol = static_cast<typename HandlerType::protocol_type*>(&header);
                     auto& session  = static_cast<typename HandlerType::session_type&>(socket);
                     auto  handler  = std::make_shared<HandlerType>(server);
                     co_return co_await handler->handle(session, *protocol);
                 },
                 duration,
                 limit)});
    }

    bool has_handler(uint8_t opcode) const
    {
        return this->_handlers.contains(opcode);
    }

    bool has_deserializer(uint8_t opcode) const
    {
        return this->_deserializers.contains(opcode);
    }

    const rate_limited_command& get_handler(uint8_t opcode) const
    {
        return this->_handlers.at(opcode);
    }

    const deserialize_func& get_deserializer(uint8_t opcode) const
    {
        return this->_deserializers.at(opcode);
    }
};

} // namespace fb

#endif // !__FB_PROTOCOL_HANDLER_REGISTRY_H__