#include <fb/log_collector.h>

#include <fb/amqp.h>
#include <fb/context.h>
#include <fb/logger.h>
#include <fb/model/datetime.h>

#include <json/json.h>
#include <json/writer.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <exception>
#include <format>
#include <iterator>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

using namespace fb;

log_collector::log_collector(std::string_view        hostname,
                             uint16_t                port,
                             std::string_view        uid,
                             std::string_view        pwd,
                             std::string_view        server_id,
                             std::string_view        server_name,
                             std::optional<uint32_t> world) :
    _hostname(std::string(hostname)),
    _port(port),
    _uid(std::string(uid)),
    _pwd(std::string(pwd)),
    _server_id(std::string(server_id)),
    _server_name(std::string(server_name)),
    _world(world)
{
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
            auto count = std::min(this->_buffer.size(), MAX_BATCH_LOGS);
            batch.reserve(count);
            for (size_t i = 0; i < count; i++)
            {
                batch.push_back(std::move(this->_buffer.front()));
                this->_buffer.pop_front();
            }
        }

        if (batch.empty())
            continue;

        auto sent = this->publish(batch);
        if (sent < batch.size())
        {
            std::unique_lock<std::mutex> lock(this->_buffer_mutex);
            this->_buffer.insert(this->_buffer.begin(),
                                 std::make_move_iterator(batch.begin() + static_cast<std::ptrdiff_t>(sent)),
                                 std::make_move_iterator(batch.end()));
            if (this->_buffer.size() > MAX_BUFFERED_LOGS)
            {
                auto dropped = this->_buffer.size() - MAX_BUFFERED_LOGS;
                this->_buffer.erase(this->_buffer.begin(), this->_buffer.begin() + dropped);
                fb::logger::warn("Dropped {} oldest log entries while log RabbitMQ is unavailable", dropped);
            }

            this->_buffer_cv.wait_for(lock, batch_interval, [this] {
                return this->_stop_requested.load(std::memory_order_relaxed);
            });
        }
    }

    // Drain remaining buffer on shutdown
    this->_next_connect = {};
    while (true)
    {
        std::vector<Json::Value> batch;
        {
            std::lock_guard<std::mutex> lock(this->_buffer_mutex);
            if (this->_buffer.empty())
                break;
            auto count = std::min(this->_buffer.size(), MAX_BATCH_LOGS);
            batch.reserve(count);
            for (size_t i = 0; i < count; i++)
            {
                batch.push_back(std::move(this->_buffer.front()));
                this->_buffer.pop_front();
            }
        }

        auto sent = this->publish(batch);
        if (sent < batch.size())
        {
            std::lock_guard<std::mutex> lock(this->_buffer_mutex);
            fb::logger::warn("Dropped final {} log entries on shutdown", batch.size() - sent + this->_buffer.size());
            break;
        }
    }
}

size_t log_collector::publish(const std::vector<Json::Value>& batch)
{
    if (this->_amqp == nullptr)
    {
        auto now = std::chrono::steady_clock::now();
        if (now < this->_next_connect)
            return 0;

        this->_next_connect = now + RECONNECT_INTERVAL;
        auto amqp           = std::make_unique<fb::amqp::socket>();
        if (amqp->connect(this->_hostname, this->_port, this->_uid, this->_pwd, "/") == false ||
            amqp->confirm_select() == false)
        {
            fb::logger::warn("Failed to connect to log RabbitMQ at {}:{}", this->_hostname, this->_port);
            return 0;
        }
        this->_amqp = std::move(amqp);
    }

    Json::StreamWriterBuilder builder;
    builder["emitUTF8"]    = true;
    builder["indentation"] = "";
    auto writer            = std::unique_ptr<Json::StreamWriter>(builder.newStreamWriter());
    auto routing_key       = this->get_routing_key();

    size_t sent = 0;
    while (sent < batch.size())
    {
        auto   body  = std::string("[");
        size_t count = 0;
        while (sent + count < batch.size())
        {
            std::ostringstream stream;
            writer->write(batch[sent + count], &stream);
            auto entry = stream.str();
            if (entry.size() + 2 > MAX_BATCH_BYTES)
            {
                if (count > 0)
                    break;

                fb::logger::warn("Dropped log entry ({} bytes) larger than the batch limit", entry.size());
                sent++;
                continue;
            }

            auto separator = count > 0 ? size_t{1} : size_t{0};
            if (body.size() + separator + entry.size() + 1 > MAX_BATCH_BYTES)
                break;

            if (count > 0)
                body += ',';
            body += entry;
            count++;
        }

        if (count == 0)
            continue;

        body         += ']';
        auto message  = std::vector<uint8_t>(body.begin(), body.end());
        if (this->_amqp->publish("amq.direct", routing_key, message) == false)
        {
            fb::logger::warn("Failed to publish log batch ({} entries) with routing key: {}", count, routing_key);
            this->_amqp.reset();
            return sent;
        }
        sent += count;
    }
    return sent;
}

std::string log_collector::get_routing_key() const
{
    if (this->_world)
        return std::format("fb.{}.log", *this->_world);
    return "fb.log";
}
