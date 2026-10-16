#include <fb/model/model.h>

#include <boost/date_time/gregorian/gregorian.hpp>

#include <chrono>
#include <cstdint>
#include <exception>
#include <format>
#include <optional>
#include <string>

#include <croncpp.h>

std::optional<fb::model::datetime> fb::model::schedule::next_execution(const fb::model::datetime& now) const
{
    if (this->script.empty() || this->func.empty())
        return std::nullopt;

    const auto has_at   = this->at.has_value();
    const auto has_cron = this->cron.has_value() && this->cron->empty() == false;
    if (has_at == has_cron) // require exactly one of at / cron
        return std::nullopt;

    // cron, at, and active bounds are Korean civil times. `now` is UTC.
    const auto now_kst = now.to_kst();
    if (has_at)
    {
        if (now_kst < this->at.value())
            return this->at.value().kst_to_utc();
        return std::nullopt;
    }
    else
    {
        auto from = now_kst;
        if (this->active.has_value())
        {
            const auto& active = this->active.value();
            if (active.end.has_value() && now_kst > active.end.value())
                return std::nullopt;

            if (active.begin.has_value() && from < active.begin.value())
            {
                // cron_next is strictly after `from`; start 1s before begin so begin itself can match.
                from = active.begin.value() - fb::model::timespan(std::chrono::seconds(1));
            }
        }

        auto next = std::optional<fb::model::datetime>{};
        try
        {
            auto after     = std::tm{};
            after.tm_year  = static_cast<int>(from.year()) - 1900;
            after.tm_mon   = static_cast<int>(from.month()) - 1;
            after.tm_mday  = static_cast<int>(from.day());
            after.tm_hour  = static_cast<int>(from.hours());
            after.tm_min   = static_cast<int>(from.minutes());
            after.tm_sec   = static_cast<int>(from.seconds());
            after.tm_isdst = -1;

            auto found = ::cron::cron_next(::cron::make_cron(this->cron.value()), after);
            if (found.tm_year != 0 || found.tm_mday != 0)
            {
                next = fb::model::datetime(std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}",
                                                       found.tm_year + 1900,
                                                       found.tm_mon + 1,
                                                       found.tm_mday,
                                                       found.tm_hour,
                                                       found.tm_min,
                                                       found.tm_sec));
            }
        }
        catch (const ::cron::bad_cronexpr&)
        { }

        if (!next.has_value())
            return std::nullopt;

        if (this->active.has_value() && this->active->end.has_value() && next.value() > this->active->end.value())
            return std::nullopt;

        return next.value().kst_to_utc();
    }
}

bool fb::model::event::in_window(const fb::model::datetime& now,
                                 const fb::model::datetime& occurrence,
                                 const fb::model::timespan& duration)
{
    if (now < occurrence)
        return false;
    return now < (occurrence + duration);
}

std::optional<fb::model::datetime> fb::model::event::lunar_occurrence(uint16_t year,
                                                                      uint16_t month,
                                                                      uint16_t day,
                                                                      uint16_t hours,
                                                                      uint16_t minutes,
                                                                      uint16_t seconds)
{
    try
    {
        fb::model::lunar_date date{};
        date.year  = year;
        date.month = month;
        date.day   = day;
        date.leap  = false;
        return fb::model::datetime::from_lunar(date, hours, minutes, seconds);
    }
    catch (const std::exception&)
    {
        return std::nullopt;
    }
}

fb::model::datetime fb::model::event::solar_occurrence(const fb::model::datetime& start, int year, int month, int day)
{
    const auto last = boost::gregorian::gregorian_calendar::end_of_month_day(year, month);
    if (day > last)
        day = last;

    return fb::model::datetime(std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}",
                                           year,
                                           month,
                                           day,
                                           start.hours(),
                                           start.minutes(),
                                           start.seconds()));
}

bool fb::model::event::is_active(const fb::model::datetime& now_utc) const
{
    const auto now = now_utc.to_kst();
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
            auto year  = static_cast<int>(this->start.year());
            auto month = static_cast<int>(this->start.month()) + n * static_cast<int>(every);
            while (month > 12)
            {
                ++year;
                month -= 12;
            }
            while (month < 1)
            {
                --year;
                month += 12;
            }
            auto occ = solar_occurrence(this->start, year, month, static_cast<int>(this->start.day()));
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
            auto occ = solar_occurrence(this->start,
                                        static_cast<int>(this->start.year()) + n * static_cast<int>(every),
                                        static_cast<int>(this->start.month()),
                                        static_cast<int>(this->start.day()));
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
