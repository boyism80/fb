#ifndef __FB_HTTP_CLIENT_H__
#define __FB_HTTP_CLIENT_H__

#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <string_view>
#include <vector>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <fb/stream.h>
#include <fb/stream_reader.h>
#include <fb/stream_writer.h>
#include <fb/config.h>
#include <fb/async_executor.h>

#define REGISTER_RESPONSE(Request, Response) \
    template <> struct response_of<Request>  \
    {                                        \
        using type = Response;               \
    };

template <typename Request>
struct response_of;

namespace fb {

using namespace std::chrono_literals;

class http_client
{
private:
    using pending_task = std::function<void()>;

    fb::async_executor&      _executor;
    size_t                   _max_concurrent;
    std::mutex               _queue_mutex;
    std::queue<pending_task> _queue;
    std::atomic<size_t>      _in_flight{0};

    void process_pending()
    {
        pending_task task;
        {
            std::lock_guard lock(this->_queue_mutex);
            if (this->_queue.empty() || this->_in_flight.load(std::memory_order_relaxed) >= this->_max_concurrent)
                return;
            task = std::move(this->_queue.front());
            this->_queue.pop();
            this->_in_flight.fetch_add(1, std::memory_order_relaxed);
        }
        task();
        this->post_process_pending();
    }

    void post_process_pending()
    {
        auto& io_context = static_cast<boost::asio::io_context&>(this->_executor);
        boost::asio::post(io_context, [this] {
            this->process_pending();
        });
    }

public:
    /**
     * Constructs the HTTP client with an external concurrency limit.
     * @param executor The async executor (e.g. acceptor).
     * @param max_concurrent Maximum number of in-flight HTTP requests; must be greater than 0.
     */
    explicit http_client(fb::async_executor& executor, size_t max_concurrent) :
        _executor(executor),
        _max_concurrent(max_concurrent)
    { }

    http_client(const http_client&)             = delete;
    http_client& operator= (const http_client&) = delete;
    ~http_client()                              = default;

private:
    boost::asio::awaitable<std::vector<uint8_t>> boost_get_raw_async(std::string                         host,
                                                                    std::string                         path,
                                                                    std::map<std::string, std::string>  headers,
                                                                    std::chrono::steady_clock::duration timeout)
    {
        try
        {
            auto raw_host = host;
            if (raw_host.rfind("http://", 0) == 0)
                raw_host.erase(0, 7);
            else if (raw_host.rfind("https://", 0) == 0)
                raw_host.erase(0, 8);

            auto const colon_pos = raw_host.find(':');
            auto const host_name = (colon_pos == std::string::npos ? raw_host : raw_host.substr(0, colon_pos));
            auto const port = (colon_pos == std::string::npos ? std::string("80") : raw_host.substr(colon_pos + 1));

            auto& io_context = static_cast<boost::asio::io_context&>(this->_executor);
            auto  resolver   = boost::asio::ip::tcp::resolver{io_context};
            auto  stream     = boost::beast::tcp_stream{io_context};

            stream.expires_after(timeout);
            auto const results = co_await resolver.async_resolve(host_name, port, boost::asio::use_awaitable);
            co_await stream.async_connect(results, boost::asio::use_awaitable);

            auto req =
                boost::beast::http::request<boost::beast::http::empty_body>{boost::beast::http::verb::get,
                                                                            url_encode(UTF8(path, PLATFORM::WINDOWS)),
                                                                            11};
            req.set(boost::beast::http::field::host, host_name);
            req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            for (auto const& h : headers)
            {
                req.set(h.first, h.second);
            }

            stream.expires_after(timeout);
            co_await boost::beast::http::async_write(stream, req, boost::asio::use_awaitable);

            auto buffer = boost::beast::flat_buffer{};
            auto res    = boost::beast::http::response<boost::beast::http::dynamic_body>{};
            stream.expires_after(timeout);
            co_await boost::beast::http::async_read(stream, buffer, res, boost::asio::use_awaitable);

            auto body_bytes = std::vector<uint8_t>{};
            if (res.body().size() > 0)
            {
                body_bytes.reserve(res.body().size());
            }
            for (auto const& seq : res.body().data())
            {
                auto buf      = seq; // boost::asio::const_buffer
                auto data_ptr = static_cast<const uint8_t*>(buf.data());
                body_bytes.insert(body_bytes.end(), data_ptr, data_ptr + buf.size());
            }

            auto ec = boost::beast::error_code{};
            stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

            co_return body_bytes;
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(std::format("HTTP GET request failed: {}", e.what()));
        }
        catch (...)
        {
            throw std::runtime_error("HTTP GET request failed: Unknown error occurred");
        }
    }

private:
    boost::asio::awaitable<std::vector<uint8_t>> boost_post_raw_async(std::string                         host,
                                                                      std::string                         path,
                                                                      std::map<std::string, std::string>  headers,
                                                                      std::chrono::steady_clock::duration timeout,
                                                                      std::vector<uint8_t>                body)
    {
        try
        {
            auto raw_host = host;
            if (raw_host.rfind("http://", 0) == 0)
                raw_host.erase(0, 7);
            else if (raw_host.rfind("https://", 0) == 0)
                raw_host.erase(0, 8);

            auto const colon_pos = raw_host.find(':');
            auto const host_name = (colon_pos == std::string::npos ? raw_host : raw_host.substr(0, colon_pos));
            auto const port = (colon_pos == std::string::npos ? std::string("80") : raw_host.substr(colon_pos + 1));

            auto& io_context = static_cast<boost::asio::io_context&>(this->_executor);
            auto  resolver   = boost::asio::ip::tcp::resolver{io_context};
            auto  stream     = boost::beast::tcp_stream{io_context};

            stream.expires_after(timeout);
            auto const results = co_await resolver.async_resolve(host_name, port, boost::asio::use_awaitable);
            co_await stream.async_connect(results, boost::asio::use_awaitable);

            auto req = boost::beast::http::request<boost::beast::http::vector_body<uint8_t>>{
                boost::beast::http::verb::post,
                url_encode(UTF8(path, PLATFORM::WINDOWS)),
                11};

            req.set(boost::beast::http::field::host, host_name);
            req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

            for (auto const& h : headers)
            {
                req.set(h.first, h.second);
            }

            req.body() = body;
            req.prepare_payload();

            stream.expires_after(timeout);
            co_await boost::beast::http::async_write(stream, req, boost::asio::use_awaitable);

            auto buffer = boost::beast::flat_buffer{};
            auto res    = boost::beast::http::response<boost::beast::http::dynamic_body>{};
            stream.expires_after(timeout);
            co_await boost::beast::http::async_read(stream, buffer, res, boost::asio::use_awaitable);

            auto body_bytes = std::vector<uint8_t>{};
            if (res.body().size() > 0)
            {
                body_bytes.reserve(res.body().size());
            }
            for (auto const& seq : res.body().data())
            {
                auto const buf      = seq;
                auto const data_ptr = static_cast<const uint8_t*>(buf.data());
                body_bytes.insert(body_bytes.end(), data_ptr, data_ptr + buf.size());
            }

            auto ec = boost::beast::error_code{};
            stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

            co_return body_bytes;
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(std::format("HTTP request failed: {}", e.what()));
        }
        catch (...)
        {
            throw std::runtime_error("HTTP request failed: Unknown error occurred");
        }
    }

public:
    template <typename T> async::task<T> get(std::string_view service, std::string_view path)
    {
        auto  service_str = std::string(service);
        auto& config      = fb::config<>(service_str);
        auto  host        = std::format("http://{}:{}", config["ip"].asCString(), config["port"].asUInt());
        auto  path_str    = std::string(path);
        auto  thread      = this->_executor.threads.current();
        auto  result      = co_await this->boost_get_async<T>(host, path_str);
        if (thread != nullptr)
            co_await thread->switching();
        co_return result;
    }

private:
    template <typename Response> [[nodiscard]] async::task<Response> boost_get_async(std::string_view host,
                                                                                     std::string_view path)
    {
        auto host_str = std::string(host);
        auto path_str = std::string(path);
        auto promise  = std::make_shared<async::task_completion_source<Response>>();
        auto headers  = std::map<std::string, std::string>{
            {"Content-Type", "application/octet-stream"},
        };

        pending_task task = [this, promise, host_str, path_str, headers]() {
            auto& ctx = static_cast<boost::asio::io_context&>(this->_executor);
            boost::asio::co_spawn(
                ctx,
                this->boost_get_raw_async(host_str, path_str, headers, 5s),
                [this, promise](std::exception_ptr ep, std::vector<uint8_t> bytes) {
                    if (ep)
                    {
                        promise->set_exception(ep);
                        this->_in_flight.fetch_sub(1, std::memory_order_relaxed);
                        this->post_process_pending();
                        return;
                    }
                    try
                    {
                        auto reader        = fb::stream_reader<big_endian>(bytes);
                        auto protocol_type = reader.read<uint32_t>();
                        auto protocol_size = reader.read<uint32_t>();
                        auto offset        = bytes.data() + sizeof(uint32_t) + sizeof(uint32_t);
                        promise->set_value(Response::Deserialize(offset));
                    }
                    catch (std::exception& e)
                    {
                        promise->set_exception(std::make_exception_ptr(e));
                    }
                    this->_in_flight.fetch_sub(1, std::memory_order_relaxed);
                    this->post_process_pending();
                });
        };

        bool trigger;
        {
            std::lock_guard lock(this->_queue_mutex);
            trigger = this->_queue.empty();
            this->_queue.push(std::move(task));
        }
        if (trigger)
            this->post_process_pending();

        return promise->task();
    }

public:
    template <typename Request> [[nodiscard]] async::task<typename response_of<Request>::type>
    post(std::string_view service, std::string_view path, const Request& request)
    {
        auto  service_str = std::string(service);
        auto& config      = fb::config<>(service_str);
        auto  host        = std::format("http://{}:{}", config["ip"].asCString(), config["port"].asUInt());
        auto  path_str    = std::string(path);
        auto  thread      = this->_executor.threads.current();
        auto  result      = co_await this->boost_post_async<Request>(host, path_str, request);
        if (thread != nullptr)
            co_await thread->switching();
        co_return result;
    }

    /// <summary>
    /// Sends a POST request with binary data (fb::stream) to the specified URL.
    /// </summary>
    /// <param name="url">The full URL (e.g., "http://host:port").</param>
    /// <param name="path">The path component of the URL.</param>
    /// <param name="data">The binary data to send as request body.</param>
    /// <returns>A task representing the asynchronous operation.</returns>
    async::task<void> post(std::string_view url, std::string_view path, const fb::stream& data)
    {
        auto url_str  = std::string(url);
        auto path_str = std::string(path);
        auto thread   = this->_executor.threads.current();
        co_await this->boost_post_binary_async(url_str, path_str, data);
        if (thread != nullptr)
            co_await thread->switching();
    }

private:
    template <typename Request> [[nodiscard]] async::task<typename response_of<Request>::type>
    boost_post_async(std::string const& host, std::string const& path, Request const& body)
    {
        auto const serialized_payload = body.Serialize();
        auto       stream_req         = fb::stream();
        auto       writer             = fb::stream_writer<>(stream_req);

        writer.write<uint32_t>(static_cast<uint32_t>(Request::FlatBufferProtocolType));
        writer.write<uint32_t>(serialized_payload.size());
        writer.write(serialized_payload.data(), serialized_payload.size());

        auto promise = std::make_shared<async::task_completion_source<typename response_of<Request>::type>>();
        auto headers = std::map<std::string, std::string>{
            {"Content-Type", "application/octet-stream"}
        };

        auto body_vec = std::vector<uint8_t>(stream_req.begin(), stream_req.end());

        pending_task task = [this, promise, host, path, headers, body_vec]() {
            auto& ctx = static_cast<boost::asio::io_context&>(this->_executor);
            boost::asio::co_spawn(
                ctx,
                this->boost_post_raw_async(host, path, headers, std::chrono::seconds{5}, body_vec),
                [this, promise](std::exception_ptr ep, std::vector<uint8_t> bytes) {
                    if (ep)
                    {
                        promise->set_exception(ep);
                        this->_in_flight.fetch_sub(1, std::memory_order_relaxed);
                        this->post_process_pending();
                        return;
                    }
                    try
                    {
                        auto reader        = fb::stream_reader<big_endian>(bytes);
                        auto protocol_type = reader.read<uint32_t>();
                        auto protocol_len  = reader.read<uint32_t>();
                        auto offset        = bytes.data() + sizeof(uint32_t) * 2;
                        promise->set_value(response_of<Request>::type::Deserialize(offset));
                    }
                    catch (...)
                    {
                        promise->set_exception(std::current_exception());
                    }
                    this->_in_flight.fetch_sub(1, std::memory_order_relaxed);
                    this->post_process_pending();
                });
        };

        bool trigger;
        {
            std::lock_guard lock(this->_queue_mutex);
            trigger = this->_queue.empty();
            this->_queue.push(std::move(task));
        }
        if (trigger)
            this->post_process_pending();

        return promise->task();
    }

private:
    [[nodiscard]] async::task<void> boost_post_binary_async(std::string_view  url,
                                                            std::string_view  path,
                                                            const fb::stream& data)
    {
        auto url_str  = std::string(url);
        auto path_str = std::string(path);
        auto promise  = std::make_shared<async::task_completion_source<void>>();
        auto headers  = std::map<std::string, std::string>{
            {"Content-Type",     "application/octet-stream"},
            {"Content-Encoding", "deflate"                 }
        };

        auto body = std::vector<uint8_t>(data.begin(), data.end());

        pending_task task = [this, promise, url_str, path_str, headers, body]() {
            auto& ctx = static_cast<boost::asio::io_context&>(this->_executor);
            boost::asio::co_spawn(
                ctx,
                this->boost_post_raw_async(url_str, path_str, headers, std::chrono::seconds{30}, body),
                [this, promise](std::exception_ptr ep, std::vector<uint8_t> /*bytes*/) {
                    if (ep)
                    {
                        promise->set_exception(ep);
                        this->_in_flight.fetch_sub(1, std::memory_order_relaxed);
                        this->post_process_pending();
                        return;
                    }
                    promise->set_value();
                    this->_in_flight.fetch_sub(1, std::memory_order_relaxed);
                    this->post_process_pending();
                });
        };

        bool trigger;
        {
            std::lock_guard lock(this->_queue_mutex);
            trigger = this->_queue.empty();
            this->_queue.push(std::move(task));
        }
        if (trigger)
            this->post_process_pending();

        return promise->task();
    }
};

} // namespace fb

#endif // !__FB_HTTP_CLIENT_H__