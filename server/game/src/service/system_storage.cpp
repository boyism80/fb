#include <fb/game/service/system_storage.h>
#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/config.h>
#include <fb/amqp_route.h>
#include <fb/logger.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <json/json.h>
#include <sstream>
#include <algorithm>
#include <format>
#include <mutex>

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

    if (dto.expired_date.has_value() && !dto.expired_date.value().empty())
        box.expire_date = fb::model::datetime(dto.expired_date.value());

    if (dto.external_ref.has_value() && !dto.external_ref.value().empty())
        box.external_ref = dto.external_ref.value();

    return box;
}

storage_box::entry service::system_storage::from_storage_box_dto(const fb::protocol::internal::StorageBox& dto)
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

std::string service::system_storage::attachments_to_json(const std::vector<fb::model::dsl>& attachments)
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
    return stream.str();
}

service::system_storage::system_storage(fb::game::server& server) :
    server(server)
{ }

void service::system_storage::deliver(const std::vector<storage_box::entry>& entries,
                                      const std::vector<uint32_t>&           user_ids)
{
    if (entries.empty() || entries.size() != user_ids.size())
        return;

    for (size_t i = 0; i < entries.size(); ++i)
    {
        auto ch = this->server.characters.find(user_ids[i]);
        if (ch == nullptr)
            continue;

        auto weak    = ch->template weak_from_this_as<character>();
        auto entry   = entries[i];
        auto builder = this->server.threads.new_builder(weak);
        builder.func = [weak, entry](auto&) -> async::task<void> {
            auto ptr = weak.lock();
            if (ptr == nullptr)
                co_return;

            if (entry.system_storage_box_id.has_value() &&
                ptr->storage_box.contains_system_box(entry.system_storage_box_id.value()))
                co_return;

            ptr->storage_box.add({entry});
            co_return;
        };
        builder.enqueue();
    }
}

void service::system_storage::on_write_box(const fb::protocol::internal::StorageBox& dto)
{
    auto entry = from_storage_box_dto(dto);
    auto user  = dto.user != 0 ? dto.user : 0u;
    if (user == 0)
        return;

    this->deliver({entry}, {user});
}

void service::system_storage::on_deliver(const std::vector<fb::protocol::internal::StorageWriteEntry>& entries)
{
    auto converted = std::vector<storage_box::entry>{};
    auto user_ids  = std::vector<uint32_t>{};
    converted.reserve(entries.size());
    user_ids.reserve(entries.size());

    for (const auto& entry : entries)
    {
        const auto& box     = entry.box;
        const auto  user_id = box.user != 0 ? box.user : entry.user;
        user_ids.push_back(user_id);
        converted.push_back(from_storage_box_dto(box));
    }

    this->deliver(converted, user_ids);
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
        entries.push_back(from_storage_box_dto(dto));
    }

    this->init_character(ch, entries);
}

async::task<void> service::system_storage::sync(character& ch)
{
    const auto  world = ch.world();
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

        const auto now = this->server.now();
        auto       ids = std::vector<uint32_t>{};

        for (const auto& dto : resp.boxes)
        {
            auto box = from_system_storage_dto(dto);
            if (box.expired(now))
                continue;

            if (!box.eligible_for(*ptr))
                continue;

            if (ptr->storage_box.contains_system_box(box.id))
                continue;

            try
            {
                auto&& deliver_resp = co_await this->server.http.post(
                    "internal",
                    "/storage/system/deliver",
                    internal_reqs::DeliverSystemStorage{world, box.id, {ptr->id}, fb::config<uint32_t>("id")});
                if (deliver_resp.error != 0)
                    fb::logger::warn("DeliverSystemStorage sync failed for box {}: error {}",
                                     box.id,
                                     deliver_resp.error);
            }
            catch (const std::exception& e)
            {
                fb::logger::warn("DeliverSystemStorage sync request failed for box {}: {}", box.id, e.what());
            }
        }
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("sync system storage failed for user {}: {}", ch.id, e.what());
    }

    co_return;
}

async::task<bool> service::system_storage::create(uint32_t                           world,
                                                  uint32_t                           user_id,
                                                  std::string_view                   external_ref,
                                                  std::string_view                   title,
                                                  std::string_view                   message,
                                                  const std::vector<fb::model::dsl>& attachments,
                                                  const std::optional<std::string>&  expire_date)
{
    const auto attachments_json = attachments_to_json(attachments);

    try
    {
        auto&& resp = co_await this->server.http.post(
            "internal",
            "/storage/write",
            internal_reqs::WriteStorageBox{world,
                                           user_id,
                                           std::nullopt,
                                           std::string(title),
                                           std::string(message),
                                           attachments_json,
                                           expire_date,
                                           external_ref.empty() ? std::nullopt
                                                                : std::make_optional(std::string(external_ref)),
                                           fb::config<uint32_t>("id")});

        if (resp.error != 0)
        {
            fb::logger::warn("WriteStorageBox failed for {}: error {}", external_ref, resp.error);
            co_return false;
        }

        co_return true;
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("WriteStorageBox request failed for {}: {}", external_ref, e.what());
        co_return false;
    }
}

async::task<bool> service::system_storage::create(uint32_t                           world,
                                                  std::string_view                   user_name,
                                                  std::string_view                   title,
                                                  std::string_view                   message,
                                                  const std::vector<fb::model::dsl>& attachments,
                                                  const std::optional<std::string>&  expire_date,
                                                  std::string_view                   external_ref)
{
    const auto attachments_json = attachments_to_json(attachments);

    try
    {
        auto&& resp = co_await this->server.http.post(
            "internal",
            "/storage/write",
            internal_reqs::WriteStorageBox{world,
                                           0,
                                           std::make_optional(std::string(user_name)),
                                           std::string(title),
                                           std::string(message),
                                           attachments_json,
                                           expire_date,
                                           external_ref.empty() ? std::nullopt
                                                                : std::make_optional(std::string(external_ref)),
                                           fb::config<uint32_t>("id")});

        if (resp.error != 0)
        {
            fb::logger::warn("WriteStorageBox failed for name {}: error {}", user_name, resp.error);
            co_return false;
        }

        co_return true;
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("WriteStorageBox request failed for name {}: {}", user_name, e.what());
        co_return false;
    }
}

async::task<bool> service::system_storage::create_system(uint32_t                           world,
                                                         std::string_view                   title,
                                                         std::string_view                   message,
                                                         const std::vector<fb::model::dsl>& attachments,
                                                         const std::optional<std::string>&  expire_date,
                                                         std::string_view                   external_ref)
{
    const auto attachments_json = attachments_to_json(attachments);

    try
    {
        auto&& resp = co_await this->server.http.post(
            "internal",
            "/storage/system",
            internal_reqs::WriteSystemStorageBox{world,
                                                 std::string(title),
                                                 std::string(message),
                                                 attachments_json,
                                                 expire_date,
                                                 external_ref.empty() ? std::nullopt
                                                                      : std::make_optional(std::string(external_ref))});

        if (resp.error != 0)
        {
            fb::logger::warn("WriteSystemStorageBox failed for {}: error {}", external_ref, resp.error);
            co_return false;
        }

        co_return true;
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("WriteSystemStorageBox request failed for {}: {}", external_ref, e.what());
        co_return false;
    }
}

async::task<void> service::system_storage::poll_and_deliver()
{
    auto world = fb::config<std::optional<uint32_t>>("world");
    if (!world)
        co_return;

    const auto now = this->server.now();
    prune_expired_boxes(this->_pending_boxes, now);

    if (this->server.characters.size() == 0)
        co_return;

    auto        max_box_id = uint32_t{0};
    const auto& fetch_url  = std::format("/storage/system/{}?offset={}", *world, this->_poll_offset);

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

    static constexpr size_t chunk_limit = 100;
    for (const auto& box : this->_pending_boxes)
    {
        if (box.expired(now))
            continue;

        auto eligible = std::make_shared<std::vector<uint32_t>>();
        auto mutex    = std::make_shared<std::mutex>();
        co_await this->server.characters.foreach_async(
            [eligible, mutex, box_id = box.id, created = box.created_date](auto& ch) -> async::task<void> {
                if (ch->created_date() < created && !ch->storage_box.contains_system_box(box_id))
                {
                    auto _ = std::lock_guard(*mutex);
                    eligible->push_back(ch->id);
                }
                co_return;
            });

        if (eligible->empty())
            continue;

        for (std::size_t i = 0; i < eligible->size(); i += chunk_limit)
        {
            const auto end         = std::min(i + chunk_limit, eligible->size());
            auto       chunk_users = std::vector<uint32_t>{};
            chunk_users.assign(eligible->begin() + static_cast<std::ptrdiff_t>(i),
                               eligible->begin() + static_cast<std::ptrdiff_t>(end));

            try
            {
                auto&& resp =
                    co_await this->server.http.post("internal",
                                                    "/storage/system/deliver",
                                                    internal_reqs::DeliverSystemStorage{*world,
                                                                                        box.id,
                                                                                        std::move(chunk_users),
                                                                                        fb::config<uint32_t>("id")});
                if (resp.error != 0)
                    fb::logger::warn("DeliverSystemStorage failed for box {}: error {}", box.id, resp.error);
            }
            catch (const std::exception& e)
            {
                fb::logger::warn("DeliverSystemStorage request failed for box {}: {}", box.id, e.what());
            }
        }
    }

    co_return;
}
