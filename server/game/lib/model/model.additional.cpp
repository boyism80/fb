#include <fb/model/model.h>

std::optional<fb::model::datetime> fb::model::schedule::next_execution(const fb::model::datetime& now) const
{
    if (this->script.empty())
        return std::nullopt;

    if (this->date.end.has_value() && now > this->date.end.value())
        return std::nullopt;

    if (!this->date.end.has_value())
    {
        if (!this->date.begin.has_value())
            return std::nullopt;

        if (now >= this->date.begin.value())
            return std::nullopt;

        return this->date.begin.value();
    }

    if (!this->repeat.has_value())
        return std::nullopt;

    auto begin    = this->date.begin.value_or(now);
    auto end      = this->date.end.value();
    auto interval = this->repeat.value();

    if (now < begin)
        return begin;

    if (now >= end)
        return std::nullopt;

    auto elapsed          = now - begin;
    auto intervals_passed = elapsed.total_milliseconds() / interval.total_milliseconds();
    auto next =
        begin + fb::model::timespan(std::chrono::milliseconds((intervals_passed + 1) * interval.total_milliseconds()));

    if (next > end)
        return std::nullopt;

    return next;
}
