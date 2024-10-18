#include "resource.h"
#include <fb/game/context.h>
// #include <fb/leak.h>
#include <fb/mst.h>
#include <fb/game/worker.h>

using namespace fb::model::enum_value;

int main(int argc, const char** argv)
{
    auto& c = fb::console::get();
    auto& config = fb::config::get();

    auto height = 9;
    c.box(c.width()-1, height);

    auto header = "The Kingdom of the wind [GAME]";
    c.cursor((c.width()-1 - strlen(header)) / 2, 3).render(header);

    auto github = "https://github.com/boyism80/fb";
    c.cursor(c.width()-1 - strlen(github) - 3, 5).render(github);

    auto madeby = "made by cshyeon";
    c.cursor(c.width()-1 - strlen(madeby) - 3, 6).render(madeby);

    c.cursor(0, height + 1);

    try
    {
        //_CrtSetBreakAlloc(7997394);

#ifdef _WIN32
        fb::model::option::encoding(cp949);
        ::SetConsoleIcon(IDI_BARAM);
        ::SetConsoleTitle(CONSOLE_TITLE);
#endif

        boost::asio::io_context io_context;
        auto& config = fb::config::get();
        auto context = std::make_unique<fb::game::context>(io_context, config["port"].asInt());
        context->model.mob_spawn.hook.built = [&context](const fb::model::array_container<fb::model::mob_spawn>& model) 
        {
            for (auto& spawn : model)
            {
                context->rezen.push_back(fb::game::rezen(*context.get(), spawn));
            }
        };

        context->model.item.hook.build = [](const Json::Value& json) ->fb::model::item*
        {
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
        fb::game::model_loader(*context).run();
        fb::game::map_loader(*context).run();
        fb::game::npc_spawner(*context).run();

        context->run();
    }
    catch(std::exception& e)
    {
        fb::logger::fatal(e.what());
    }

    // Release
    return 0;
}