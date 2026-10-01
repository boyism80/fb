#ifndef __OUTBOUND_BUFFER_H__
#define __OUTBOUND_BUFFER_H__

#include <fb/stream.h>
#include <fb/tcp_socket.h>

#include <memory>

namespace fb {

class outbound_buffer
{
private:
    struct state;

    std::shared_ptr<state> _state;

public:
    outbound_buffer();
    ~outbound_buffer();

    outbound_buffer(const outbound_buffer&)             = delete;
    outbound_buffer& operator= (const outbound_buffer&) = delete;
    outbound_buffer(outbound_buffer&&)                  = default;
    outbound_buffer& operator= (outbound_buffer&&)      = default;

    void append(std::shared_ptr<fb::tcp_socket> endpoint, fb::stream wire);
    void flush();
};

} // namespace fb

#endif // !__OUTBOUND_BUFFER_H__
