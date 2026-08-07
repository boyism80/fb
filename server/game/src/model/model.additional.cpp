#include <fb/model/model.h>

#include <croncpp.h>
#include <ctime>
#include <format>
#include <optional>

namespace {

std::tm to_tm(const fb::model::datetime& value)
{
    std::tm tm{};
    tm.tm_year  = static_cast<int>(value.year()) - 1900;
    tm.tm_mon   = static_cast<int>(value.month()) - 1;
    tm.tm_mday  = static_cast<int>(value.day());
    tm.tm_hour  = static_cast<int>(value.hours());
    tm.tm_min   = static_cast<int>(value.minutes());
    tm.tm_sec   = static_cast<int>(value.seconds());
    tm.tm_isdst = -1;
    return tm;
}

fb::model::datetime from_tm(const std::tm& tm)
{
    return fb::model::datetime(std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}",
                                           tm.tm_year + 1900,
                                           tm.tm_mon + 1,
                                           tm.tm_mday,
                                           tm.tm_hour,
                                           tm.tm_min,
                                           tm.tm_sec));
}

std::optional<fb::model::datetime> cron_next(const std::string& expression, const fb::model::datetime& after)
{
    try
    {
        auto cron = cron::make_cron(expression);
        auto next = cron::cron_next(cron, to_tm(after));
        if (next.tm_year == 0 && next.tm_mday == 0)
            return std::nullopt;
        return from_tm(next);
    }
    catch (const cron::bad_cronexpr&)
    {
        return std::nullopt;
    }
}

} // namespace

std::optional<fb::model::datetime> fb::model::schedule::next_execution(const fb::model::datetime& now) const
{
    if (this->script.empty() || this->func.empty())
        return std::nullopt;

    const auto has_at   = this->at.has_value();
    const auto has_cron = this->cron.has_value() && this->cron->empty() == false;
    if (has_at == has_cron) // require exactly one of at / cron
        return std::nullopt;

    if (has_at)
    {
        if (now < this->at.value())
            return this->at.value();
        return std::nullopt;
    }
    else
    {
        auto from = now;
        if (this->active.has_value())
        {
            const auto& active = this->active.value();
            if (active.end.has_value() && now > active.end.value())
                return std::nullopt;

            if (active.begin.has_value() && from < active.begin.value())
            {
                // cron_next is strictly after `from`; start 1s before begin so begin itself can match.
                from = active.begin.value() - fb::model::timespan(std::chrono::seconds(1));
            }
        }

        auto next = ::cron_next(this->cron.value(), from);
        if (!next.has_value())
            return std::nullopt;

        if (this->active.has_value() && this->active->end.has_value() && next.value() > this->active->end.value())
            return std::nullopt;

        return next;
    }
}
