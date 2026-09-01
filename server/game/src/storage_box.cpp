#include <fb/game/storage.h>
#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/config.h>
#include <fb/logger.h>
#include <fb/model/datetime.h>
#include <fb/model/model.h>
#include <fb/encoding.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <json/json.h>
#include <sstream>
#include <algorithm>

using namespace fb::game;

storage_box::storage_box(character& owner) :
    _owner(owner)
{ }

std::string storage_box::attachments_to_json(const std::vector<fb::model::dsl>& attachments)
{
    if (attachments.empty())
        return "[]";

    auto json_array = Json::Value{Json::arrayValue};
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

fb::protocol::internal::StorageBox storage_box::to_save_dto(uint32_t user_id, const entry& box)
{
    const auto system_storage_id = box.system_storage_box_id.has_value() ? box.system_storage_box_id.value() : 0u;
    return fb::protocol::internal::StorageBox{
        user_id,
        box.id,
        system_storage_id,
        box.title,
        box.message,
        attachments_to_json(box.attachments),
        box.received,
        box.expire_date.has_value() ? std::make_optional(box.expire_date->to_string()) : std::nullopt,
    };
}

std::vector<fb::protocol::internal::StorageBox> storage_box::to_save_dtos(uint32_t                   user_id,
                                                                          const fb::model::datetime& now) const
{
    this->_owner.assert_thread();

    auto dtos = std::vector<fb::protocol::internal::StorageBox>{};
    dtos.reserve(this->_entries.size());
    for (const auto& [id, box] : this->_entries)
    {
        if (box.expire_date.has_value() && box.expire_date.value() < now)
            continue;

        dtos.push_back(to_save_dto(user_id, box));
    }

    return dtos;
}

void storage_box::init(const std::vector<entry>& entries)
{
    this->_owner.assert_thread();

    this->_entries.clear();
    this->_sequence = 1;

    for (const auto& e : entries)
    {
        this->_entries.emplace(e.id, e);
        if (e.id >= this->_sequence)
            this->_sequence = e.id + 1;
    }
}

void storage_box::add(const std::vector<entry>& entries)
{
    this->_owner.assert_thread();

    for (const auto& box : entries)
    {
        if (box.system_storage_box_id.has_value())
        {
            if (this->contains_system_box(box.system_storage_box_id.value()))
                continue;
        }

        auto e = box;
        if (e.id == 0)
        {
            e.id = this->_sequence++;
        }
        else if (e.id >= this->_sequence)
        {
            this->_sequence = e.id + 1;
        }

        this->_entries.emplace(e.id, e);

        auto log_data              = Json::Value();
        log_data["character_id"]   = static_cast<Json::Int64>(this->_owner.id);
        log_data["character_name"] = UTF8(this->_owner.name(), PLATFORM::WINDOWS);
        log_data["entry_id"]       = static_cast<Json::Int64>(e.id);
        if (e.system_storage_box_id.has_value())
            log_data["system_storage_box_id"] = static_cast<Json::Int64>(e.system_storage_box_id.value());
        log_data["title"]      = UTF8(e.title, PLATFORM::WINDOWS);
        auto attachments_array = Json::Value(Json::arrayValue);
        for (const auto& attachment : e.attachments)
        {
            attachments_array.append(attachment.to_json());
        }
        log_data["attachments"] = attachments_array;
        log_data["expire_date"] = e.expire_date.has_value()
                                      ? Json::Value(UTF8(e.expire_date.value().to_string(), PLATFORM::WINDOWS))
                                      : Json::Value::null;
        this->_owner.server.log.write("storage_box_entry_add", log_data);

        if (!e.title.empty())
            this->_owner.message(std::format(_TEXT(MESSAGE_STORAGE_BOX_REWARD_ADDED), e.title), MESSAGE_TYPE::STATE);
        else
            this->_owner.message(_TEXT(MESSAGE_STORAGE_BOX_REWARD_ADDED_NO_TITLE), MESSAGE_TYPE::STATE);
    }
}

bool storage_box::contains_system_box(uint32_t system_storage_box_id) const
{
    this->_owner.assert_thread();

    return std::any_of(this->_entries.cbegin(), this->_entries.cend(), [&](const auto& pair) {
        return pair.second.system_storage_box_id.has_value() &&
               pair.second.system_storage_box_id.value() == system_storage_box_id;
    });
}

async::task<bool> storage_box::receive_reward(uint32_t entry_id)
{
    this->_owner.assert_thread();

    fb::logger::debug("storage_box.receive_reward start user={} entry={} invent_free={} invent_size={} attachments={}",
                      this->_owner.id,
                      entry_id,
                      this->_owner.items.free_size(),
                      CONTAINER_CAPACITY - this->_owner.items.free_size(),
                      this->_entries.contains(entry_id) ? this->_entries.at(entry_id).attachments.size() : 0);

    auto it = this->_entries.find(entry_id);
    if (it == this->_entries.end())
    {
        fb::logger::warn("storage_box.receive_reward entry not found user={} entry={}", this->_owner.id, entry_id);
        co_return false;
    }

    if (it->second.received)
    {
        fb::logger::debug("storage_box.receive_reward already received user={} entry={}", this->_owner.id, entry_id);
        co_return false;
    }

    auto now = this->_owner.server.now();
    if (it->second.expire_date.has_value() && it->second.expire_date.value() < now)
    {
        fb::logger::debug("storage_box.receive_reward expired user={} entry={}", this->_owner.id, entry_id);
        co_return false;
    }

    auto rewardable = this->_owner.items.is_rewardable(it->second.attachments);
    fb::logger::debug("storage_box.receive_reward is_rewardable={} user={} entry={} free={}",
                      rewardable,
                      this->_owner.id,
                      entry_id,
                      this->_owner.items.free_size());
    if (rewardable == false)
    {
        fb::logger::debug("storage_box.receive_reward reject before claim (not rewardable) user={} entry={}",
                          this->_owner.id,
                          entry_id);
        co_return false;
    }

    auto weak  = this->_owner.weak_from_this_as<character>();
    auto world = fb::config<uint32_t>("world");
    fb::logger::debug("storage_box.receive_reward claim begin user={} entry={} world={} http_delay_ms={}",
                      this->_owner.id,
                      entry_id,
                      world,
                      this->_owner.server.http.response_delay().total_milliseconds());

    fb::protocol::internal::response::ClaimStorageBox claim_resp{};
    try
    {
        claim_resp = co_await this->_owner.server.http.post(
            "internal",
            "/storage/claim",
            fb::protocol::internal::request::ClaimStorageBox{world, this->_owner.id, entry_id});
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("storage_box.receive_reward claim exception user={} entry={}: {}",
                         this->_owner.id,
                         entry_id,
                         e.what());
        co_return false;
    }

    fb::logger::debug("storage_box.receive_reward claim done user={} entry={} error={}",
                      this->_owner.id,
                      entry_id,
                      claim_resp.error);

    co_await this->_owner.server.threads.switching(weak);
    auto ptr = weak.lock();
    if (ptr == nullptr)
    {
        fb::logger::warn("storage_box.receive_reward owner gone after claim entry={}", entry_id);
        co_return false;
    }

    if (claim_resp.error != 0)
    {
        fb::logger::warn("storage_box.receive_reward claim failed user={} entry={} error={}",
                         ptr->id,
                         entry_id,
                         claim_resp.error);
        co_return false;
    }

    it = ptr->storage_box._entries.find(entry_id);
    if (it == ptr->storage_box._entries.end())
    {
        fb::logger::warn("storage_box.receive_reward entry missing after claim user={} entry={}", ptr->id, entry_id);
        co_return false;
    }

    it->second.received = true;
    fb::logger::debug("storage_box.receive_reward grant begin user={} entry={} free={}",
                      ptr->id,
                      entry_id,
                      ptr->items.free_size());

    bool granted = false;
    try
    {
        granted = co_await ptr->reward(it->second.attachments);
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("storage_box.receive_reward grant exception user={} entry={}: {}",
                         ptr->id,
                         entry_id,
                         e.what());
        granted = false;
    }

    fb::logger::debug("storage_box.receive_reward grant result={} user={} entry={} free={}",
                      granted,
                      ptr->id,
                      entry_id,
                      ptr->items.free_size());

    if (granted == false)
    {
        fb::logger::debug("storage_box.receive_reward unclaim begin user={} entry={}", ptr->id, entry_id);
        fb::protocol::internal::response::UnclaimStorageBox unclaim_resp{};
        try
        {
            unclaim_resp = co_await ptr->server.http.post(
                "internal",
                "/storage/unclaim",
                fb::protocol::internal::request::UnclaimStorageBox{world, ptr->id, entry_id});
        }
        catch (const std::exception& e)
        {
            fb::logger::warn("storage_box.receive_reward unclaim exception user={} entry={}: {}",
                             ptr->id,
                             entry_id,
                             e.what());
        }

        co_await ptr->server.threads.switching(weak);
        ptr = weak.lock();
        if (ptr != nullptr)
        {
            auto fail_it = ptr->storage_box._entries.find(entry_id);
            if (fail_it != ptr->storage_box._entries.end())
                fail_it->second.received = false;
        }

        if (unclaim_resp.error != 0)
            fb::logger::warn("UnclaimStorageBox failed for user {} entry {}: error {}",
                             this->_owner.id,
                             entry_id,
                             unclaim_resp.error);

        fb::logger::debug("storage_box.receive_reward unclaim done user={} entry={} error={}",
                          this->_owner.id,
                          entry_id,
                          unclaim_resp.error);
        co_return false;
    }

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(ptr->id);
    log_data["character_name"] = UTF8(ptr->name(), PLATFORM::WINDOWS);
    log_data["entry_id"]       = static_cast<Json::Int64>(entry_id);
    log_data["title"]          = UTF8(it->second.title, PLATFORM::WINDOWS);
    auto attachments_array     = Json::Value(Json::arrayValue);
    for (const auto& attachment : it->second.attachments)
    {
        attachments_array.append(attachment.to_json());
    }
    log_data["attachments"] = attachments_array;
    ptr->server.log.write("storage_box_attachment_receive", log_data);

    fb::logger::debug("storage_box.receive_reward success user={} entry={}", ptr->id, entry_id);
    co_return true;
}

void storage_box::set_received(uint32_t entry_id, bool received)
{
    this->_owner.assert_thread();
    auto it = this->_entries.find(entry_id);
    if (it == this->_entries.end())
        return;

    it->second.received = received;
}

const storage_box::entry_map& storage_box::entries() const
{
    this->_owner.assert_thread();
    return this->_entries;
}

uint32_t storage_box::next_sequence() const
{
    this->_owner.assert_thread();
    return this->_sequence;
}

void storage_box::set_sequence(uint32_t value)
{
    this->_owner.assert_thread();
    this->_sequence = value;
}
