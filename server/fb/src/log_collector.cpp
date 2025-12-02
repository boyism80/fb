#include <fb/log_collector.h>
#include <fb/amqp.h>
#include <fb/logger.h>
#include <json/json.h>
#include <chrono>
#include <format>
#include <random.h>

using namespace fb;

log_collector::log_collector(const std::string& hostname,
                             uint16_t           port,
                             const std::string& uid,
                             const std::string& pwd,
                             const std::string& server_id,
                             const std::string& server_name,
                             size_t             queue_size) :
    _server_id(server_id),
    _server_name(server_name),
    _queue_size(queue_size)
{
    // Create and connect to RabbitMQ
    this->_amqp    = std::make_unique<fb::amqp::socket>();
    auto connected = this->_amqp->connect(hostname, port, uid, pwd, "/");
    if (!connected)
    {
        fb::logger::warn("Failed to connect to log RabbitMQ at {}:{}", hostname, port);
        this->_amqp.reset();
    }
}

void log_collector::write(const std::string& event_type, const Json::Value& data)
{
    try
    {
        // Create log entry
        Json::Value log_entry;
        log_entry["timestamp"]   = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        log_entry["event"]       = event_type;
        log_entry["server_id"]   = this->_server_id;
        log_entry["server_name"] = this->_server_name;
        log_entry["data"]        = data;

        // Serialize to JSON string
        auto json_string = this->serialize_log_entry(log_entry);

        // Convert to byte vector
        auto message = std::vector<uint8_t>(json_string.begin(), json_string.end());

        // Select random routing key
        auto routing_key = this->select_random_routing_key();

        // Publish to RabbitMQ (using amq.direct exchange with routing key)
        if (this->_amqp == nullptr)
        {
            fb::logger::warn("AMQP connection not available, skipping log publish");
            return;
        }

        if (!this->_amqp->publish("amq.direct", routing_key, message))
        {
            fb::logger::warn("Failed to publish log with routing key: {}", routing_key);
        }
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("Failed to write log: {}", e.what());
    }
}

std::string log_collector::serialize_log_entry(const Json::Value& log_entry) const
{
    auto writer      = Json::FastWriter{};
    auto json_string = writer.write(log_entry);
    // Remove trailing newline from FastWriter
    if (!json_string.empty() && json_string.back() == '\n')
    {
        json_string.pop_back();
    }
    return json_string;
}

std::string log_collector::select_random_routing_key() const
{
    auto queue_index = ::random<size_t>(0, this->_queue_size - 1);
    return std::format("fb.log.{}", queue_index);
}
