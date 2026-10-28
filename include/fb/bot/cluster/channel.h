#ifndef __FB_BOT_CLUSTER_CHANNEL_H__
#define __FB_BOT_CLUSTER_CHANNEL_H__

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <json/json.h>

#include <deque>
#include <memory>
#include <optional>
#include <string>

namespace fb::bot::cluster {

class channel : public std::enable_shared_from_this<channel>
{
public:
    using stream = boost::beast::websocket::stream<boost::beast::tcp_stream>;

private:
    stream                  _stream;
    std::deque<std::string> _queue;
    bool                    _writing = false;
    bool                    _closing = false;
    bool                    _closed  = false;

public:
    explicit channel(stream&& stream);

    boost::asio::awaitable<std::optional<Json::Value>> read();
    void                                               send(const Json::Value& message);
    void                                               close();
    bool                                               closed() const;

private:
    boost::asio::awaitable<void> flush();
};

} // namespace fb::bot::cluster

#endif
