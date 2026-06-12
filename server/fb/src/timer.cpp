#include <fb/timer.h>

using namespace fb;

timer::timer(const handle_callback_type& fn,
             const fb::model::timespan&  duration,
             repeat_type                 repeat,
             async::propagation::token   context) :
    fn(fn),
    duration(duration),
    repeat(repeat),
    context(std::move(context))
{ }
