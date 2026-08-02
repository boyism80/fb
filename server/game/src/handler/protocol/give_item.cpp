#include <fb/game/handler/protocol/give_item.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

give_item::give_item(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::give_item>(server)
{ }

async::task<bool> give_item::handle(fb::socket<character>& session, game_reqs::give_item& request)
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
    auto& model = item->model();
    auto  error = std::optional<std::string>{};

    try
    {
        if (model.trade == false)
            throw std::runtime_error(_TEXT(MESSAGE_ITEM_CANNOT_GIVE));

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
                    throw std::runtime_error(_TEXT(MESSAGE_ITEM_TARGET_INVENTORY_FULL));
            }
            else
            {
                if (you->items.free() == false)
                    throw std::runtime_error(_TEXT(MESSAGE_ITEM_TARGET_INVENTORY_FULL));
            }

            item = me->items.remove(item, count, ITEM_DELETE_TYPE::GIVE);
            if (count == 1)
                you->message(std::format(_TEXT(MESSAGE_ITEM_GIVE_SINGLE), me->name(), name_with(item->name())));
            else
                you->message(
                    std::format(_TEXT(MESSAGE_ITEM_GIVE_MULTIPLE), me->name(), name_with(item->name()), count));
            std::ignore = co_await you->items.add(item);
        }
        break;

        case OBJECT_TYPE::MOB:
        {
            auto mob = std::static_pointer_cast<fb::game::mob>(forward);
            if (mob->items().size() >= CONTAINER_CAPACITY)
                throw std::runtime_error(_TEXT(MESSAGE_ITEM_CANNOT_GIVE_ANYMORE));

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
        error = e.what();
    }

    if (error.has_value())
        me->message(error.value());

    co_return true;
}
