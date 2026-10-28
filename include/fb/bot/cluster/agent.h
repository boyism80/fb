#ifndef __FB_BOT_CLUSTER_AGENT_H__
#define __FB_BOT_CLUSTER_AGENT_H__

#include <fb/bot/cluster/channel.h>

#include <boost/asio.hpp>
#include <json/json.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace fb::bot::cluster {

class agent
{
public:
    struct options
    {
        std::string executable;
        std::string master;
        std::string token;
        std::string name;
        std::string config;
        uint32_t    seats    = 1;
        uint32_t    max_bots = 500;
    };

private:
    struct child;

private:
    boost::asio::io_context& _context;
    options                  _options;
    std::shared_ptr<channel> _channel;
    std::shared_ptr<child>   _child;
    uint32_t                 _id      = 0;
    bool                     _running = true;

public:
    agent(boost::asio::io_context& context, options options);
    ~agent();

    boost::asio::awaitable<void> run();

private:
    boost::asio::awaitable<bool> session();
    void                         handle(const Json::Value& message);
    void                         spawn(const std::string& run_id, const std::vector<std::string>& arguments);
    void                         write(const Json::Value& message);
    void                         stop();
    void                         on_output(const std::shared_ptr<child>& source, Json::Value message);
    void                         on_exit(const std::shared_ptr<child>& source);
};

} // namespace fb::bot::cluster

#endif
