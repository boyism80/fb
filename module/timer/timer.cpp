#include "timer.h"

fb::timer::timer(boost::asio::io_context& context, std::function<void(uint64_t)> fn, uint32_t ms) : 
    boost::asio::steady_timer(context, std::chrono::milliseconds(ms)),
    _context(context), _fn(fn), _ms(ms)
{
    this->async_wait(boost::bind(&timer::handle_loop, this));
}

fb::timer::~timer()
{
}

void fb::timer::handle_loop()
{
    boost::asio::dispatch
    (
        this->_context,
        [this] ()
        {
            this->_fn(now());
        }
    );
    this->expires_from_now(std::chrono::milliseconds(this->_ms));
    this->async_wait(boost::bind(&timer::handle_loop, this));
}

uint64_t fb::timer::now()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void fb::timer::run(boost::asio::io_context& context, std::function<void(void)> fn, uint32_t ms)
{
    auto timer = new boost::asio::steady_timer(context, std::chrono::milliseconds(ms));
    timer->async_wait
    (
        [&context, fn, timer] (const boost::system::error_code& e)
        {
            boost::asio::dispatch(context, fn);
            delete timer;
        }
    );
}

fb::timer_container::timer_container(boost::asio::io_context& context) : 
    _context(context)
{ }

fb::timer_container::~timer_container()
{
    for(auto x : *this)
        delete x;
}

void fb::timer_container::push(std::function<void(uint64_t)> fn, uint32_t ms)
{
    this->push_back(new timer(this->_context, fn, ms));
}

void fb::timer_container::cancel()
{
    for(auto x : *this)
        x->cancel();
}
