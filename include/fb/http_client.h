#ifndef __FB_HTTP_CLIENT_H__
#define __FB_HTTP_CLIENT_H__

#include <atomic>
#include <chrono>
#include <format>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <string_view>
#include <vector>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <fb/stream.h>
#include <fb/stream_reader.h>
#include <fb/stream_writer.h>
#include <fb/config.h>
#include <fb/async_executor.h>
#include <fb/model/datetime.h>
#include <fb/thread.h>

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
    std::atomic<int64_t>     _response_delay_ms{0};

    [[nodiscard]] async::task<void> sleep(fb::thread* thread)
    {
        if (thread == nullptr)
            co_return;

        co_await thread->switching();

        auto delay_ms = this->_response_delay_ms.load(std::memory_order_relaxed);
        if (delay_ms <= 0)
            co_return;

        co_await thread->sleep(fb::model::timespan(std::chrono::milliseconds(delay_ms)));
    }

    void pump()
    {
        auto& io_context = static_cast<boost::asio::io_context&>(this->_executor);
        boost::asio::post(io_context, [this] {
            while (true)
            {
                pending_task task;
                {
                    std::lock_guard lock(this->_queue_mutex);
                    if (this->_queue.empty() ||
                        this->_in_flight.load(std::memory_order_relaxed) >= this->_max_concurrent)
                        break;

                    task = std::move(this->_queue.front());
                    this->_queue.pop();
                    this->_in_flight.fetch_add(1, std::memory_order_relaxed);
                }
                task();
            }
        });
    }

    struct http_endpoint
    {
        std::string host;
        std::string port;
    };

    static http_endpoint parse_http_endpoint(std::string host_url)
    {
        if (host_url.rfind("http://", 0) == 0)
            host_url.erase(0, 7);
        else if (host_url.rfind("https://", 0) == 0)
            host_url.erase(0, 8);

        const auto colon_pos = host_url.find(':');
        if (colon_pos == std::string::npos)
            return {.host = std::move(host_url), .port = "80"};

        return {.host = host_url.substr(0, colon_pos), .port = host_url.substr(colon_pos + 1)};
    }

    class http_request_deadline
    {
    public:
        http_request_deadline(boost::asio::any_io_executor        executor,
                              boost::asio::ip::tcp::socket&       socket,
                              std::chrono::steady_clock::duration timeout) :
            _socket(socket),
            _timer(std::move(executor))
        {
            _timer.expires_after(timeout);
            _timer.async_wait([this](const boost::system::error_code& ec) {
                if (ec || _released.load(std::memory_order_acquire))
                    return;

                boost::system::error_code cancel_ec;
                _socket.cancel(cancel_ec);
            });
        }

        void release()
        {
            if (_released.load(std::memory_order_acquire))
                return;

            _released.store(true, std::memory_order_release);
            boost::system::error_code ec;
            _timer.cancel(ec);
        }

        ~http_request_deadline()
        {
            release();
        }

    private:
        boost::asio::ip::tcp::socket& _socket;
        boost::asio::steady_timer     _timer;
        std::atomic<bool>             _released{false};
    };

    static void close_http_socket(boost::asio::ip::tcp::socket& socket)
    {
        boost::system::error_code ec;
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket.close(ec);
    }

    static std::vector<uint8_t>
    response_body_bytes(const boost::beast::http::response<boost::beast::http::dynamic_body>& res)
    {
        auto body_bytes = std::vector<uint8_t>{};
        if (res.body().size() > 0)
            body_bytes.reserve(res.body().size());

        for (const auto& seq : res.body().data())
        {
            const auto buf      = seq;
            const auto data_ptr = static_cast<const uint8_t*>(buf.data());
            body_bytes.insert(body_bytes.end(), data_ptr, data_ptr + buf.size());
        }

        return body_bytes;
    }

    template <typename Request>
    boost::asio::awaitable<std::vector<uint8_t>> exchange_http_async(boost::asio::io_context&            io_context,
                                                                     const http_endpoint&                endpoint,
                                                                     std::chrono::steady_clock::duration timeout,
                                                                     Request                             request)
    {
        auto resolver = boost::asio::ip::tcp::resolver{io_context};
        auto socket   = boost::asio::ip::tcp::socket{io_context};

        http_request_deadline deadline{io_context.get_executor(), socket, timeout};

        const auto results = co_await resolver.async_resolve(endpoint.host, endpoint.port, boost::asio::use_awaitable);
        co_await boost::asio::async_connect(socket, results, boost::asio::use_awaitable);

        co_await boost::beast::http::async_write(socket, request, boost::asio::use_awaitable);

        auto buffer = boost::beast::flat_buffer{};
        auto res    = boost::beast::http::response<boost::beast::http::dynamic_body>{};
        co_await boost::beast::http::async_read(socket, buffer, res, boost::asio::use_awaitable);

        deadline.release();
        close_http_socket(socket);

        co_return response_body_bytes(res);
    }

    static std::runtime_error wrap_http_error(std::string_view operation, const std::exception& e)
    {
        return std::runtime_error(std::format("HTTP {} request failed: {}", operation, e.what()));
    }

    static std::pair<std::string, std::string> split_absolute_url(std::string_view full_url)
    {
        auto url        = std::string(full_url);
        auto scheme_end = url.find("://");
        if (scheme_end == std::string::npos)
            throw std::runtime_error(std::format("Invalid URL: {}", full_url));

        auto path_start = url.find('/', scheme_end + 3);
        if (path_start == std::string::npos)
            return {url, "/"};

        return {url.substr(0, path_start), url.substr(path_start)};
    }

    boost::asio::awaitable<std::vector<uint8_t>> boost_get_raw_async(std::string                         host,
                                                                     std::string                         path,
                                                                     std::map<std::string, std::string>  headers,
                                                                     std::chrono::steady_clock::duration timeout,
                                                                     bool require_success = false)
    {
        try
        {
            auto& io_context = static_cast<boost::asio::io_context&>(this->_executor);
            auto  endpoint   = parse_http_endpoint(std::move(host));

            auto req =
                boost::beast::http::request<boost::beast::http::empty_body>{boost::beast::http::verb::get,
                                                                            url_encode(UTF8(path, PLATFORM::WINDOWS)),
                                                                            11};
            req.set(boost::beast::http::field::host, endpoint.host);
            req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            for (const auto& [name, value] : headers)
                req.set(name, value);

            auto resolver = boost::asio::ip::tcp::resolver{io_context};
            auto socket   = boost::asio::ip::tcp::socket{io_context};

            http_request_deadline deadline{io_context.get_executor(), socket, timeout};

            const auto results =
                co_await resolver.async_resolve(endpoint.host, endpoint.port, boost::asio::use_awaitable);
            co_await boost::asio::async_connect(socket, results, boost::asio::use_awaitable);
            co_await boost::beast::http::async_write(socket, req, boost::asio::use_awaitable);

            auto buffer = boost::beast::flat_buffer{};
            auto res    = boost::beast::http::response<boost::beast::http::dynamic_body>{};
            co_await boost::beast::http::async_read(socket, buffer, res, boost::asio::use_awaitable);

            deadline.release();
            close_http_socket(socket);

            if (require_success)
            {
                auto status = res.result_int();
                if (status < 200 || status >= 300)
                {
                    throw std::runtime_error(std::format("HTTP GET failed with status {}: {}", status, path));
                }
            }

            co_return response_body_bytes(res);
        }
        catch (const std::exception& e)
        {
            throw wrap_http_error("GET", e);
        }
        catch (...)
        {
            throw std::runtime_error("HTTP GET request failed: Unknown error occurred");
        }
    }

    boost::asio::awaitable<std::vector<uint8_t>> boost_post_raw_async(std::string                         host,
                                                                      std::string                         path,
                                                                      std::map<std::string, std::string>  headers,
                                                                      std::chrono::steady_clock::duration timeout,
                                                                      std::vector<uint8_t>                body)
    {
        try
        {
            auto& io_context = static_cast<boost::asio::io_context&>(this->_executor);
            auto  endpoint   = parse_http_endpoint(std::move(host));

            auto req = boost::beast::http::request<boost::beast::http::vector_body<uint8_t>>{
                boost::beast::http::verb::post,
                url_encode(UTF8(path, PLATFORM::WINDOWS)),
                11};

            req.set(boost::beast::http::field::host, endpoint.host);
            req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            for (const auto& [name, value] : headers)
                req.set(name, value);

            req.body() = std::move(body);
            req.prepare_payload();

            co_return co_await exchange_http_async(io_context, endpoint, timeout, std::move(req));
        }
        catch (const std::exception& e)
        {
            throw wrap_http_error("POST", e);
        }
        catch (...)
        {
            throw std::runtime_error("HTTP POST request failed: Unknown error occurred");
        }
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

    void response_delay(const fb::model::timespan& value)
    {
        this->_response_delay_ms.store(value.total_milliseconds(), std::memory_order_relaxed);
    }

    fb::model::timespan response_delay() const
    {
        return fb::model::timespan(std::chrono::milliseconds(this->_response_delay_ms.load(std::memory_order_relaxed)));
    }

    template <typename T> async::task<T> get(std::string_view service, std::string_view path)
    {
        auto  service_str = std::string(service);
        auto& config      = fb::config<>(service_str);
        auto  host        = std::format("http://{}:{}", config["ip"].asCString(), config["port"].asUInt());
        auto  path_str    = std::string(path);
        auto  thread      = this->_executor.threads.current();
        auto  result      = co_await this->boost_get_async<T>(host, path_str);
        co_await this->sleep(thread);
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
            boost::asio::co_spawn(ctx,
                                  this->boost_get_raw_async(host_str, path_str, headers, 5s),
                                  [this, promise](std::exception_ptr ep, std::vector<uint8_t> bytes) {
                                      if (ep)
                                      {
                                          promise->set_exception(ep);
                                          this->_in_flight.fetch_sub(1, std::memory_order_relaxed);
                                          this->pump();
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
                                      this->pump();
                                  });
        };

        bool trigger;
        {
            std::lock_guard lock(this->_queue_mutex);
            trigger = this->_queue.empty();
            this->_queue.push(std::move(task));
        }
        if (trigger)
            this->pump();

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
        co_await this->sleep(thread);
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
        co_await this->sleep(thread);
    }

    /// <summary>
    /// Downloads raw bytes from an absolute HTTP URL (e.g. http://host:port/path/file.json).
    /// Throws if the response status is not 2xx.
    /// </summary>
    async::task<std::vector<uint8_t>> get_bytes(std::string_view full_url)
    {
        auto [host, path] = split_absolute_url(full_url);
        auto promise      = std::make_shared<async::task_completion_source<std::vector<uint8_t>>>();
        auto headers      = std::map<std::string, std::string>{};

        pending_task task = [this, promise, host = std::move(host), path = std::move(path), headers]() {
            auto& ctx = static_cast<boost::asio::io_context&>(this->_executor);
            boost::asio::co_spawn(ctx,
                                  this->boost_get_raw_async(host, path, headers, std::chrono::seconds{30}, true),
                                  [this, promise](std::exception_ptr ep, std::vector<uint8_t> bytes) {
                                      if (ep)
                                      {
                                          promise->set_exception(ep);
                                          this->_in_flight.fetch_sub(1, std::memory_order_relaxed);
                                          this->pump();
                                          return;
                                      }
                                      promise->set_value(std::move(bytes));
                                      this->_in_flight.fetch_sub(1, std::memory_order_relaxed);
                                      this->pump();
                                  });
        };

        bool trigger;
        {
            std::lock_guard lock(this->_queue_mutex);
            trigger = this->_queue.empty();
            this->_queue.push(std::move(task));
        }
        if (trigger)
            this->pump();

        return promise->task();
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
            boost::asio::co_spawn(ctx,
                                  this->boost_post_raw_async(host, path, headers, std::chrono::seconds{5}, body_vec),
                                  [this, promise](std::exception_ptr ep, std::vector<uint8_t> bytes) {
                                      if (ep)
                                      {
                                          promise->set_exception(ep);
                                          this->_in_flight.fetch_sub(1, std::memory_order_relaxed);
                                          this->pump();
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
                                      this->pump();
                                  });
        };

        bool trigger;
        {
            std::lock_guard lock(this->_queue_mutex);
            trigger = this->_queue.empty();
            this->_queue.push(std::move(task));
        }
        if (trigger)
            this->pump();

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
                        this->pump();
                        return;
                    }
                    promise->set_value();
                    this->_in_flight.fetch_sub(1, std::memory_order_relaxed);
                    this->pump();
                });
        };

        bool trigger;
        {
            std::lock_guard lock(this->_queue_mutex);
            trigger = this->_queue.empty();
            this->_queue.push(std::move(task));
        }
        if (trigger)
            this->pump();

        return promise->task();
    }
};

} // namespace fb

#endif // !__FB_HTTP_CLIENT_H__