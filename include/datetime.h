#ifndef __DATETIME_H__
#define __DATETIME_H__

#include <string>
#include <chrono>
#include <boost/xpressive/xpressive.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

class timespan
{
private:
    std::chrono::milliseconds _ms = std::chrono::milliseconds(0);

public:
    timespan()
    {}

    timespan(const std::string& f)
    {
        static const auto regex = boost::xpressive::sregex::compile("((?P<day>\\d+).)?(?P<hour>\\d{1,2}):(?P<min>\\d{1,2}):(?P<sec>\\d{1,2})(?:.(?P<ms>\\d+))?");
        auto what = boost::xpressive::smatch();
        if (boost::xpressive::regex_match(f, what, regex) == false)
            throw std::runtime_error("cannot parse timespan");

        auto day = what["day"].matched ? std::stoi(what["day"].str()) : 0;
        auto hours = std::stoi(what["hour"].str());
        auto mins = std::stoi(what["min"].str());
        auto secs = std::stoi(what["sec"].str());
        auto ms = 0;
        if (what["ms"].matched)
        {
            auto in = what["ms"].str();
            auto step = 100;
            for (int i = 0; i < std::min<uint32_t>(3, in.size()); i++)
            {
                auto v = in.at(i) - '0';
                ms += v * step;
                step /= 10;
            }
        }

        this->_ms = std::chrono::milliseconds(ms)
            + std::chrono::seconds(secs)
            + std::chrono::minutes(mins)
            + std::chrono::hours(hours)
            + std::chrono::hours(day * 24);
    }

    timespan(int days, int hours, int minutes, int seconds, int milliseconds) : _ms(std::chrono::milliseconds(milliseconds)
        + std::chrono::seconds(seconds)
        + std::chrono::minutes(minutes)
        + std::chrono::hours(hours)
        + std::chrono::hours(days * 24))
    {
    }

    template <class _Rep, class _Period>
    timespan(const std::chrono::duration<_Rep, _Period>& duration) : _ms(std::chrono::duration_cast<std::chrono::milliseconds>(duration))
    {}

    timespan(const timespan& ts) : _ms(ts._ms)
    {}

    ~timespan()
    {}

public:
    template <class _Rep, class _Period>
    operator std::chrono::duration<_Rep, _Period>() const
    {
        return std::chrono::duration_cast<std::chrono::duration<_Rep, _Period>>(this->_ms);
    }

    friend timespan operator + (const timespan& ts1, const timespan& ts2)
    {
        return timespan(ts1._ms + ts2._ms);
    }

    friend timespan operator - (const timespan& ts1, const timespan& ts2)
    {
        return timespan(ts1._ms - ts2._ms);
    }

    template <class _Rep, class _Period>
    friend timespan operator + (const timespan& ts, std::chrono::duration<_Rep, _Period> duration)
    {
        return timespan(ts._ms + duration);
    }

    template <class _Rep, class _Period>
    friend timespan operator - (const timespan& ts, std::chrono::duration<_Rep, _Period> duration)
    {
        return timespan(ts._ms - duration);
    }

    friend bool operator == (const timespan& ts1, const timespan& ts2)
    {
        return ts1._ms == ts2._ms;
    }

    template <class _Rep, class _Period>
    friend bool operator == (const timespan& ts1, std::chrono::duration<_Rep, _Period> duration)
    {
        return ts1._ms == duration;
    }

    friend bool operator != (const timespan& ts1, const timespan& ts2)
    {
        return ts1._ms != ts2._ms;
    }

    template <class _Rep, class _Period>
    friend bool operator != (const timespan& ts1, std::chrono::duration<_Rep, _Period> duration)
    {
        return ts1._ms != duration;
    }

    friend bool operator > (const timespan& ts1, const timespan& ts2)
    {
        return ts1._ms > ts2._ms;
    }

    template <class _Rep, class _Period>
    friend bool operator > (const timespan& ts1, std::chrono::duration<_Rep, _Period> duration)
    {
        return ts1._ms > duration;
    }

    friend bool operator < (const timespan& ts1, const timespan& ts2)
    {
        return ts1._ms < ts2._ms;
    }

    template <class _Rep, class _Period>
    friend bool operator < (const timespan& ts1, std::chrono::duration<_Rep, _Period> duration)
    {
        return ts1._ms < duration;
    }

    friend bool operator >= (const timespan& ts1, const timespan& ts2)
    {
        return ts1._ms >= ts2._ms;
    }

    template <class _Rep, class _Period>
    friend bool operator >= (const timespan& ts1, std::chrono::duration<_Rep, _Period> duration)
    {
        return ts1._ms >= duration;
    }

    friend bool operator <= (const timespan& ts1, const timespan& ts2)
    {
        return ts1._ms <= ts2._ms;
    }

    template <class _Rep, class _Period>
    friend bool operator <= (const timespan& ts1, std::chrono::duration<_Rep, _Period> duration)
    {
        return ts1._ms <= duration;
    }

    timespan& operator += (const timespan& ts)
    {
        this->_ms += ts._ms;
        return *this;
    }

    timespan& operator -= (const timespan& ts)
    {
        this->_ms -= ts._ms;
        return *this;
    }

    template <class _Rep, class _Period>
    timespan& operator += (std::chrono::duration<_Rep, _Period> duration)
    {
        this->_ms += duration;
        return *this;
    }

    template <class _Rep, class _Period>
    timespan& operator -= (std::chrono::duration<_Rep, _Period> duration)
    {
        this->_ms -= duration;
        return *this;
    }

    int milliseconds() const
    {
        return this->_ms.count() % 1000;
    }

    int seconds() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(this->_ms).count() % 60;
    }

    int minutes() const
    {
        return std::chrono::duration_cast<std::chrono::minutes>(this->_ms).count() % 60;
    }

    int hours() const
    {
        return std::chrono::duration_cast<std::chrono::hours>(this->_ms).count() % 24;
    }

    int days() const
    {
        return std::chrono::duration_cast<std::chrono::hours>(this->_ms).count() / 24;
    }

    int64_t total_milliseconds() const
    {
        return this->_ms.count();
    }

    bool empty() const
    {
        return this->total_milliseconds() == 0;
    }

    std::string to_string()
    {
        auto sstream = std::stringstream();
        auto days = this->days();
        if (days > 0)
            sstream << days << '.';

        sstream << std::setw(2) << std::setfill('0') << this->hours() << ':'
            << std::setw(2) << std::setfill('0') << this->minutes() << ':'
            << std::setw(2) << std::setfill('0') << this->seconds();

        auto ms = this->milliseconds();
        if (ms > 0)
            sstream << '.' << ms;

        return sstream.str();
    }
};

class datetime
{
private:
    boost::posix_time::ptime _ptime;

public:
    datetime() : _ptime(boost::posix_time::second_clock::local_time())
    {}

    datetime(const std::string& f) : _ptime(boost::posix_time::time_from_string(f))
    {}

    datetime(const datetime& dt) : _ptime(dt._ptime)
    {}

public:
    uint16_t year() const
    {
        return this->_ptime.date().year();
    }

    uint16_t month() const
    {
        return this->_ptime.date().month();
    }

    uint16_t day() const
    {
        return this->_ptime.date().day();
    }

    uint16_t hours() const
    {
        return this->_ptime.time_of_day().hours();
    }

    uint16_t minutes() const
    {
        return this->_ptime.time_of_day().minutes();
    }

    uint16_t seconds() const
    {
        return this->_ptime.time_of_day().seconds();
    }

    uint16_t milliseconds() const
    {
        return this->_ptime.time_of_day().total_milliseconds() % 1000;
    }

    datetime& add_days(int days)
    {
        this->_ptime += boost::posix_time::hours(days * 24);
        return *this;
    }

    datetime& add_hours(int h)
    {
        this->_ptime += boost::posix_time::hours(h);
        return *this;
    }

    datetime& add_minutes(int min)
    {
        this->_ptime += boost::posix_time::minutes(min);
        return *this;
    }

    datetime& add_seconds(int s)
    {
        this->_ptime += boost::posix_time::seconds(s);
        return *this;
    }

    datetime& add_milliseconds(int ms)
    {
        this->_ptime += boost::posix_time::milliseconds(ms);
        return *this;
    }

    datetime& add_timespan(const timespan& ts)
    {
        this->add_milliseconds(ts.total_milliseconds());
        return *this;
    }

    friend datetime operator + (const datetime& dt, const timespan& ts)
    {
        auto result = datetime(dt);
        result.add_timespan(ts);
        return result;
    }

    friend datetime operator - (const datetime& dt, const timespan& ts)
    {
        auto result = datetime(dt);
        result.add_milliseconds(-ts.total_milliseconds());
        return result;
    }

    friend timespan operator - (const datetime& dt1, const datetime& dt2)
    {
        auto diff = dt1._ptime - dt2._ptime;
        return timespan(boost::posix_time::to_simple_string(diff));
    }

    datetime& operator += (const timespan& ts)
    {
        this->add_timespan(ts);
        return *this;
    }

    datetime& operator -= (const timespan& ts)
    {
        this->add_milliseconds(-ts.total_milliseconds());
        return *this;
    }

    friend bool operator == (const datetime& dt1, const datetime& dt2)
    {
        return dt1._ptime == dt2._ptime;
    }

    friend bool operator != (const datetime& dt1, const datetime& dt2)
    {
        return dt1._ptime != dt2._ptime;
    }

    friend bool operator > (const datetime& dt1, const datetime& dt2)
    {
        return dt1._ptime > dt2._ptime;
    }

    friend bool operator < (const datetime& dt1, const datetime& dt2)
    {
        return dt1._ptime < dt2._ptime;
    }

    friend bool operator >= (const datetime& dt1, const datetime& dt2)
    {
        return dt1._ptime >= dt2._ptime;
    }

    friend bool operator <= (const datetime& dt1, const datetime& dt2)
    {
        return dt1._ptime <= dt2._ptime;
    }

    template <class _Rep, class _Period>
    friend datetime operator + (const datetime& dt, std::chrono::duration<_Rep, _Period> duration)
    {
        return datetime(dt) + timespan(duration);
    }

    template <class _Rep, class _Period>
    friend datetime operator - (const datetime& dt, std::chrono::duration<_Rep, _Period> duration)
    {
        return datetime(dt) - timespan(duration);
    }

    template <class _Rep, class _Period>
    datetime& operator += (std::chrono::duration<_Rep, _Period> duration)
    {
        (*this) += timespan(duration);
        return *this;
    }

    template <class _Rep, class _Period>
    datetime& operator -= (std::chrono::duration<_Rep, _Period> duration)
    {
        (*this) -= timespan(duration);
        return *this;
    }

    std::string to_string() const
    {
        auto sstream = std::stringstream();
        sstream << this->year() << '-'
            << std::setw(2) << std::setfill('0') << this->month() << '-'
            << std::setw(2) << std::setfill('0') << this->day() << ' '
            << std::setw(2) << std::setfill('0') << this->hours() << ':'
            << std::setw(2) << std::setfill('0') << this->minutes() << ':'
            << std::setw(2) << std::setfill('0') << this->seconds();

        auto ms = this->milliseconds();
        if (ms > 0)
            sstream << ':' << ms;

        return sstream.str();
    }
};

#endif