#ifndef __FB_LOG_COLLECTOR_H__
#define __FB_LOG_COLLECTOR_H__

#include <json/json.h>
#include <fb/amqp.h>
#include <random.h>

namespace fb {

class log_collector
{
private:
    std::unique_ptr<fb::amqp::socket> _amqp;
    std::string                       _server_id;
    std::string                       _server_name;
    size_t                            _queue_size;

public:
    log_collector(const std::string& hostname,
                  uint16_t           port,
                  const std::string& uid,
                  const std::string& pwd,
                  const std::string& server_id,
                  const std::string& server_name,
                  size_t             queue_size);
    ~log_collector() = default;

    log_collector(const log_collector&)             = delete;
    log_collector(log_collector&&)                  = delete;
    log_collector& operator= (const log_collector&) = delete;
    log_collector& operator= (log_collector&&)      = delete;

public:
    void write(const std::string& event_type, const Json::Value& data);

private:
    std::string serialize_log_entry(const Json::Value& log_entry) const;
    std::string select_random_routing_key() const;
};

} // namespace fb

#endif // !__FB_LOG_COLLECTOR_H__
