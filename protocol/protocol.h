#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <module/common/type.h>

namespace fb { namespace protocol { namespace base {

class request
{
protected:
    request() {}
    ~request() {}

public:
    virtual void            deserialize(fb::istream& in_stream) = 0;
};


class response
{
protected:
    response() {}
    ~response() {}

public:
    virtual void            serialize(fb::ostream& out_stream) const = 0;
};

} } }

#include <protocol/request.common.h>
#include <protocol/response.common.h>

#endif // !__PROTOCOL_H__
