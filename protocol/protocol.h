#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "module/common/type.h"
#include "module/stream/stream.h"

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
protected:
    header() = default;
public:
    ~header() = default;

public:
    virtual void            serialize(fb::ostream& out_stream) const { }
    virtual void            deserialize(fb::istream& in_stream) { }
};

} } }

#include "protocol/request.common.h"
#include "protocol/response.common.h"

#endif // !__PROTOCOL_H__
