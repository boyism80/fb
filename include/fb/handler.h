#ifndef FB_HANDLER_H
#define FB_HANDLER_H

#include <async/task.h>

namespace fb {

template <typename ServerType, typename ProtocolType>
class handler
{
public:
    using server_type   = ServerType;
    using protocol_type = ProtocolType;
    using session_type  = server_type::session_type;

protected:
    server_type& server;

protected:
    handler(server_type& server) :
        server(server)
    { }

    virtual async::task<bool> handle(session_type& session, protocol_type& protocol) = 0;
};

} // namespace fb

#endif // FB_HANDLER_H