#include <fb/model/model.h>

std::optional<fb::model::datetime> fb::model::schedule::next_execution(const fb::model::datetime& now) const
{
    if (this->script.empty())
        return std::nullopt;
    if (!this->date.begin.has_value())
        return std::nullopt;

    const auto& begin = this->date.begin.value();

    /*
     * 1. Only date.begin (no end) = begin ~ inf
     *    - No repeat: run exactly at begin (one-time).
     *    - Repeat: next = begin + (n+1)*interval indefinitely.
     */
    if (!this->date.end.has_value())
    {
        if (!this->repeat.has_value())
        {
            if (now < begin)
                return begin;
            return std::nullopt;
        }
        if (now < begin)
            return begin;
        auto elapsed     = now - begin;
        auto interval_ms = static_cast<int64_t>(this->repeat.value().total_milliseconds());
        if (interval_ms <= 0)
            return std::nullopt;
        auto elapsed_ms = elapsed.total_milliseconds();
        auto remainder  = elapsed_ms % interval_ms;
        auto next       = now + fb::model::timespan(std::chrono::milliseconds(interval_ms - remainder));
        return next;
    }

    /*
     * 2. date.begin and date.end = begin ~ end
     *    - No repeat: if now in [begin, end], run immediately (one-time in window).
     *    - Repeat: next = begin + (k+1)*interval for smallest k with (begin + (k+1)*interval) > now and <= end.
     */
    const auto& end = this->date.end.value();
    if (now > end)
        return std::nullopt;
    if (now < begin)
        return begin;

    if (!this->repeat.has_value())
        return now;

    auto elapsed     = now - begin;
    auto interval_ms = static_cast<int64_t>(this->repeat.value().total_milliseconds());
    if (interval_ms <= 0)
        return std::nullopt;
    auto elapsed_ms = elapsed.total_milliseconds();
    auto remainder  = elapsed_ms % interval_ms;
    auto next       = now + fb::model::timespan(std::chrono::milliseconds(interval_ms - remainder));
    if (next > end)
        return std::nullopt;
    return next;
}
