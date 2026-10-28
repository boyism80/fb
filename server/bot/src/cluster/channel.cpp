#include <fb/bot/cluster/channel.h>
#include <fb/bot/cluster/link.h>

using namespace fb::bot::cluster;

channel::channel(stream&& stream) :
    _stream(std::move(stream))
{ }

boost::asio::awaitable<std::optional<Json::Value>> channel::read()
{
    auto self = this->shared_from_this();
    while (this->_closed == false)
    {
        auto buffer = boost::beast::flat_buffer{};
        auto error  = boost::system::error_code{};
        co_await this->_stream.async_read(buffer, boost::asio::redirect_error(boost::asio::use_awaitable, error));
        if (error)
        {
            this->_closed = true;
            break;
        }

        auto text    = boost::beast::buffers_to_string(buffer.data());
        auto message = Json::Value{};
        if (link::decode(text, message))
            co_return message;
    }
    co_return std::nullopt;
}

void channel::send(const Json::Value& message)
{
    if (this->_closed || this->_closing)
        return;

    this->_queue.push_back(link::serialize(message));
    if (this->_writing)
        return;

    this->_writing = true;
    boost::asio::co_spawn(this->_stream.get_executor(), this->flush(), boost::asio::detached);
}

void channel::close()
{
    if (this->_closed || this->_closing)
        return;

    this->_closing = true;
    if (this->_writing)
        return;

    this->_writing = true;
    boost::asio::co_spawn(this->_stream.get_executor(), this->flush(), boost::asio::detached);
}

bool channel::closed() const
{
    return this->_closed;
}

boost::asio::awaitable<void> channel::flush()
{
    auto self = this->shared_from_this();
    while (this->_queue.empty() == false && this->_closed == false)
    {
        auto text  = std::move(this->_queue.front());
        auto error = boost::system::error_code{};
        this->_queue.pop_front();
        this->_stream.text(true);
        co_await this->_stream.async_write(boost::asio::buffer(text),
                                           boost::asio::redirect_error(boost::asio::use_awaitable, error));
        if (error)
            this->_closed = true;
    }

    if (this->_closing && this->_closed == false)
    {
        auto error = boost::system::error_code{};
        co_await this->_stream.async_close(boost::beast::websocket::close_code::normal,
                                           boost::asio::redirect_error(boost::asio::use_awaitable, error));
        this->_closed = true;
    }

    this->_queue.clear();
    this->_writing = false;
}
