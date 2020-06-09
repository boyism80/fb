#include "timer.h"

fb::timer::timer(boost::asio::io_context& context, std::function<void(uint64_t)> fn, uint32_t ms) : 
    boost::asio::steady_timer(context, std::chrono::milliseconds(ms)),
    _fn(fn), _ms(ms)
{
    this->async_wait(boost::bind(&timer::handle, this));
}

fb::timer::~timer()
{
}

void fb::timer::handle()
{
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    this->_fn(now);
    this->expires_from_now(std::chrono::milliseconds(this->_ms));
    this->async_wait(boost::bind(&timer::handle, this));
}

fb::timer_container::timer_container(boost::asio::io_context& context) : 
    _context(context)
{

}

fb::timer_container::~timer_container()
{
    for(auto x : *this)
        delete x;
}

void fb::timer_container::push(std::function<void(uint64_t)> fn, uint32_t ms)
{
    auto new_timer = new timer(this->_context, fn, ms);
    this->push_back(new_timer);
}

void fb::timer_container::cancel()
{
    for(auto x : *this)
        x->cancel();
}
