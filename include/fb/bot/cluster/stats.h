#ifndef __FB_BOT_CLUSTER_STATS_H__
#define __FB_BOT_CLUSTER_STATS_H__

#include <json/json.h>

#include <array>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace fb::bot::cluster {

class stats
{
public:
    static constexpr size_t BUCKETS = 128;

    struct entry
    {
        uint64_t                      count    = 0;
        uint64_t                      failures = 0;
        uint64_t                      timed    = 0;
        double                        sum      = 0.0;
        double                        min      = 0.0;
        double                        max      = 0.0;
        std::array<uint64_t, BUCKETS> buckets{};

        void        add(double ms);
        void        merge(const entry& other);
        void        merge(const Json::Value& json);
        double      percentile(double ratio) const;
        Json::Value to_json() const;
    };

private:
    mutable std::mutex                               _mutex;
    std::map<std::string, entry>                     _entries;
    std::unordered_map<std::type_index, std::string> _names;

public:
    static size_t bucket(double ms);
    static double value(size_t bucket);

    std::string name(const std::type_info& type);
    void        add(const std::string& name, double ms, bool failed);
    void        count(const std::string& name, bool failed);
    Json::Value take();
};

} // namespace fb::bot::cluster

#endif
