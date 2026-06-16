#ifndef __OUTBOUND_BUFFER_H__
#define __OUTBOUND_BUFFER_H__

#include <boost/asio/ip/tcp.hpp>
#include <cstdint>
#include <fb/stream.h>
#include <memory>

namespace fb {

class outbound_buffer
{
public:
    outbound_buffer();
    ~outbound_buffer();

    outbound_buffer(const outbound_buffer&)             = delete;
    outbound_buffer& operator= (const outbound_buffer&) = delete;
    outbound_buffer(outbound_buffer&&)                  = default;
    outbound_buffer& operator= (outbound_buffer&&)      = default;

    void append(std::shared_ptr<boost::asio::ip::tcp::socket> endpoint, fb::stream wire);
    void flush();

private:
    struct state;

    std::shared_ptr<state> _state;

    static uint32_t endpoint_key(boost::asio::ip::tcp::socket& endpoint);
    void            write(std::shared_ptr<boost::asio::ip::tcp::socket> endpoint, std::shared_ptr<fb::stream> wire);
};

} // namespace fb

#endif // !__OUTBOUND_BUFFER_H__
