#include <fb/game/service/system_storage.h>
#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/config.h>
#include <fb/logger.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <json/json.h>
#include <sstream>
#include <algorithm>
#include <format>

using namespace fb::game;
namespace internal_resp = fb::protocol::internal::response;
namespace internal_reqs = fb::protocol::internal::request;

system_storage_box service::system_storage::from_system_storage_dto(const fb::protocol::internal::SystemStorageBox& dto)
{
    auto box = system_storage_box{
        .id           = dto.id,
        .title        = dto.title,
        .message      = dto.message,
        .attachments  = dto.attachments,
        .created_date = fb::model::datetime(dto.created_date),
    };

    if (dto.user != 0)
        box.user = dto.user;

    if (dto.expired_date.has_value() && !dto.expired_date.value().empty())
        box.expire_date = fb::model::datetime(dto.expired_date.value());

    if (dto.external_ref.has_value() && !dto.external_ref.value().empty())
        box.external_ref = dto.external_ref.value();

    return box;
}

storage_box::entry service::system_storage::from_login_storage_box(const fb::protocol::internal::StorageBox& dto)
{
    auto box = storage_box::entry{};
    box.id   = dto.id;
    if (dto.system_storage_box_id != 0)
        box.system_storage_box_id = dto.system_storage_box_id;
    box.title   = dto.title;
    box.message = dto.message;

    if (!dto.attachments.empty())
    {
        auto json   = Json::Value{};
        auto reader = Json::Reader{};
        auto stream = std::istringstream(dto.attachments);
        if (reader.parse(stream, json) && json.isArray())
        {
            box.attachments.reserve(json.size());
            for (const auto& item : json)
            {
                box.attachments.emplace_back(item);
            }
        }
    }

    box.received = dto.received;
    if (dto.expired_date.has_value())
        box.expire_date = fb::model::datetime(dto.expired_date.value());

    return box;
}

void service::system_storage::prune_expired_boxes(std::vector<system_storage_box>& boxes,
                                                  const fb::model::datetime&       now)
{
    boxes.erase(std::remove_if(boxes.begin(),
                               boxes.end(),
                               [&](const system_storage_box& box) {
                                   return box.expired(now);
                               }),
                boxes.end());
}

service::system_storage::system_storage(fb::game::server& server) :
    server(server)
{ }

void service::system_storage::on_deliver(const std::vector<uint32_t>& user_ids, const system_storage_box& box)
{
    if (user_ids.empty())
        return;

    auto entry = box.to_entry();

    for (const auto user_id : user_ids)
    {
        auto ch = this->server.characters.find(user_id);
        if (ch == nullptr)
            continue;

        auto weak    = ch->template weak_from_this_as<character>();
        auto builder = this->server.threads.new_builder(weak);
        builder.func = [weak, box_id = box.id, entry](auto&) -> async::task<void> {
            auto ptr = weak.lock();
            if (ptr == nullptr)
                co_return;

            if (ptr->storage_box.contains_system_box(box_id))
                co_return;

            ptr->storage_box.apply_delivered({entry});
            co_return;
        };
        builder.enqueue();
    }
}

void service::system_storage::init_character(character& ch, const std::vector<storage_box::entry>& entries)
{
    ch.storage_box.init(entries);
}

void service::system_storage::init_from_login(character&                                             ch,
                                              const std::vector<fb::protocol::internal::StorageBox>& boxes)
{
    auto entries = std::vector<storage_box::entry>{};
    entries.reserve(boxes.size());
    for (const auto& dto : boxes)
    {
        entries.push_back(from_login_storage_box(dto));
    }

    this->init_character(ch, entries);
}

async::task<void> service::system_storage::sync(character& ch)
{
    const auto  world = fb::config<uint32_t>("world");
    const auto& url   = std::format("/storage/system/{}?offset=0", world);

    auto weak = ch.weak_from_this_as<character>();
    try
    {
        auto&& resp = co_await this->server.http.get<internal_resp::GetSystemStorageBoxes>("internal", url);
        co_await this->server.threads.switching(weak);

        auto ptr = weak.lock();
        if (ptr == nullptr)
            co_return;

        if (resp.error != 0)
            co_return;

        const auto now   = this->server.now();
        auto       batch = std::vector<storage_box::entry>{};

        for (const auto& dto : resp.boxes)
        {
            auto box = from_system_storage_dto(dto);
            if (box.expired(now))
                continue;

            if (!box.eligible_for(*ptr))
                continue;

            if (ptr->storage_box.contains_system_box(box.id))
                continue;

            batch.push_back(box.to_entry());
        }

        if (!batch.empty())
            ptr->storage_box.apply_delivered(batch);
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("sync system storage failed for user {}: {}", ch.id, e.what());
    }

    co_return;
}

async::task<void> service::system_storage::create(uint32_t                           user_id,
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
        {
            json_array.append(dsl.to_json());
        }

        auto builder           = Json::StreamWriterBuilder{};
        builder["emitUTF8"]    = true;
        builder["indentation"] = "";
        auto writer            = std::unique_ptr<Json::StreamWriter>(builder.newStreamWriter());
        auto stream            = std::ostringstream{};
        writer->write(json_array, &stream);
        attachments_json = stream.str();
    }

    auto&& resp = co_await this->server.http.post("internal",
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

async::task<void> service::system_storage::poll_and_deliver()
{
    const auto now = this->server.now();
    prune_expired_boxes(this->_pending_boxes, now);

    if (this->server.characters.size() == 0)
        co_return;

    auto        max_box_id = uint32_t{0};
    const auto  world      = fb::config<uint32_t>("world");
    const auto& fetch_url  = std::format("/storage/system/{}?offset={}", world, this->_poll_offset);

    try
    {
        auto&& resp = co_await this->server.http.get<internal_resp::GetSystemStorageBoxes>("internal", fetch_url);
        if (resp.error == 0)
        {
            for (const auto& dto : resp.boxes)
            {
                auto box = from_system_storage_dto(dto);
                if (box.expired(now))
                    continue;

                max_box_id = std::max(max_box_id, box.id);

                const auto already_tracked = std::any_of(this->_pending_boxes.cbegin(),
                                                         this->_pending_boxes.cend(),
                                                         [&](const system_storage_box& existing) {
                                                             return existing.id == box.id;
                                                         });
                if (!already_tracked)
                    this->_pending_boxes.push_back(std::move(box));
            }
        }
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("Failed to fetch system storage boxes: {}", e.what());
    }

    if (max_box_id > 0)
        this->_poll_offset = max_box_id + 1;

    for (const auto& box : this->_pending_boxes)
    {
        if (box.expired(now))
            continue;

        if (box.user.has_value())
        {
            this->on_deliver({box.user.value()}, box);
        }
        else
        {
            auto eligible = this->server.characters.collect_ids([&](const auto& ch) {
                return ch->created_date() < box.created_date;
            });

            this->on_deliver(eligible, box);
        }
    }

    co_return;
}
