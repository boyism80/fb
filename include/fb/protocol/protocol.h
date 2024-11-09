#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fb/stream.h>

#define BIND_ID(n) static constexpr uint8_t id = (n);

namespace fb { namespace protocol { namespace internal {

enum class services : uint8_t
{
    GATEWAY,
    LOGIN,
    GAME
};

enum id : uint8_t
{
    SUBSCRIBE,
    TRANSFER,
    LOGIN,
    LOGOUT,
    WHISPER,
    MESSAGE,
    SHUTDOWN,
};

}}} // namespace fb::protocol::internal

namespace fb { namespace protocol { namespace base {

class header
{
protected:
    header() = default;

public:
    ~header() = default;

public:
    virtual void serialize(fb::ostream& out_stream) const
    { }
    virtual void deserialize(fb::istream& in_stream)
    { }
};

}}} // namespace fb::protocol::base

#include <fb/protocol/request.common.h>
#include <fb/protocol/response.common.h>

#endif // !__PROTOCOL_H__
