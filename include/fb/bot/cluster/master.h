#ifndef __FB_BOT_CLUSTER_MASTER_H__
#define __FB_BOT_CLUSTER_MASTER_H__

#include <fb/bot/cluster/channel.h>
#include <fb/bot/cluster/stats.h>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <json/json.h>

#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace fb::bot::cluster {

class master
{
public:
    struct options
    {
        std::string listen = "127.0.0.1:8089";
        std::string token;
        std::string web = "web";
    };

private:
    using clock    = std::chrono::steady_clock;
    using request  = boost::beast::http::request<boost::beast::http::string_body>;
    using response = boost::beast::http::response<boost::beast::http::string_body>;

    struct lease
    {
        uint32_t process   = 0;
        uint32_t seat_base = 0;
        uint32_t seats     = 0;
    };

    struct agent_entry
    {
        uint32_t                 id = 0;
        std::string              key;
        std::string              name;
        std::string              host;
        std::string              version;
        uint32_t                 pid      = 0;
        uint32_t                 seats    = 1;
        uint32_t                 max_bots = 0;
        std::shared_ptr<channel> link;
        bool                     busy     = false;
        bool                     excluded = false;
        uint32_t                 users    = 0;
        uint32_t                 bots     = 0;
        std::vector<std::string> batch;
        Json::Value              metrics;
        clock::time_point        seen;
        clock::time_point        lost;
    };

    struct suite_entry
    {
        std::string              name;
        std::vector<std::string> labels;
        bool                     serial     = false;
        bool                     extra_slot = false;
        uint32_t                 bots       = 0;
        std::string              error;
    };

    struct suite_run
    {
        std::string name;
        bool        serial   = false;
        std::string state    = "pending";
        uint32_t    agent    = 0;
        uint32_t    attempts = 0;
        double      seconds  = 0.0;
        std::string log;
    };

    struct run_state
    {
        std::string                         id;
        std::string                         kind;
        std::string                         state = "running";
        std::string                         started_at;
        std::string                         finished_at;
        clock::time_point                   started;
        double                              elapsed = 0.0;
        std::string                         client;
        std::string                         target;
        uint32_t                            retry    = 1;
        uint32_t                            users    = 0;
        uint32_t                            rate     = 1;
        uint32_t                            cap      = 0;
        uint32_t                            duration = 0;
        uint32_t                            offset   = 0;
        std::vector<suite_run>              suites;
        std::map<std::string, stats::entry> totals;
        std::map<std::string, stats::entry> window;
        Json::Value                         timeline{Json::arrayValue};
    };

private:
    boost::asio::io_context&              _context;
    options                               _options;
    std::map<uint32_t, agent_entry>       _agents;
    std::map<std::string, lease>          _leases;
    std::vector<suite_entry>              _catalog;
    Json::Value                           _files{Json::arrayValue};
    std::string                           _loaded_at;
    uint32_t                              _revision = 0;
    std::optional<run_state>              _run;
    std::vector<std::shared_ptr<channel>> _dashboards;
    uint32_t                              _next_agent = 1;

public:
    master(boost::asio::io_context& context, options options);

    boost::asio::awaitable<void> run();

private:
    boost::asio::awaitable<void> serve(boost::asio::ip::tcp::socket socket);
    boost::asio::awaitable<void> serve_agent(std::shared_ptr<channel> link, bool local);
    boost::asio::awaitable<void> serve_dashboard(std::shared_ptr<channel> link);
    boost::asio::awaitable<void> tick();

    response    route(const request& request);
    response    file(const request& request, const std::string& path) const;
    Json::Value snapshot() const;
    Json::Value report(size_t points) const;
    Json::Value suites() const;
    Json::Value targets() const;
    Json::Value history() const;
    Json::Value reload();
    Json::Value start(const Json::Value& body);
    void        stop();
    void        scale(uint32_t users);
    void        dispatch();
    void        assign(agent_entry& agent, const std::vector<std::string>& names);
    void        rebalance();
    void        finish();
    void        handle(agent_entry& agent, const Json::Value& message);
    void        drop(agent_entry& agent);
    void        broadcast();
    void        save_leases() const;
    void        load_leases();

    static Json::Value table(const std::map<std::string, stats::entry>& totals, double elapsed);
    static std::string now(std::string_view format);
};

} // namespace fb::bot::cluster

#endif
