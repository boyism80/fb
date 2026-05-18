#include <fb/game/server.h>
#include <fb/config.h>
#include <fb/game/system_storage_box.h>
#include <fb/logger.h>
#include <json/json.h>
#include <sstream>

using namespace fb::game;

namespace internal_resp = fb::protocol::internal::response;
namespace internal_reqs = fb::protocol::internal::request;

namespace {

bool is_expired(const system_storage_box& box, const fb::model::datetime& now)
{
    return box.expire_date.has_value() && box.expire_date.value() < now;
}

bool has_system_storage_entry(const storage_box& box, uint32_t system_storage_box_id)
{
    return std::any_of(box.entries().cbegin(), box.entries().cend(), [&](const auto& pair) {
        return pair.second.system_storage_box_id.has_value() &&
               pair.second.system_storage_box_id.value() == system_storage_box_id;
    });
}

bool is_eligible(const character& ch, const system_storage_box& box)
{
    if (box.user.has_value())
        return ch.id == box.user.value();

    return ch.created_date() < box.created_date;
}

storage_box::entry entry_from_system_storage(const system_storage_box& box)
{
    auto e                  = storage_box::entry{};
    e.system_storage_box_id = box.id;
    e.title                 = box.title;
    e.message               = box.message;

    if (!box.attachments.empty())
    {
        auto json   = Json::Value{};
        auto reader = Json::Reader{};
        auto stream = std::istringstream(box.attachments);
        if (reader.parse(stream, json) && json.isArray())
        {
            e.attachments.reserve(json.size());
            for (const auto& item : json)
                e.attachments.emplace_back(item);
        }
    }

    e.received = false;
    if (box.expire_date.has_value())
        e.expire_date = box.expire_date;

    return e;
}

} // namespace

void server::apply_system_storage_to_users(const std::vector<uint32_t>& user_ids, const system_storage_box& box)
{
    if (user_ids.empty())
        return;

    auto entry = entry_from_system_storage(box);

    this->characters.write([this, user_ids, box_id = box.id, entry = std::move(entry)](auto& characters) {
        for (const auto user_id : user_ids)
        {
            auto ch = characters.find(user_id);
            if (ch == nullptr)
                continue;

            auto weak = ch->template weak_from_this_as<character>();
            this->threads.enqueue(weak, [ch, box_id, entry](auto&) -> async::task<void> {
                if (has_system_storage_entry(ch->storage_box, box_id))
                    co_return;

                ch->storage_box.apply_delivered({entry});
                co_return;
            });
        }
    });
}

async::task<void> server::sync_system_storage_for_character(character& ch)
{
    const auto  world = fb::config<uint32_t>("world");
    const auto& url   = std::format("/storage/system/{}?offset=0", world);

    auto weak = ch.weak_from_this_as<character>();
    try
    {
        auto&& resp = co_await this->http.get<internal_resp::GetSystemStorageBoxes>("internal", url);
        co_await this->threads.switching(weak);

        auto ptr = weak.lock();
        if (ptr == nullptr)
            co_return;

        if (resp.error != 0)
            co_return;

        const auto now   = this->now();
        auto       batch = std::vector<storage_box::entry>{};

        for (const auto& dto : resp.boxes)
        {
            auto box = system_storage_box(dto);
            if (is_expired(box, now))
                continue;

            if (!is_eligible(*ptr, box))
                continue;

            if (has_system_storage_entry(ptr->storage_box, box.id))
                continue;

            batch.push_back(entry_from_system_storage(box));
        }

        if (!batch.empty())
            ptr->storage_box.apply_delivered(batch);
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("sync_system_storage_for_character failed for user {}: {}", ch.id, e.what());
    }

    co_return;
}

async::task<void> server::create_system_storage(uint32_t                           user_id,
                                                std::string_view                   external_ref,
                                                std::string_view                   title,
                                                std::string_view                   message,
                                                const std::vector<fb::model::dsl>& attachments)
{
    const auto world = fb::config<uint32_t>("world");

    auto attachments_json = std::string{};
    {
        auto json_array = Json::Value(Json::arrayValue);
        for (const auto& dsl : attachments)
            json_array.append(dsl.to_json());

        auto builder           = Json::StreamWriterBuilder{};
        builder["emitUTF8"]    = true;
        builder["indentation"] = "";
        auto writer            = std::unique_ptr<Json::StreamWriter>(builder.newStreamWriter());
        auto stream            = std::ostringstream{};
        writer->write(json_array, &stream);
        attachments_json = stream.str();
    }

    auto&& resp = co_await this->http.post("internal",
                                           "/storage/system",
                                           internal_reqs::WriteSystemStorageBox{world,
                                                                                user_id,
                                                                                std::string(title),
                                                                                std::string(message),
                                                                                attachments_json,
                                                                                std::nullopt,
                                                                                std::string(external_ref)});

    if (resp.error != 0)
        fb::logger::warn("WriteSystemStorageBox failed for {}: error {}", external_ref, resp.error);

    co_return;
}
