#include <fb/game/character.h>

#include <fb/game/server.h>
#include <fb/logger.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <macro.h>

#include <cstdint>
#include <exception>
#include <format>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace fb::game;

character::options::options(character& owner) :
    _owner(owner)
{ }

bool character::options::enabled(OPTION key) const
{
    this->_owner.assert_thread();

    auto opt = static_cast<uint8_t>(key);
    if (opt == 0 || opt > static_cast<uint8_t>(OPTION::VISIBLE_HELMET))
        throw std::runtime_error(std::format("invalid setting key : {:#x}", opt));

    return this->_values[opt];
}

void character::options::load(const fb::protocol::internal::Option& row)
{
    this->assign(OPTION::WHISPER, row.whisper, false);
    this->assign(OPTION::GROUP, row.group, false);
    this->assign(OPTION::ROAR, row.roar, false);
    this->assign(OPTION::NEWS, row.news, false);
    this->assign(OPTION::MAGIC_EFFECT, row.magic_effect, false);
    this->assign(OPTION::WEATHER_EFFECT, row.weather_effect, false);
    this->assign(OPTION::FIXED_MOVE, row.fixed_move, false);
    this->assign(OPTION::TRADE, row.trade, false);
    this->assign(OPTION::FAST_MOVE, row.fast_move, false);
    this->assign(OPTION::EFFECT_SOUND, row.effect_sound, false);
    this->assign(OPTION::PK_PROTECT, row.pk_protect, false);
    this->assign(OPTION::VISIBLE_HELMET, row.visible_helmet, false);
}

async::task<bool> character::options::change(std::vector<std::pair<OPTION, bool>> changes)
{
    this->_owner.assert_thread();

    auto weak    = this->_owner.weak_from_this_as<character>();
    auto world   = this->_owner.world();
    auto uid     = this->_owner.id;
    auto stored  = std::vector<internal::OptionChange>{};
    auto applied = std::vector<std::pair<OPTION, bool>>{};
    for (auto& [key, value] : changes)
    {
        auto opt = static_cast<uint8_t>(key);
        if (opt == 0 || opt > static_cast<uint8_t>(OPTION::VISIBLE_HELMET))
            continue;

        if (key == OPTION::GROUP && value == false && this->_owner.group.id().has_value())
        {
            auto left = true;
            try
            {
                left = co_await this->_owner.server.groups.handle_action(this->_owner, this->_owner.name());
            }
            catch (std::exception& e)
            {
                fb::logger::warn("options: groups.handle_action failed: {}", e.what());
            }

            auto self = weak.lock();
            if (self == nullptr)
                co_return false;

            if (left == false)
            {
                self->message(_TEXT(MESSAGE_OPTION_UPDATE_FAILED));
                self->update_option();
                continue;
            }
        }

        stored.emplace_back(opt, value);
        applied.emplace_back(key, value);
    }

    if (stored.empty())
        co_return false;

    auto&& resp = co_await this->_owner.server.http.post(weak,
                                                         "internal",
                                                         "/in-game/option",
                                                         internal_reqs::SetOption{world, uid, std::move(stored)});
    auto   self = weak.lock();
    if (self == nullptr)
        co_return false;

    if (resp.success)
    {
        for (auto& [key, value] : applied)
        {
            this->assign(key, value, true);
        }
        co_return true;
    }
    else
    {
        self->message(_TEXT(MESSAGE_OPTION_UPDATE_FAILED));
        self->update_option();
        co_return false;
    }
}

void character::options::assign(OPTION key, bool value, bool notify)
{
    this->_owner.assert_thread();

    auto opt = static_cast<uint8_t>(key);
    if (opt == 0 || opt > static_cast<uint8_t>(OPTION::VISIBLE_HELMET))
        return;

    if (this->_values[opt] == value)
        return;

    this->_values[opt] = value;

    if (key == OPTION::FIXED_MOVE)
    {
        if (value)
            this->_owner._camera_pivot.reset();
        else
            this->_owner.camera_pivot();
    }

    if (key == OPTION::VISIBLE_HELMET)
        this->_owner.show();

    this->_owner.update(UPDATE_STATE_LEVEL::EXP_MONEY | UPDATE_STATE_LEVEL::CROWD_CONTROL);
    this->_owner.update_option();

    if (notify)
    {
        this->_owner.listener.on_option_changed(this->_owner, key, value);
        if (key == OPTION::VISIBLE_HELMET && this->_owner.client_version == fb::protocol::CLIENT_VERSION::v651)
        {
            this->_owner.update_external();
            this->_owner.group.refresh_portrait();
        }
    }
}
