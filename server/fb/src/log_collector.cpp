#include <fb/log_collector.h>
#include <fb/context.h>
#include <fb/amqp.h>
#include <fb/logger.h>
#include <fb/model/datetime.h>
#include <json/json.h>
#include <json/writer.h>
#include <sstream>
#include <chrono>
#include <format>
#include <vector>

using namespace fb;

log_collector::log_collector(std::string_view hostname,
                             uint16_t         port,
                             std::string_view uid,
                             std::string_view pwd,
                             std::string_view server_id,
                             std::string_view server_name,
                             uint32_t         world) :
    _server_id(std::string(server_id)),
    _server_name(std::string(server_name)),
    _world(world)
{
    this->_amqp    = std::make_unique<fb::amqp::socket>();
    auto connected = this->_amqp->connect(hostname, port, uid, pwd, "/");
    if (!connected)
    {
        fb::logger::warn("Failed to connect to log RabbitMQ at {}:{}", hostname, port);
        this->_amqp.reset();
    }

    this->_worker = std::thread(&log_collector::worker_run, this);
}

log_collector::~log_collector()
{
    stop();
}

void log_collector::write(std::string_view event_type, const Json::Value& data)
{
    try
    {
        Json::Value entry;
        entry["timestamp"]   = fb::model::datetime().to_string();
        entry["event"]       = std::string(event_type);
        entry["server_id"]   = this->_server_id;
        entry["server_name"] = this->_server_name;
        entry["data"]        = data;

        if (auto* ctx = context::local::try_get())
            entry["transaction_id"] = ctx->transaction_id;

        std::lock_guard<std::mutex> lock(this->_buffer_mutex);
        this->_buffer.push_back(std::move(entry));
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("Failed to buffer log: {}", e.what());
    }
}

void log_collector::stop()
{
    if (this->_stop_requested.exchange(true))
        return;

    this->_buffer_cv.notify_all();

    if (this->_worker.joinable())
        this->_worker.join();
}

void log_collector::worker_run()
{
    const auto batch_interval = std::chrono::seconds(1);

    while (!this->_stop_requested.load(std::memory_order_relaxed))
    {
        std::vector<Json::Value> batch;
        {
            std::unique_lock<std::mutex> lock(this->_buffer_mutex);
            this->_buffer_cv.wait_for(lock, batch_interval, [this] {
                return this->_stop_requested.load(std::memory_order_relaxed) || !this->_buffer.empty();
            });
            batch.reserve(this->_buffer.size());
            while (!this->_buffer.empty())
            {
                batch.push_back(std::move(this->_buffer.front()));
                this->_buffer.pop_front();
            }
        }

        if (batch.empty())
            continue;

        if (this->_amqp != nullptr)
        {
            auto body        = this->serialize_log_array(batch);
            auto message     = std::vector<uint8_t>(body.begin(), body.end());
            auto routing_key = this->get_routing_key();
            if (!this->_amqp->publish("amq.direct", routing_key, message))
                fb::logger::warn("Failed to publish log batch ({} entries) with routing key: {}",
                                 batch.size(),
                                 routing_key);
        }
    }

    // Drain remaining buffer on shutdown
    while (true)
    {
        std::vector<Json::Value> batch;
        {
            std::lock_guard<std::mutex> lock(this->_buffer_mutex);
            if (this->_buffer.empty())
                break;
            batch.reserve(this->_buffer.size());
            while (!this->_buffer.empty())
            {
                batch.push_back(std::move(this->_buffer.front()));
                this->_buffer.pop_front();
            }
        }

        if (this->_amqp != nullptr)
        {
            auto body        = this->serialize_log_array(batch);
            auto message     = std::vector<uint8_t>(body.begin(), body.end());
            auto routing_key = this->get_routing_key();
            if (!this->_amqp->publish("amq.direct", routing_key, message))
                fb::logger::warn("Failed to publish final log batch ({} entries) with routing key: {}",
                                 batch.size(),
                                 routing_key);
        }
    }
}

std::string log_collector::serialize_log_array(const std::vector<Json::Value>& entries) const
{
    Json::Value root(Json::arrayValue);
    for (const auto& e : entries)
    {
        root.append(e);
    }

    Json::StreamWriterBuilder builder;
    builder["emitUTF8"]    = true;
    builder["indentation"] = "";

    auto               writer = std::unique_ptr<Json::StreamWriter>(builder.newStreamWriter());
    std::ostringstream stream;
    writer->write(root, &stream);
    return stream.str();
}

std::string log_collector::get_routing_key() const
{
    if (this->_world > 0)
        return std::format("fb.{}.log", this->_world);
    return "fb.log";
}
