#include <fb/bot/cluster/stats.h>

#ifndef _WIN32
#include <cxxabi.h>
#endif

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <format>
#include <vector>

using namespace fb::bot::cluster;

void stats::entry::add(double ms)
{
    this->min = this->timed == 0 ? ms : std::min(this->min, ms);
    this->max = this->timed == 0 ? ms : std::max(this->max, ms);
    this->timed++;
    this->sum += ms;
    this->buckets.at(stats::bucket(ms))++;
}

void stats::entry::merge(const entry& other)
{
    if (other.timed > 0)
    {
        this->min = this->timed == 0 ? other.min : std::min(this->min, other.min);
        this->max = this->timed == 0 ? other.max : std::max(this->max, other.max);
    }

    this->count    += other.count;
    this->failures += other.failures;
    this->timed    += other.timed;
    this->sum      += other.sum;
    for (size_t i = 0; i < BUCKETS; i++)
    {
        this->buckets.at(i) += other.buckets.at(i);
    }
}

void stats::entry::merge(const Json::Value& json)
{
    auto other     = entry{};
    other.count    = json.get("count", 0).asUInt64();
    other.failures = json.get("failures", 0).asUInt64();
    other.timed    = json.get("timed", 0).asUInt64();
    other.sum      = json.get("sum", 0.0).asDouble();
    other.min      = json.get("min", 0.0).asDouble();
    other.max      = json.get("max", 0.0).asDouble();
    for (auto& key : json["buckets"].getMemberNames())
    {
        auto index = static_cast<size_t>(std::strtoul(key.c_str(), nullptr, 10));
        if (index < BUCKETS)
            other.buckets.at(index) = json["buckets"][key].asUInt64();
    }
    this->merge(other);
}

double stats::entry::percentile(double ratio) const
{
    if (this->timed == 0)
        return 0.0;

    auto target = static_cast<uint64_t>(std::ceil(this->timed * ratio));
    auto total  = uint64_t{0};
    for (size_t i = 0; i < BUCKETS; i++)
    {
        total += this->buckets.at(i);
        if (total >= target)
            return std::clamp(stats::value(i), this->min, this->max);
    }
    return this->max;
}

Json::Value stats::entry::to_json() const
{
    auto json        = Json::Value{};
    json["count"]    = static_cast<Json::UInt64>(this->count);
    json["failures"] = static_cast<Json::UInt64>(this->failures);
    json["timed"]    = static_cast<Json::UInt64>(this->timed);
    json["sum"]      = this->sum;
    json["min"]      = this->min;
    json["max"]      = this->max;
    json["buckets"]  = Json::Value{Json::objectValue};
    for (size_t i = 0; i < BUCKETS; i++)
    {
        if (this->buckets.at(i) > 0)
            json["buckets"][std::to_string(i)] = static_cast<Json::UInt64>(this->buckets.at(i));
    }
    return json;
}

size_t stats::bucket(double ms)
{
    if (ms < 1.0)
        return 0;

    auto index = static_cast<size_t>(std::floor(std::log(ms) / std::log(1.1))) + 1;
    return std::min(index, BUCKETS - 1);
}

double stats::value(size_t bucket)
{
    if (bucket == 0)
        return 0.5;

    return std::pow(1.1, static_cast<double>(bucket) - 0.5);
}

std::string stats::name(const std::type_info& type)
{
    auto lock  = std::lock_guard(this->_mutex);
    auto found = this->_names.find(type);
    if (found != this->_names.end())
        return found->second;

#ifdef _WIN32
    auto text = std::string{type.name()};
#else
    auto status    = 0;
    auto demangled = abi::__cxa_demangle(type.name(), nullptr, nullptr, &status);
    auto text      = std::string{status == 0 && demangled != nullptr ? demangled : type.name()};
    std::free(demangled);
#endif

    for (auto prefix : {"struct ", "class "})
    {
        if (text.starts_with(prefix))
            text = text.substr(std::string_view{prefix}.size());
    }
    text = text.substr(0, text.find('<'));

    auto parts = std::vector<std::string>{};
    for (auto begin = size_t{0}; begin <= text.size();)
    {
        auto end = text.find("::", begin);
        parts.push_back(text.substr(begin, end == std::string::npos ? std::string::npos : end - begin));
        if (end == std::string::npos)
            break;
        begin = end + 2;
    }

    auto protocol = std::find(parts.begin(), parts.end(), "protocol");
    auto result   = protocol != parts.end() && std::next(protocol) != parts.end() && parts.size() >= 2
                        ? std::format("{}:{}", *std::next(protocol), parts.back())
                        : parts.back();
    this->_names.insert({type, result});
    return result;
}

void stats::add(const std::string& name, double ms, bool failed)
{
    auto  lock  = std::lock_guard(this->_mutex);
    auto& entry = this->_entries[name];
    entry.count++;
    if (failed)
        entry.failures++;
    entry.add(ms);
}

void stats::count(const std::string& name, bool failed)
{
    auto  lock  = std::lock_guard(this->_mutex);
    auto& entry = this->_entries[name];
    entry.count++;
    if (failed)
        entry.failures++;
}

Json::Value stats::take()
{
    auto entries = std::map<std::string, entry>{};
    {
        auto lock = std::lock_guard(this->_mutex);
        entries.swap(this->_entries);
    }

    auto json = Json::Value{Json::objectValue};
    for (auto& [name, entry] : entries)
    {
        json[name] = entry.to_json();
    }
    return json;
}
