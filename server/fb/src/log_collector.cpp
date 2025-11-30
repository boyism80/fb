#include <fb/log_collector.h>
#include <fb/http_client.h>
#include <json/json.h>
#include <fb/stream.h>
#include <chrono>
#include <format>

using namespace fb;
using namespace std::chrono_literals;

log_collector::log_collector(http_client&       http,
                             const std::string& server_id,
                             const std::string& server_name,
                             const std::string& log_server_url,
                             size_t             max_buffer_size,
                             size_t             flush_batch_size) :
    _http(http),
    _server_id(server_id),
    _server_name(server_name),
    _max_buffer_size(max_buffer_size),
    _flush_batch_size(flush_batch_size),
    _log_server_url(log_server_url)
{ }

void log_collector::write(const std::string& event_type, const Json::Value& data)
{
    Json::Value log_entry;
    log_entry["timestamp"]   = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    log_entry["event"]       = event_type;
    log_entry["server_id"]   = this->_server_id;
    log_entry["server_name"] = this->_server_name;
    log_entry["data"]        = data;

    this->_log_buffer.write([this, log_entry](auto& buffer) {
        if (buffer.size() >= this->_max_buffer_size)
        {
            // Buffer overflow: remove oldest entry
            buffer.erase(buffer.begin());
        }

        buffer.push_back(log_entry);
    });
}

async::task<void> log_collector::flush()
{
    while (true)
    {
        std::vector<Json::Value> logs_to_send;

        auto result = co_await this->_log_buffer.async_write([this](auto& buffer) -> async::task<std::vector<Json::Value>> {
            if (buffer.empty())
                co_return std::vector<Json::Value>{};

            // Take up to flush_batch_size logs
            auto                     batch_size = std::min(this->_flush_batch_size, buffer.size());
            std::vector<Json::Value> extracted;
            extracted.reserve(batch_size);
            std::move(buffer.begin(), buffer.begin() + batch_size, std::back_inserter(extracted));
            buffer.erase(buffer.begin(), buffer.begin() + batch_size);
            co_return extracted;
        });

        if (result.empty())
            co_return;

        logs_to_send = std::move(result);

        try
        {
            auto compressed_data = this->serialize_and_compress(logs_to_send);

            // TODO: Send compressed data to log server via HTTP POST
            // For now, just a placeholder
            // co_await this->_http.post_raw(_log_server_url, "/log/ingest", compressed_data);
        }
        catch (const std::exception& e)
        {
            fb::logger::warn("Failed to flush logs: {}", e.what());
            // On failure, put logs back to buffer (up to max_buffer_size)
            this->_log_buffer.write([this, &logs_to_send](auto& buffer) {
                auto remaining = this->_max_buffer_size - buffer.size();
                if (remaining > 0)
                {
                    auto to_restore = std::min(remaining, logs_to_send.size());
                    buffer.insert(buffer.begin(), logs_to_send.begin(), logs_to_send.begin() + to_restore);
                }
            });
            co_return; // Stop flushing on error
        }
    }
}

fb::stream log_collector::serialize_and_compress(const std::vector<Json::Value>& logs) const
{
    // Convert Json::Value array to JSON string
    Json::Value json_array(Json::arrayValue);
    for (const auto& log : logs)
    {
        json_array.append(log);
    }

    Json::FastWriter writer;
    std::string      json_string = writer.write(json_array);
    // Remove trailing newline from FastWriter
    if (!json_string.empty() && json_string.back() == '\n')
    {
        json_string.pop_back();
    }

    // Convert UTF-8 string to stream
    fb::stream utf8_stream(reinterpret_cast<const uint8_t*>(json_string.data()), json_string.size());

    // Compress using zlib
    return utf8_stream.compress();
}
