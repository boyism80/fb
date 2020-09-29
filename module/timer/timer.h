#ifndef __FB_TIMER_H__
#define __FB_TIMER_H__

#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

namespace fb {

class timer : private boost::asio::steady_timer
{
private:
    uint32_t                        _ms;
    std::function<void(uint64_t)>   _fn;

public:
    using boost::asio::steady_timer::cancel;

public:
    timer(boost::asio::io_context& context, std::function<void(uint64_t)> fn, uint32_t ms);
    ~timer();

private:
    void                            handle();

public:
    static uint64_t                 now();
};


class timer_container : private std::vector<timer*>
{
private:
    boost::asio::io_context&        _context;

public:
    timer_container(boost::asio::io_context& context);
    ~timer_container();

public:
    void                            push(std::function<void(uint64_t)> fn, uint32_t ms);
    void                            cancel();
};

}

#endif // !__FB_TIMER_H__