#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <fb/core/type.h>
#include <fb/core/stream.h>

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

} } }

namespace fb { namespace protocol { namespace base {

class header
{
public:
    const int __id;

protected:
    header(int id) : __id(id)
    { }

public:
    ~header() = default;

public:
    virtual void            serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(this->__id);
    }
    virtual void            deserialize(fb::istream& in_stream) { }
};

} } }

#include <fb/protocol/request.common.h>
#include <fb/protocol/response.common.h>

#endif // !__PROTOCOL_H__
