#ifndef __FB_LOG_COLLECTOR_H__
#define __FB_LOG_COLLECTOR_H__

#include <json/json.h>
#include <fb/amqp.h>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

namespace fb {

class log_collector
{
private:
    std::unique_ptr<fb::amqp::socket> _amqp;
    std::string                       _server_id;
    std::string                       _server_name;
    uint32_t                          _world;

    std::deque<Json::Value>           _buffer;
    std::mutex                        _buffer_mutex;
    std::condition_variable           _buffer_cv;
    std::atomic<bool>                 _stop_requested{false};
    std::thread                       _worker;

public:
    log_collector(std::string_view hostname,
                  uint16_t           port,
                  std::string_view uid,
                  std::string_view pwd,
                  std::string_view server_id,
                  std::string_view server_name,
                  uint32_t           world);
    ~log_collector();

    log_collector(const log_collector&)             = delete;
    log_collector(log_collector&&)                  = delete;
    log_collector& operator= (const log_collector&) = delete;
    log_collector& operator= (log_collector&&)      = delete;

public:
    void write(std::string_view event_type, const Json::Value& data);

    /**
     * Requests shutdown and blocks until the worker thread has flushed all
     * buffered logs and exited. Call before destruction if not relying on dtor.
     */
    void stop();

private:
    void worker_run();
    std::string serialize_log_array(const std::vector<Json::Value>& entries) const;
    std::string get_routing_key() const;
};

} // namespace fb

#endif // !__FB_LOG_COLLECTOR_H__
