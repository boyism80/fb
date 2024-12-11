#include <fb/timer.h>

using namespace fb;

timer::timer(const handle_callback_type& fn, const fb::model::timespan& duration, bool disposable) :
    fn(fn),
    duration(duration),
    disposable(disposable)
{ }