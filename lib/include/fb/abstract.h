#ifndef __ABSTRACT_H__
#define __ABSTRACT_H__

#include <boost/asio.hpp>
#include <fb/thread.h>

namespace fb {

class icontext : public boost::asio::ip::tcp::acceptor
{
protected:
    icontext(boost::asio::io_context& context, uint16_t port) :
        boost::asio::ip::tcp::acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    { }

public:
    ~icontext() = default;

public:
    virtual fb::thread* current_thread() = 0;
};

} // namespace fb

#endif