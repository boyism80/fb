#ifndef __FB_BOT_CLUSTER_LINK_H__
#define __FB_BOT_CLUSTER_LINK_H__

#include <json/json.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

namespace fb::bot::cluster {

class link
{
public:
    static constexpr std::string_view MARKER = "@cluster ";

    using handler = std::function<void(const Json::Value&)>;

private:
    std::atomic<bool>                        _enabled = false;
    std::mutex                               _mutex;
    std::unordered_map<std::string, handler> _handlers;

public:
    bool enabled() const;
    void enable();
    void on(const std::string& type, handler fn);
    void send(const Json::Value& message) const;

    static std::string serialize(const Json::Value& message);
    static bool        parse(std::string_view text, Json::Value& message);
    static bool        decode(std::string_view text, Json::Value& message);
};

} // namespace fb::bot::cluster

#endif
