#include <fb/game/context.h>
#include <fb/leak.h>
#include <fb/mst.h>
#include <fb/game/worker.h>
#include <boost/program_options.hpp>
#include <filesystem>
#ifdef _WIN32
#include <Windows.h>
#include "resource.h"
#else
#include <execinfo.h>
#endif

using namespace fb;
using namespace fb::model::enum_value;

int main(int argc, char* argv[])
{
    namespace po = boost::program_options;

    try
    {
        po::options_description desc("Game Server Options");
        desc.add_options()("help,h", "Show help message")("config,c",
                                                          po::value<std::string>()->default_value("config.json"),
                                                          "Configuration file path");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help"))
            return std::cout << desc << std::endl, 0;

        auto config_path = vm["config"].as<std::string>();

        // Initialize config system
        if (!fb::init_config(config_path))
        {
            std::cerr << "Failed to initialize config from: " << config_path << std::endl;
            return -1;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Command line parsing error: " << e.what() << std::endl;
        return -1;
    }

    try
    {
        //_CrtSetBreakAlloc(7997394);

#ifdef _WIN32
        ::set_console_icon(IDI_BARAM);
        ::SetConsoleTitle(CONSOLE_TITLE);
        fb::model::option::decoding(cp949);
        flatbuffers::option::encoding(utf8);
        flatbuffers::option::decoding(cp949);
#endif

        auto io_context = boost::asio::io_context{};
        auto context    = std::make_shared<fb::game::context>(io_context, fb::config<uint16_t>("port"));
        auto signals    = boost::asio::signal_set(io_context, SIGINT, SIGTERM);
        signals.async_wait([&context](const boost::system::error_code& ec, int signal_number) {
            context->exit();
        });

        context->model.item.hook.build = [](const Json::Value& json) -> fb::model::item* {
            auto type = fb::model::build<ITEM_TYPE>(json["type"]);
            switch (type)
            {
            case ITEM_TYPE::STUFF:
                return fb::model::build<fb::model::item*>(json);
            case ITEM_TYPE::CASH:
                return fb::model::build<fb::model::cash*>(json);
            case ITEM_TYPE::CONSUME:
                return fb::model::build<fb::model::consume*>(json);
            case ITEM_TYPE::WEAPON:
                return fb::model::build<fb::model::weapon*>(json);
            case ITEM_TYPE::ARMOR:
                return fb::model::build<fb::model::armor*>(json);
            case ITEM_TYPE::HELMET:
                return fb::model::build<fb::model::helmet*>(json);
            case ITEM_TYPE::RING:
                return fb::model::build<fb::model::ring*>(json);
            case ITEM_TYPE::SHIELD:
                return fb::model::build<fb::model::shield*>(json);
            case ITEM_TYPE::AUXILIARY:
                return fb::model::build<fb::model::auxiliary*>(json);
            case ITEM_TYPE::BOW:
                return fb::model::build<fb::model::bow*>(json);
            case ITEM_TYPE::PACKAGE:
                return fb::model::build<fb::model::pack*>(json);
            default:
                return nullptr;
            }
        };
        fb::model::loader(context->model).run();
        fb::game::map_loader(*context).run();
        fb::game::script_loader(*context).run();
        fb::game::npc_spawner(*context).run();

        fb::console::set_mode(fb::console::mode::plain);
        context->run();
    }
    catch (std::exception& e)
    {
        std::cerr << "unhandled exception catched in main : " << e.what() << std::endl;
    }

    // Release
    return 0;
}