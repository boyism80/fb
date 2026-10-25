#include <fb/config.h>
#include <fb/encoding.h>
#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/game/storage.h>
#include <fb/logger.h>
#include <fb/model/datetime.h>
#include <fb/model/model.h>
#include <fb/protocol/flatbuffer/protocol.h>

#include <json/json.h>

#include <algorithm>
#include <cstdint>
#include <exception>
#include <format>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

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

storage_box::box_dto storage_box::to_save_dto(uint32_t user_id, const entry& box)
{
    const auto system_storage_id = box.system_storage_box_id.has_value() ? box.system_storage_box_id.value() : 0u;
    return box_dto{
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

bool storage_box::expired(const entry& box, const fb::model::datetime& now)
{
    return box.expire_date.has_value() && box.expire_date.value() < now;
}

std::vector<storage_box::box_dto> storage_box::to_save_dtos(uint32_t user_id, const fb::model::datetime& now) const
{
    this->_owner.assert_thread();

    auto dtos = std::vector<box_dto>{};
    dtos.reserve(this->_entries.size());
    for (const auto& [id, box] : this->_entries)
    {
        if (box.system_storage_box_id.has_value() == false && (box.received || expired(box, now)))
            continue;

        dtos.push_back(to_save_dto(user_id, box));
    }

    return dtos;
}

void storage_box::init(const std::vector<entry>& entries, const std::vector<uint32_t>& archived)
{
    this->_owner.assert_thread();

    this->_entries.clear();
    this->_sequence = 1;

    for (const auto& e : entries)
    {
        if (e.id >= this->_sequence)
            this->_sequence = e.id + 1;

        // Copies of one system box may exist from older duplicate distributions; keep a single one, the received one if
        // any.
        if (e.system_storage_box_id.has_value())
        {
            auto found = std::find_if(this->_entries.begin(), this->_entries.end(), [&](const auto& pair) {
                return pair.second.system_storage_box_id == e.system_storage_box_id;
            });
            if (found != this->_entries.end())
            {
                if (found->second.received || e.received == false)
                    continue;

                this->_entries.erase(found);
            }
        }

        this->_entries.emplace(e.id, e);
    }

    auto now = this->_owner.server.now();
    for (auto it = this->_entries.begin(); it != this->_entries.end();)
    {
        const auto& box = it->second;
        if (box.system_storage_box_id.has_value() &&
            std::find(archived.begin(), archived.end(), box.system_storage_box_id.value()) != archived.end() &&
            (box.received || expired(box, now)))
            it = this->_entries.erase(it);
        else
            ++it;
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

        if (this->_entries.emplace(e.id, e).second == false)
            continue;

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

const char* storage_box::result_name(receive_result result)
{
    switch (result)
    {
    case receive_result::RECEIVED:
        return "received";
    case receive_result::NOT_FOUND:
        return "not_found";
    case receive_result::ALREADY_RECEIVED:
        return "already_received";
    case receive_result::EXPIRED:
        return "expired";
    case receive_result::INVENTORY_FULL:
        return "inventory_full";
    default:
        return "failed";
    }
}

async::task<storage_box::receive_result> storage_box::receive_reward(uint32_t entry_id)
{
    this->_owner.assert_thread();

    auto it = this->_entries.find(entry_id);
    if (it == this->_entries.end())
    {
        fb::logger::warn("storage_box.receive_reward entry not found user={} entry={}", this->_owner.id, entry_id);
        co_return receive_result::NOT_FOUND;
    }

    if (it->second.received)
        co_return receive_result::ALREADY_RECEIVED;

    if (expired(it->second, this->_owner.server.now()))
        co_return receive_result::EXPIRED;

    if (this->_owner.items.is_rewardable(it->second.attachments) == false)
        co_return receive_result::INVENTORY_FULL;

    auto box            = it->second;
    it->second.received = true;

    auto granted = false;
    try
    {
        granted = co_await this->_owner.reward(box.attachments);
    }
    catch (const std::exception& e)
    {
        fb::logger::fatal("storage_box.receive_reward grant exception user={} entry={}: {}",
                          this->_owner.id,
                          entry_id,
                          e.what());
        co_return receive_result::FAILED;
    }

    it = this->_entries.find(entry_id);
    if (granted == false)
    {
        if (it != this->_entries.end())
            it->second.received = false;
        co_return receive_result::INVENTORY_FULL;
    }

    if (it != this->_entries.end() && box.system_storage_box_id.has_value() == false)
        this->_entries.erase(it);

    auto log_data              = Json::Value();
    log_data["character_id"]   = static_cast<Json::Int64>(this->_owner.id);
    log_data["character_name"] = UTF8(this->_owner.name(), PLATFORM::WINDOWS);
    log_data["entry_id"]       = static_cast<Json::Int64>(entry_id);
    log_data["title"]          = UTF8(box.title, PLATFORM::WINDOWS);
    auto attachments_array     = Json::Value(Json::arrayValue);
    for (const auto& attachment : box.attachments)
    {
        attachments_array.append(attachment.to_json());
    }
    log_data["attachments"] = attachments_array;
    this->_owner.server.log.write("storage_box_attachment_receive", log_data);
    co_return receive_result::RECEIVED;
}

const storage_box::entry_map& storage_box::entries() const
{
    this->_owner.assert_thread();
    return this->_entries;
}
