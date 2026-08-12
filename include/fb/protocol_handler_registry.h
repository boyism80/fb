#ifndef __FB_PROTOCOL_HANDLER_REGISTRY_H__
#define __FB_PROTOCOL_HANDLER_REGISTRY_H__

#include <unordered_map>
#include <functional>
#include <memory>
#include <chrono>
#include <async/task.h>
#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/datetime.h>

namespace fb {

using namespace std::chrono_literals;

template <typename T>
class socket;

template <typename T>
class acceptor;

template <typename U>
concept ProtocolHeader = std::is_base_of_v<fb::protocol::header, U>;

// Protocols tag themselves with `version`, except those whose class name is
// already `version` (a member cannot share the enclosing class name).
template <typename ProtocolType>
constexpr fb::protocol::CLIENT_VERSION protocol_version_of()
{
    if constexpr (requires { ProtocolType::version; })
        return ProtocolType::version;
    else
        return ProtocolType::protocol_version;
}

template <typename T>
class protocol_handler_registry
{
public:
    using handle_func      = std::function<async::task<bool>(fb::socket<T>&, fb::protocol::header&)>;
    using deserialize_func = std::function<std::shared_ptr<fb::protocol::header>(fb::stream_reader<big_endian>&)>;

private:
    // Assignable: entries are replaced in place while binding handlers.
    struct rate_limited_command
    {
    public:
        handle_func                         fn;
        std::chrono::steady_clock::duration duration = 1s;
        uint32_t                            limit    = 0xFFFFFFFF;

        rate_limited_command() = default;

        rate_limited_command(const handle_func&                         fn,
                             const std::chrono::steady_clock::duration& duration,
                             uint32_t                                   limit = 0xFFFFFFFF) :
            fn(fn),
            duration(duration),
            limit(limit)
        { }
    };

    struct version_entry
    {
        deserialize_func     deserializer;
        rate_limited_command handler;
    };

private:
    fb::acceptor<T>& _owner;
    // opcode → packed CLIENT_VERSION → entry
    std::unordered_map<uint8_t, std::unordered_map<uint16_t, version_entry>> _entries;

public:
    protocol_handler_registry(fb::acceptor<T>& owner) :
        _owner(owner)
    { }

    protocol_handler_registry(const protocol_handler_registry&)             = delete;
    protocol_handler_registry& operator= (const protocol_handler_registry&) = delete;

public:
    // Registers Handler<V> for each CLIENT_VERSION where protocol_type::supported.
    template <template <fb::protocol::CLIENT_VERSION> class Handler> void bind()
    {
        this->bind_one_if_supported<Handler<fb::protocol::CLIENT_VERSION::v550>>();
        this->bind_one_if_supported<Handler<fb::protocol::CLIENT_VERSION::v565>>();
        this->bind_one_if_supported<Handler<fb::protocol::CLIENT_VERSION::v651>>();
    }

    template <typename HandlerType> void bind_one_if_supported()
    {
        using protocol_type = typename HandlerType::protocol_type;
        // Discarded statement: bind_one must not be instantiated for unsupported specializations.
        if constexpr (fb::protocol::protocol_supported<protocol_type>())
            this->bind_one<HandlerType>();
    }

    template <typename HandlerType> void bind_one()
    {
        using session_type  = typename HandlerType::session_type;
        using protocol_type = typename HandlerType::protocol_type;

        static_assert(fb::protocol::protocol_supported<protocol_type>(),
                      "bind_one requires a supported protocol specialization");

        auto& server = static_cast<typename HandlerType::server_type&>(this->_owner);

        auto duration = std::chrono::milliseconds(HandlerType::duration_ms);
        auto limit    = HandlerType::limit;
        auto opcode   = protocol_type::opcode;
        auto version  = static_cast<uint16_t>(protocol_version_of<protocol_type>());

        auto& slot = this->_entries[opcode][version];

        slot.deserializer = [](auto& reader) -> std::shared_ptr<fb::protocol::header> {
            auto protocol = std::make_shared<protocol_type>();
            protocol->deserialize(reader);
            return std::static_pointer_cast<fb::protocol::header>(protocol);
        };

        slot.handler = rate_limited_command(
            [this, &server](fb::socket<T>& socket, fb::protocol::header& header) -> async::task<bool> {
                auto* protocol = static_cast<protocol_type*>(&header);
                auto& session  = static_cast<session_type&>(socket);
                auto  handler  = std::make_shared<HandlerType>(server);
                co_return co_await handler->handle(session, *protocol);
            },
            duration,
            limit);
    }

    bool has_entry(uint8_t opcode, fb::protocol::CLIENT_VERSION version) const
    {
        auto it = this->_entries.find(opcode);
        if (it == this->_entries.end())
            return false;
        return it->second.contains(static_cast<uint16_t>(version));
    }

    bool has_opcode(uint8_t opcode) const
    {
        return this->_entries.contains(opcode);
    }

    const deserialize_func& get_deserializer(uint8_t opcode, fb::protocol::CLIENT_VERSION version) const
    {
        return this->_entries.at(opcode).at(static_cast<uint16_t>(version)).deserializer;
    }

    const rate_limited_command& get_handler(uint8_t opcode, fb::protocol::CLIENT_VERSION version) const
    {
        return this->_entries.at(opcode).at(static_cast<uint16_t>(version)).handler;
    }
};

} // namespace fb

#endif // !__FB_PROTOCOL_HANDLER_REGISTRY_H__
