#ifndef __ABSTRACT_H__
#define __ABSTRACT_H__

#include <boost/asio.hpp>
#include <fb/thread_container.h>

namespace fb {

class context : public boost::asio::ip::tcp::acceptor
{
protected:
    boost::asio::io_context& _boost_context;

public:
    thread_container threads;

protected:
    context(boost::asio::io_context& context, uint16_t port);

public:
    ~context() = default;

public:
    operator boost::asio::io_context& () const;
};

} // namespace fb

#endif