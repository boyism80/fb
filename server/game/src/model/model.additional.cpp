#include <fb/model/model.h>

#include <croncpp.h>
#include <ctime>
#include <format>
#include <optional>
#include <boost/date_time/gregorian/gregorian.hpp>

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

fb::model::datetime with_ymd(const fb::model::datetime& base, int year, int month, int day)
{
    const auto last = boost::gregorian::gregorian_calendar::end_of_month_day(year, month);
    if (day > last)
        day = last;

    return fb::model::datetime(std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}",
                                           year,
                                           month,
                                           day,
                                           base.hours(),
                                           base.minutes(),
                                           base.seconds()));
}

fb::model::datetime add_solar_months(const fb::model::datetime& base, int months)
{
    auto y = static_cast<int>(base.year());
    auto m = static_cast<int>(base.month()) + months;
    while (m > 12)
    {
        ++y;
        m -= 12;
    }
    while (m < 1)
    {
        --y;
        m += 12;
    }
    return with_ymd(base, y, m, static_cast<int>(base.day()));
}

fb::model::datetime add_solar_years(const fb::model::datetime& base, int years)
{
    return with_ymd(base,
                    static_cast<int>(base.year()) + years,
                    static_cast<int>(base.month()),
                    static_cast<int>(base.day()));
}

bool in_window(const fb::model::datetime& now, const fb::model::datetime& occ, const fb::model::timespan& duration)
{
    if (now < occ)
        return false;
    return now < (occ + duration);
}

std::optional<fb::model::datetime>
lunar_occurrence(uint16_t year, uint16_t month, uint16_t day, uint16_t hours, uint16_t minutes, uint16_t seconds)
{
    try
    {
        fb::model::lunar_date lunar{};
        lunar.year  = year;
        lunar.month = month;
        lunar.day   = day;
        lunar.leap  = false;
        return fb::model::datetime::from_lunar(lunar, hours, minutes, seconds);
    }
    catch (const std::exception&)
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

bool fb::model::event::is_active(const fb::model::datetime& now) const
{
    using period_t = fb::model::enum_value::EVENT_PERIOD;

    const auto period = this->period.value_or(period_t::NONE);
    const auto every  = this->period_value.value_or(0);
    if (period != period_t::NONE && every == 0)
        return false;

    if (period == period_t::NONE || this->period.has_value() == false)
        return in_window(now, this->start, this->duration);

    if (this->lunar)
    {
        fb::model::lunar_date anchor{};
        fb::model::lunar_date now_lunar{};
        try
        {
            anchor    = this->start.to_lunar();
            now_lunar = now.to_lunar();
        }
        catch (const std::exception&)
        {
            return false;
        }

        if (now_lunar.leap)
            return false;

        if (period == period_t::YEAR)
        {
            for (int delta = -1; delta <= 1; ++delta)
            {
                const auto year = static_cast<int>(now_lunar.year) + delta * static_cast<int>(every);
                if (year <= 0)
                    continue;

                auto occ = lunar_occurrence(static_cast<uint16_t>(year),
                                            anchor.month,
                                            anchor.day,
                                            this->start.hours(),
                                            this->start.minutes(),
                                            this->start.seconds());
                if (occ.has_value() && in_window(now, occ.value(), this->duration))
                    return true;
            }
            return false;
        }

        if (period == period_t::MONTH)
        {
            auto cursor_year  = static_cast<int>(now_lunar.year);
            auto cursor_month = static_cast<int>(now_lunar.month);
            for (int i = 0; i < 6; ++i)
            {
                cursor_month -= static_cast<int>(every);
                while (cursor_month < 1)
                {
                    cursor_month += 12;
                    --cursor_year;
                }
            }
            for (int i = 0; i < 12; ++i)
            {
                if (cursor_year > 0)
                {
                    auto occ = lunar_occurrence(static_cast<uint16_t>(cursor_year),
                                                static_cast<uint16_t>(cursor_month),
                                                anchor.day,
                                                this->start.hours(),
                                                this->start.minutes(),
                                                this->start.seconds());
                    if (occ.has_value() && in_window(now, occ.value(), this->duration))
                        return true;
                }
                cursor_month += static_cast<int>(every);
                while (cursor_month > 12)
                {
                    cursor_month -= 12;
                    ++cursor_year;
                }
            }
            return false;
        }

        if (period == period_t::DAY)
        {
            if (now < this->start)
                return false;

            const auto elapsed = now - this->start;
            const auto step_ms = std::chrono::milliseconds(std::chrono::hours(24 * static_cast<int>(every)));
            if (step_ms.count() <= 0)
                return false;

            const auto n = elapsed.total_milliseconds() / step_ms.count();
            for (int back = 0; back <= 1; ++back)
            {
                if (n < back)
                    continue;
                auto occ = this->start + fb::model::timespan(std::chrono::milliseconds((n - back) * step_ms.count()));
                if (in_window(now, occ, this->duration))
                    return true;
            }
            return false;
        }

        return false;
    }

    if (period == period_t::DAY)
    {
        if (now < this->start)
            return false;

        const auto elapsed = now - this->start;
        const auto step_ms = std::chrono::milliseconds(std::chrono::hours(24 * static_cast<int>(every)));
        if (step_ms.count() <= 0)
            return false;

        const auto n = elapsed.total_milliseconds() / step_ms.count();
        for (int back = 0; back <= 1; ++back)
        {
            if (n < back)
                continue;
            auto occ = this->start + fb::model::timespan(std::chrono::milliseconds((n - back) * step_ms.count()));
            if (in_window(now, occ, this->duration))
                return true;
        }
        return false;
    }

    if (period == period_t::MONTH)
    {
        const auto rough = (static_cast<int>(now.year()) - static_cast<int>(this->start.year())) * 12 +
                           (static_cast<int>(now.month()) - static_cast<int>(this->start.month()));
        const auto base_n = rough / static_cast<int>(every);
        for (int delta = -2; delta <= 2; ++delta)
        {
            const auto n = base_n + delta;
            if (n < 0)
                continue;
            auto occ = add_solar_months(this->start, n * static_cast<int>(every));
            if (in_window(now, occ, this->duration))
                return true;
        }
        return false;
    }

    if (period == period_t::YEAR)
    {
        const auto base_n = static_cast<int>(now.year()) - static_cast<int>(this->start.year());
        for (int delta = -2; delta <= 2; ++delta)
        {
            const auto n = base_n / static_cast<int>(every) + delta;
            if (n < 0)
                continue;
            auto occ = add_solar_years(this->start, n * static_cast<int>(every));
            if (in_window(now, occ, this->duration))
                return true;
        }
        return false;
    }

    return false;
}

bool fb::model::mob_spawn::conditions_met(const fb::model::datetime& now) const
{
    for (const auto& dsl : this->condition)
    {
        if (dsl.header != fb::model::enum_value::DSL::event)
            continue;

        const auto params = fb::model::dsl::event(dsl.params);
        if (fb::model::table::event->contains(params.id) == false)
            return false;
        if (fb::model::table::event[params.id].is_active(now) == false)
            return false;
    }
    return true;
}
