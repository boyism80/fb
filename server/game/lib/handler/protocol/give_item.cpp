#include <fb/game/handler/protocol/give_item.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

give_item::give_item(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, fb::protocol::game::request::give_item>(server)
{ }

async::task<bool> give_item::handle(fb::socket<character>& session, fb::protocol::game::request::give_item& request)
{
    auto me = session.data();
    if (me->inited() == false)
        co_return true;

    if (me->map() == nullptr)
        co_return true;

    auto forward = me->forward();
    if (forward == nullptr)
        co_return true;

    auto item = me->items[request.slot];
    if (item == nullptr)
        co_return true;

    auto  count = request.all ? item->count() : 1;
    auto& model = item->based<fb::model::item>();

    try
    {
        if (model.trade == false)
            throw std::runtime_error("줄 수 없습니다.");

        switch (forward->what())
        {
        case OBJECT_TYPE::CHARACTER:
        {
            auto you = std::static_pointer_cast<fb::game::character>(forward);
            if (model.attr(ITEM_ATTRIBUTE::BUNDLE) && you->items.index(model) != 0xFF)
            {
                auto exists = you->items.find(model);
                count       = std::min(model.capacity - exists->count(), count);
                if (count == 0)
                    throw std::runtime_error("상대방의 인벤토리가 가득 찼습니다.");
            }
            else
            {
                if (you->items.free() == false)
                    throw std::runtime_error("상대방의 인벤토리가 가득 찼습니다.");
            }

            item = me->items.remove(item, count, ITEM_DELETE_TYPE::GIVE);
            if (count == 1)
                you->message(std::format("{}님이 {} 주었습니다.", me->name(), name_with(item->name())));
            else
                you->message(std::format("{}님이 {} {}개 주었습니다.", me->name(), name_with(item->name()), count));
            you->items.add(item);
        }
        break;

        case OBJECT_TYPE::MOB:
        {
            auto mob = std::static_pointer_cast<fb::game::mob>(forward);
            if (mob->items().size() >= CONTAINER_CAPACITY)
                throw std::runtime_error("더 이상 줄 수 없습니다.");

            item = me->items.remove(item, count, ITEM_DELETE_TYPE::GIVE, true);
            mob->push_item(item);
        }
        break;

        default:
            co_return true;
        }
    }
    catch (std::exception& e)
    {
        me->message(e.what());
    }

    co_return true;
}
