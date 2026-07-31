#include <fb/game/handler/protocol/item_combine.h>
#include <fb/game/server.h>
#include <fb/encoding.h>
#include <json/json.h>

using namespace fb::game::handler::protocol;
using table = fb::model::table;

namespace game_reqs = fb::protocol::game::request;

item_combine::item_combine(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::item_combine>(server)
{ }

async::task<bool> item_combine::handle(fb::socket<character>& session, game_reqs::item_combine& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    if (request.indices.size() > CONTAINER_CAPACITY - 1)
        co_return false;

    auto dsl = std::vector<fb::model::dsl::item>();
    for (auto index : request.indices)
    {
        auto item = ch->items[index];
        if (item == nullptr)
            continue;

        auto& model = item->based<fb::model::item>();
        dsl.push_back(fb::model::dsl::item(model.id, item->count(), std::nullopt, std::nullopt, 100.0));
    }

    auto found = table::recipe->find(dsl);
    if (found == nullptr)
    {
        ch->message(_TEXT(MESSAGE_NO_RECIPE));
        co_return true;
    }

    if (found->success.size() > ch->items.free_size() + found->source.size())
    {
        ch->message(_TEXT(MESSAGE_EXCEPTION_INVENTORY_OVERFLOW));
        co_return true;
    }

    // Record items to be removed (without actually removing them yet)
    auto items_to_remove = std::vector<std::pair<std::shared_ptr<fb::game::item>, uint32_t>>();
    for (auto& x : found->source)
    {
        auto params        = fb::model::dsl::item(x.params);
        auto deleted_count = uint32_t(0);
        while (deleted_count < params.count)
        {
            auto item  = ch->items.find(table::item[params.id]);
            auto index = ch->items.index(table::item[params.id]);
            if (item == nullptr)
            {
                throw std::runtime_error(
                    std::format("user {} try to combine with {} but has no item", ch->name(), params.id));
            }

            auto count = item->count();
            items_to_remove.push_back({item, count});
            deleted_count += count;
        }
    }

    // Remove source items (always consumed regardless of success/failure)
    for (auto& [item, count] : items_to_remove)
    {
        auto deleted = ch->items.remove(item, count);
        if (deleted != nullptr)
            co_await deleted->destroy();
    }

    // Calculate success/failure probability
    auto  success = (std::rand() % 100) < found->percent;
    auto& result  = success ? found->success : found->failed;

    // Add result items (success or failure items)
    for (auto& dsl : result)
    {
        auto  params     = fb::model::dsl::item(dsl.params);
        auto  item_table = table::item;
        auto& model      = item_table[params.id];
        auto  remain     = params.count;
        while (remain > 0)
        {
            auto item  = this->server.make<fb::game::item>(table::item[params.id]);
            auto count = std::min<uint16_t>(model.capacity, remain);
            item->count(count);
            std::ignore  = co_await ch->items.add(item);
            remain      -= count;
        }
    }

    auto& message = success ? _TEXT(MESSAGE_MIX_SUCCESS) : _TEXT(MESSAGE_MIX_FAILED);
    ch->message(message);

    // Log item combine event
    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
    log_data["character_name"] = UTF8(ch->name(), PLATFORM::WINDOWS);
    log_data["success"]        = success;
    auto source_items          = Json::Value(Json::arrayValue);
    for (auto& x : found->source)
    {
        auto params          = fb::model::dsl::item(x.params);
        auto item_data       = Json::Value();
        item_data["item_id"] = static_cast<Json::Int64>(params.id);
        item_data["count"]   = static_cast<Json::Int64>(params.count);
        source_items.append(item_data);
    }
    log_data["source_items"] = source_items;
    auto result_items        = Json::Value(Json::arrayValue);
    for (auto& dsl : result)
    {
        auto params          = fb::model::dsl::item(dsl.params);
        auto item_data       = Json::Value();
        item_data["item_id"] = static_cast<Json::Int64>(params.id);
        item_data["count"]   = static_cast<Json::Int64>(params.count);
        result_items.append(item_data);
    }
    log_data["result_items"] = result_items;
    this->server.log.write("item_combine", log_data);

    co_return true;
}
