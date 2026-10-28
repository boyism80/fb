#include <fb/bot/bot_controller_factory.h>
#include <fb/bot/bot_test_runner.h>
#include <fb/bot/client_profile.h>
#include <fb/bot/cluster/agent.h>
#include <fb/bot/cluster/master.h>
#include <fb/bot/integration/lua_integration_test.h>
#include <fb/bot/test_mode.h>
#include <fb/config.h>
#include <fb/console.h>
#include <fb/encoding.h>
#include <fb/model/model.h>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <vector>

using namespace std;
namespace po = boost::program_options;

int main(int argc, char** argv)
{
#ifdef _WIN32
    fb::model::option::decoding(fb::cp949);
#endif

    // Parse command line options
    po::options_description desc("Bot Test Options");
    desc.add_options()("help,h", "Show help message")(
        "mode,m",
        po::value<string>()->default_value("load"),
        "Test mode: 'load' for load testing, 'integration' for integration testing")(
        "config,c",
        po::value<string>()->default_value("config/config.dev.json"),
        "Configuration file path")(
        "test,t",
        po::value<string>()->default_value(""),
        "Integration mode: comma-separated test names to run (e.g. skill,matchmaking), regardless of label")(
        "label,l",
        po::value<string>()->default_value(""),
        "Integration mode: run the tests registered with this label (regular, script, explorer). Empty runs regular")(
        "client",
        po::value<string>()->default_value(""),
        "Client profile for bots without one (550, 565, 651, 651:new). Empty uses client:version in the config")(
        "set",
        po::value<vector<string>>()->composing(),
        "Override a config value (key:path=value), repeatable")(
        "cluster",
        po::bool_switch()->default_value(false),
        "Report to a cluster agent through stdout and read commands from stdin")(
        "role",
        po::value<string>()->default_value(""),
        "Cluster role: 'master' serves the dashboard, 'agent' runs tests for a master. Empty runs --mode standalone")(
        "listen",
        po::value<string>()->default_value("127.0.0.1:8089"),
        "Master: dashboard and agent address")("web",
                                               po::value<string>()->default_value("web"),
                                               "Master: dashboard directory")(
        "token",
        po::value<string>()->default_value(""),
        "Master and agent: shared token. Without it the master accepts local agents only")(
        "master",
        po::value<string>()->default_value("ws://127.0.0.1:8089"),
        "Agent: master address")(
        "name",
        po::value<string>()->default_value(""),
        "Agent: display name, also the key that keeps its account range. Empty uses the host name")(
        "seats",
        po::value<uint32_t>()->default_value(0),
        "Agent: integration seats. 0 uses thread:logic in the config")("max-bots",
                                                                       po::value<uint32_t>()->default_value(500),
                                                                       "Agent: load test bot capacity");

    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    }
    catch (const po::error& e)
    {
        fb::console::puts("Error parsing command line: {}", e.what());

        auto sstream = std::stringstream();
        sstream << desc;
        fb::console::puts("{}", sstream.str());
        return -1;
    }

    if (vm.count("help"))
    {
        auto sstream = std::stringstream();
        sstream << desc;
        fb::console::puts("{}", sstream.str());
        return 0;
    }

    auto& config_path = vm["config"].as<string>();
    auto& mode_str    = vm["mode"].as<string>();
    auto& client_str  = vm["client"].as<string>();
    auto& role        = vm["role"].as<string>();
    auto  overrides   = vm.count("set") ? vm["set"].as<vector<string>>() : vector<string>{};
    auto  cluster     = vm["cluster"].as<bool>();
    if (client_str.empty() == false)
    {
        auto profile = fb::bot::client_profile::parse(client_str);
        if (profile.has_value() == false)
        {
            fb::console::puts("Invalid client profile: {}", client_str);
            return -1;
        }

        fb::bot::client_profile::configure(profile.value());
    }

    if (role == "master" || role == "agent")
    {
        if (fb::init_config(config_path) == false)
            return -1;

        fb::console::set_mode(fb::console::mode::plain);
        auto context = boost::asio::io_context{1};
        if (role == "master")
        {
            auto master = fb::bot::cluster::master(context,
                                                   fb::bot::cluster::master::options{
                                                       .listen = vm["listen"].as<string>(),
                                                       .token  = vm["token"].as<string>(),
                                                       .web    = vm["web"].as<string>(),
                                                   });
            boost::asio::co_spawn(context, master.run(), [](std::exception_ptr error) {
                if (error == nullptr)
                    return;

                try
                {
                    std::rethrow_exception(error);
                }
                catch (const std::exception& e)
                {
                    fb::console::puts("master: {}", e.what());
                }
                std::exit(-1);
            });
            context.run();
        }
        else
        {
            auto name  = vm["name"].as<string>();
            auto seats = vm["seats"].as<uint32_t>();
            if (name.empty())
                name = boost::asio::ip::host_name();
            if (seats == 0)
                seats = std::max(1u, fb::config<uint32_t>("thread:logic", 1u));

            auto agent = fb::bot::cluster::agent(context,
                                                 fb::bot::cluster::agent::options{
                                                     .executable = std::filesystem::absolute(argv[0]).string(),
                                                     .master     = vm["master"].as<string>(),
                                                     .token      = vm["token"].as<string>(),
                                                     .name       = name,
                                                     .config     = config_path,
                                                     .seats      = seats,
                                                     .max_bots   = vm["max-bots"].as<uint32_t>(),
                                                 });
            boost::asio::co_spawn(context, agent.run(), boost::asio::detached);
            context.run();
        }
        return 0;
    }
    else if (role.empty() == false)
    {
        fb::console::puts("Invalid role: {}", role);
        return -1;
    }

    auto result = 0;
    if (mode_str == "integration")
    {
        auto names   = std::vector<string>();
        auto sstream = std::stringstream(vm["test"].as<string>());
        auto name    = string();
        while (std::getline(sstream, name, ','))
        {
            if (name.empty() == false)
                names.push_back(name);
        }
        fb::bot::integration::lua_integration_test::select(std::move(names), vm["label"].as<string>());

        result = run_bot_test<fb::bot::test_mode::INTEGRATION_TEST>(config_path, overrides, cluster);
    }
    else if (mode_str == "load")
    {
        result = run_bot_test<fb::bot::test_mode::LOAD_TEST>(config_path, overrides, cluster);
    }
    else
    {
        fb::console::puts("Invalid test mode: {}", mode_str);
        fb::console::puts("Valid modes: 'load', 'integration'");
        return -1;
    }

    if (cluster)
    {
        std::fflush(stdout);
        std::fflush(stderr);
        std::_Exit(result);
    }
    return result;
}
