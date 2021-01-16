#ifndef __FB_TIMER_H__
#define __FB_TIMER_H__

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <ctime>

namespace fb {

class timer : private boost::asio::steady_timer
{
private:
    boost::asio::io_context&            _context;
    std::chrono::steady_clock::duration _duration;
    std::function<void(uint64_t)>       _fn;

public:
    using boost::asio::steady_timer::cancel;

public:
    timer(boost::asio::io_context& context, std::function<void(uint64_t)> fn, const std::chrono::steady_clock::duration& duration);
    ~timer();

private:
    void                            handle_loop();

public:
    static uint64_t                 now();

public:
    static void                     run(boost::asio::io_context& context, std::function<void()> fn, const std::chrono::steady_clock::duration& duration);
};


class timer_container : private std::vector<timer*>
{
private:
    boost::asio::io_context&        _context;

public:
    timer_container(boost::asio::io_context& context);
    ~timer_container();

public:
    void                            push(std::function<void(uint64_t)> fn, const std::chrono::steady_clock::duration& duration);
    void                            cancel();
};

}

#endif // !__FB_TIMER_H__