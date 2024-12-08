#ifndef __ABSTRACT_H__
#define __ABSTRACT_H__

#include <boost/asio.hpp>
#include <fb/thread.h>

namespace fb {

class context : public boost::asio::ip::tcp::acceptor
{
protected:
    boost::asio::io_context& _boost_context;

public:
    fb::threads threads;

protected:
    context(boost::asio::io_context& context, uint16_t port) :
        boost::asio::ip::tcp::acceptor(context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
        _boost_context(context),
        threads(context)
    { }

public:
    ~context() = default;

public:
    inline operator boost::asio::io_context& () const
    {
        return this->_boost_context;
    }
};

} // namespace fb

#endif