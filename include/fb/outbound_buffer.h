#ifndef __OUTBOUND_BUFFER_H__
#define __OUTBOUND_BUFFER_H__

#include <boost/asio/ip/tcp.hpp>
#include <cstdint>
#include <fb/stream.h>
#include <memory>
#include <unordered_map>

namespace fb {

class outbound_buffer
{
public:
    void append(std::shared_ptr<boost::asio::ip::tcp::socket> endpoint, fb::stream wire);
    void flush();

private:
    struct slot
    {
        std::shared_ptr<boost::asio::ip::tcp::socket> endpoint;
        fb::stream                                    wire;
    };

    std::unordered_map<uint32_t, slot> _pending;

    static uint32_t endpoint_key(boost::asio::ip::tcp::socket& endpoint);
    void            write(std::shared_ptr<boost::asio::ip::tcp::socket> endpoint, std::shared_ptr<fb::stream> wire);
};

} // namespace fb

#endif // !__OUTBOUND_BUFFER_H__
