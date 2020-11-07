#ifndef __FB_TIMER_H__
#define __FB_TIMER_H__

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
    void                            handle_loop();

public:
    static uint64_t                 now();

private:
    static void                     handle_once(const boost::system::error_code& e, std::function<void(void)> fn, boost::asio::steady_timer* timer);

public:
    static void                     run(boost::asio::io_context& context, std::function<void()> fn, uint32_t ms);
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