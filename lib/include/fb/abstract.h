#ifndef __ABSTRACT_H__
#define __ABSTRACT_H__

#include <boost/asio.hpp>
#include <fb/thread_container.h>

namespace fb {

class context
{
protected:
    boost::asio::io_context& _boost_context;

public:
    thread_container threads;

protected:
    context(boost::asio::io_context& context, const std::string& name, uint32_t thread_count);

public:
    virtual ~context() = default;

public:
    operator boost::asio::io_context& () const;
};

class acceptable : public context, public boost::asio::ip::tcp::acceptor
{
protected:
    acceptable(boost::asio::io_context& context, const std::string& name, uint32_t thread_count, uint16_t port);

public:
    virtual ~acceptable() = default;
};

} // namespace fb

#endif