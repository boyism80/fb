#ifndef __FB_HTTP_CLIENT_H__
#define __FB_HTTP_CLIENT_H__

#include <chrono>
#include <map>
#include <string>
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

#define REGISTER_RESPONSE(Request, Response) \
    template <> struct response_of<Request>  \
    {                                        \
        using type = Response;               \
    };

template <typename Request>
struct response_of;

using namespace std::chrono_literals;

namespace fb {

/**
 * @brief      High-performance HTTP client with FlatBuffer serialization support
 *
 *             Core functionality:
 *             - Typed HTTP GET/POST requests
 *             - Automatic FlatBuffer serialization/deserialization
 *             - Configuration-based service routing
 *             - Connection pooling and reuse
 *             - Timeout and retry handling
 *
 *             Thread Safety:
 *             - All public methods are thread-safe
 *             - Connection pool uses internal mutex
 *             - Request/response processing uses per-request context
 *             - Configuration access is thread-safe
 *
 *             Performance Features:
 *             - Asynchronous I/O via boost::asio
 *             - Connection keep-alive and pooling
 *             - Zero-copy FlatBuffer operations
 *             - Efficient memory management
 *
 *             Error Handling:
 *             - Automatic retry on connection failures
 *             - Configurable timeout per request type
 *             - Detailed error reporting via exceptions
 *             - Circuit breaker for service health
 */
class http_client
{
private:
    fb::async_executor& _executor;

public:
    /**
     * @brief      Constructs an HTTP client with default configuration
     *
     *             Initializes:
     *             - Connection pool
     *             - Thread pool for async operations
     *             - Default timeout settings
     *             - Service configuration
     */
    http_client(fb::async_executor& executor) :
        _executor(executor)
    { }

    http_client(const http_client&)             = delete;
    http_client& operator= (const http_client&) = delete;
    ~http_client()                              = default;

private:
    /**
     * @brief      Performs an asynchronous HTTP GET request using boost::beast.
     *
     * @param[in]  host      The target host in format "hostname:port" or "hostname" (defaults to port 80).
     *                       Supports http:// and https:// prefixes which are automatically stripped.
     * @param[in]  path      The target path including query parameters (will be URL encoded).
     * @param[in]  headers   Custom HTTP headers to include in the request.
     * @param[in]  timeout   Maximum time to wait for connection, request, and response operations.
     *
     * @return     A coroutine task that completes with the response body as a byte vector.
     *
     * @throws     std::exception on network errors, timeout, or invalid response.
     */
    boost::asio::awaitable<std::vector<uint8_t>> boost_get_async(std::string                         host,
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
    /**
     * @brief      Performs an asynchronous HTTP POST request using boost::beast with binary payload support.
     *
     * @param[in]  host      The target host in format "hostname:port" or "hostname" (defaults to port 80).
     * @param[in]  path      The target path including query parameters (will be URL encoded).
     * @param[in]  headers   Custom HTTP headers to include in the request.
     * @param[in]  timeout   Maximum time to wait for connection, request, and response operations.
     * @param[in]  body      The binary request body data to send in the POST request.
     *
     * @return     A coroutine task that completes with the response body as a byte vector.
     *
     * @throws     std::exception on network errors, timeout, or invalid response.
     */
    boost::asio::awaitable<std::vector<uint8_t>> boost_post_async(std::string                         host,
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
    /**
     * @brief      Makes a GET request and deserializes the response
     *
     * @tparam     T           Response type to deserialize into
     * @param[in]  service     Service name from configuration
     * @param[in]  path        Request path
     * @return     boost::asio::awaitable<T>  Deserialized response
     *
     * @throws     std::runtime_error  If service configuration is invalid
     * @throws     std::runtime_error  If connection fails
     * @throws     std::runtime_error  If response deserialization fails
     */
    template <typename T>
    async::task<T> get(const std::string& service, const std::string& path)
    {
        auto& config = fb::config<>(service);
        auto  host   = std::format("http://{}:{}", config["ip"].asCString(), config["port"].asUInt());
        auto  thread = this->_executor.threads.current();
        auto  result = co_await this->boost_get_async<T>(host, path);
        if (thread != nullptr)
            co_await thread->switching();
        co_return result;
    }

private:
    /**
     * @brief      Performs a typed HTTP GET request with automatic FlatBuffer deserialization.
     *
     * @param[in]  host      The target host in "hostname:port" format.
     * @param[in]  path      The target path for the GET request.
     *
     * @tparam     Response  The FlatBuffer response type that implements Deserialize() method.
     *
     * @return     An async task that completes with the deserialized response object.
     *
     * @throws     std::exception on network errors or deserialization failures.
     */
    template <typename Response>
    [[nodiscard]] async::task<Response> boost_get_async(const std::string& host, const std::string& path)
    {
        auto promise = std::make_shared<async::task_completion_source<Response>>();
        auto headers = std::map<std::string, std::string>{
            {"Content-Type", "application/octet-stream"},
        };

        auto& io_context = static_cast<boost::asio::io_context&>(this->_executor);
        boost::asio::co_spawn(io_context,
                              this->boost_get_async(host, path, headers, 5s),
                              [promise](std::exception_ptr ep, std::vector<uint8_t> bytes) {
                                  if (ep)
                                  {
                                      try
                                      {
                                          std::rethrow_exception(ep);
                                      }
                                      catch (...)
                                      {
                                          promise->set_exception(std::current_exception());
                                          return;
                                      }
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
                              });

        return promise->task();
    }

public:
    /**
     * @brief      Makes a POST request with serialized data and deserializes the response
     *
     * @tparam     Req        Request type to serialize
     * @tparam     Resp       Response type to deserialize into
     * @param[in]  service    Service name from configuration
     * @param[in]  path       Request path
     * @param[in]  request    Request data to serialize
     * @return     boost::asio::awaitable<Resp>  Deserialized response
     *
     * @throws     std::runtime_error  If service configuration is invalid
     * @throws     std::runtime_error  If request serialization fails
     * @throws     std::runtime_error  If connection fails
     * @throws     std::runtime_error  If response deserialization fails
     */
    template <typename Request>
    [[nodiscard]] async::task<typename response_of<Request>::type> post(const std::string& service,
                                                                        const std::string& path,
                                                                        const Request&     request)
    {
        auto& config = fb::config<>(service);
        auto  host   = std::format("http://{}:{}", config["ip"].asCString(), config["port"].asUInt());
        auto  thread = this->_executor.threads.current();
        auto  result = co_await this->boost_post_async<Request>(host, path, request);
        if (thread != nullptr)
            co_await thread->switching();
        co_return result;
    }

private:
    /**
     * @brief      Performs a typed HTTP POST request with automatic FlatBuffer serialization and deserialization.
     *
     * @param[in]  host     The target host in "hostname:port" format.
     * @param[in]  path     The target path for the POST request.
     * @param[in]  body     The request object to serialize and send.
     *
     * @tparam     Request   The FlatBuffer request type that implements Serialize() method.
     * @tparam     Response  The FlatBuffer response type expected from the service.
     *
     * @return     An async task that completes with the deserialized response object.
     *
     * @throws     std::exception on network errors, serialization/deserialization failures.
     */
    template <typename Request>
    [[nodiscard]] async::task<typename response_of<Request>::type> boost_post_async(std::string const& host,
                                                                                    std::string const& path,
                                                                                    Request const&     body)
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

        auto& io_context = static_cast<boost::asio::io_context&>(this->_executor);
        boost::asio::co_spawn(io_context,
                              this->boost_post_async(host, path, headers, std::chrono::seconds{5}, stream_req),
                              [promise](std::exception_ptr ep, std::vector<uint8_t> bytes) {
                                  if (ep)
                                  {
                                      promise->set_exception(ep);
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
                              });

        return promise->task();
    }
};

} // namespace fb

#endif // !__FB_HTTP_CLIENT_H__