#include <fb/game/context.h>
#include <fb/leak.h>
#include <fb/mst.h>
#include <fb/game/worker.h>
#ifdef _WIN32
#include <Windows.h>
#include "resource.h"
#else
#include <execinfo.h>
#endif

using namespace fb;
using namespace fb::model::enum_value;

int main(int argc, const char** argv)
{
    try
    {
        //_CrtSetBreakAlloc(7997394);

#ifdef _WIN32
        ::SetConsoleIcon(IDI_BARAM);
        ::SetConsoleTitle(CONSOLE_TITLE);
        fb::model::option::decoding(cp949);
        flatbuffers::option::encoding(utf8);
        flatbuffers::option::decoding(cp949);
#endif

        auto io_context                = boost::asio::io_context{};
        auto context                   = std::make_unique<fb::game::context>(io_context, config<uint16_t>("port"));
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

        context->run();
    }
    catch (std::exception& e)
    {
        fb::logger::fatal(std::format("unhandled exception catched in main : {}", e.what()));
#ifndef _WIN32
        void*  array[10];
        size_t size;
        size = backtrace(array, 10);
        backtrace_symbols_fd(array, size, STDERR_FILENO);
#endif
    }

    // Release
    return 0;
}