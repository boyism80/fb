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
    /// <summary>
    /// Initializes a new instance of the log_collector class.
    /// Connects to RabbitMQ for log publishing and sets up queue selection.
    /// </summary>
    /// <param name="hostname">RabbitMQ server hostname.</param>
    /// <param name="port">RabbitMQ server port.</param>
    /// <param name="uid">RabbitMQ user ID.</param>
    /// <param name="pwd">RabbitMQ password.</param>
    /// <param name="server_id">Unique identifier for the server instance.</param>
    /// <param name="server_name">Human-readable name of the server instance.</param>
    /// <param name="queue_size">Number of log queues available (fb.log.0 to fb.log.{queue_size-1}).</param>
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

    /// <summary>
    /// Writes a log entry to RabbitMQ by immediately publishing to a randomly selected queue.
    /// </summary>
    /// <param name="event_type">Type of the log event (e.g., "login", "logout", "item_use").</param>
    /// <param name="data">JSON data associated with the log event.</param>
    void write(const std::string& event_type, const Json::Value& data);

private:
    /// <summary>
    /// Serializes a log entry to JSON string format.
    /// </summary>
    /// <param name="log_entry">The log entry to serialize.</param>
    /// <returns>JSON string representation of the log entry.</returns>
    std::string serialize_log_entry(const Json::Value& log_entry) const;

    /// <summary>
    /// Selects a random routing key from available log routing keys.
    /// </summary>
    /// <returns>Routing key in format "fb.log.{index}" where index is between 0 and queue_size-1.</returns>
    std::string select_random_routing_key() const;
};

} // namespace fb

#endif // !__FB_LOG_COLLECTOR_H__
