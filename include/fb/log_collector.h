#ifndef __FB_LOG_COLLECTOR_H__
#define __FB_LOG_COLLECTOR_H__

#include <vector>
#include <chrono>
#include <json/json.h>
#include <fb/stream.h>
#include <fb/locker.h>
#include <async/task.h>

namespace fb {

class http_client;

class log_collector
{
private:
    http_client&                         _http;
    std::string                          _server_id;
    std::string                          _server_name;
    fb::locker<std::vector<Json::Value>> _log_buffer;
    size_t                               _max_buffer_size;
    size_t                               _flush_batch_size;
    std::string                          _log_server_url;

public:
    log_collector(http_client&       http,
                  const std::string& server_id,
                  const std::string& server_name,
                  const std::string& log_server_url,
                  size_t             max_buffer_size  = 1000,
                  size_t             flush_batch_size = 100);
    ~log_collector() = default;

    log_collector(const log_collector&)             = delete;
    log_collector(log_collector&&)                  = delete;
    log_collector& operator= (const log_collector&) = delete;
    log_collector& operator= (log_collector&&)      = delete;

    void              write(const std::string& event_type, const Json::Value& data);
    async::task<void> flush();

private:
    fb::stream serialize_and_compress(const std::vector<Json::Value>& logs) const;
};

} // namespace fb

#endif // !__FB_LOG_COLLECTOR_H__
