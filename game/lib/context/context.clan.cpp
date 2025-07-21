#include <fb/game/context.h>

using namespace fb::game;

void context::assert_clan(uint32_t error) const
{
    switch (static_cast<ERROR_CODE>(error))
    {
    case ERROR_CODE::NONE:
        return;

    case ERROR_CODE::CLAN_NAME_ALREADY_EXISTS:
        throw std::runtime_error(std::format("클랜명이 이미 존재함"));

    default:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), error));
    }
}

async::task<void> context::upsert_clan_then(uint32_t                                                           id,
                                            std::function<async::task<void>(std::shared_ptr<fb::game::clan>&)> fn)
{
    co_await this->clans.async_write(
        id,
        [this, id, fn](auto& clan) -> async::task<void> {
            co_await fn(clan);
        },
        [=, this]() -> async::task<std::shared_ptr<fb::game::clan>> {
            auto&& resp = co_await this->http.get<internal_resp::GetClan>("internal", std::format("/clan/{}", id));
            switch (static_cast<ERROR_CODE>(resp.error))
            {
            case ERROR_CODE::NONE:
                co_return std::make_shared<fb::game::clan>(*this, id, resp.clan.name, resp.clan.title);

            default:
                throw std::runtime_error(std::format("cannot get clan (error : {})", resp.error));
            }
        });
}

void context::update_clan(clan&                                                  clan,
                          fb::protocol::internal::Clan&                          resp1,
                          const std::vector<fb::protocol::internal::ClanMember>& resp2) const
{
    auto members = std::vector<clan_member>{};
    for (auto& member : resp2)
    {
        auto cm = clan_member{member.name, static_cast<CLAN_POSITION>(member.position)};
        members.push_back(std::move(cm));
    }

    clan.update(resp1.name, resp1.title, members);
}

async::task<void> context::create_clan(character& me, std::string name)
{
    if (me.clan_id().has_value())
        throw std::runtime_error(_TEXT(MESSAGE_ALREADY_JOINED_CLAN));

    auto   weak = me.weak_from_this_as<character>();
    auto&& resp = co_await this->http.post("internal", "/clan/create", CreateClan{me.id(), name});
    co_await this->threads.switching(weak);

    this->assert_clan(resp.error);

    auto id = resp.clan.id;
    this->clans.write(
        id,
        [=, &resp, this](auto& clan) {
            this->update_clan(*clan.get(), resp.clan, resp.members);

            if (weak.expired() == false)
            {
                auto me = weak.lock();
                me->clan_id(id);
                clan->attach_character(weak);
            }
        },
        [=, this]() -> std::shared_ptr<fb::game::clan> {
            return std::make_shared<fb::game::clan>(*this, id, name, std::nullopt);
        });
}

async::task<void> context::destroy_clan(character& me)
{
    auto weak    = me.weak_from_this_as<character>();
    auto clan_id = me.clan_id(); // don't use auto& because it will be invalidated after the clan is destroyed
    if (clan_id.has_value() == false)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    auto destroyed = co_await this->clans.async_write(clan_id.value(), [this, weak](auto& clan) -> async::task<bool> {
        auto me = weak.lock();
        if (me == nullptr)
            co_return false;

        auto members = std::vector<std::shared_ptr<fb::game::character>>();
        for (auto& [_, weak_ptr] : clan->characters())
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                continue;

            members.push_back(shared_ptr);
        }

        auto&& resp = co_await this->http.post("internal", "/clan/destroy", DestroyClan{me->id()});
        co_await this->threads.switching(weak);

        this->assert_clan(resp.error);
        auto message = std::format("{} 문파가 해체되었습니다.", clan->name());
        co_await this->characters.foreach_async(
            [this, message](auto& ch) -> async::task<void> {
                ch->message(message, MESSAGE_TYPE::NOTIFY);
                ch->clan_reset();
                co_return;
            },
            members);
        co_return true;
    });

    if (destroyed)
        this->clans.erase(clan_id.value());
}

async::task<void> context::set_clan_title(const clan& clan, std::string title)
{
    auto   name = std::string{clan.name()};
    auto&& resp =
        co_await this->http.post("internal", "/clan/title", SetClanTitle{config<uint32_t>("host"), clan.id(), title});

    co_await this->on_clan_title_changed(resp);
}

async::task<void> context::join_clan_member(const clan& clan, character& ch)
{
    auto&& resp =
        co_await this->http.post("internal", "/clan/join", JoinClan{config<uint32_t>("host"), clan.id(), ch.id()});

    co_await this->on_clan_join_member(resp);
}

async::task<void> context::leave_clan_member(const clan& clan, const std::string& name)
{
    auto&& resp =
        co_await this->http.post("internal", "/clan/leave", LeaveClan{config<uint32_t>("host"), clan.id(), name});

    co_await this->on_clan_leave_member(resp);
}

async::task<void> context::kick_clan_member(const clan& clan, const std::string& kicker, const std::string& target)
{
    auto&& resp = co_await this->http.post("internal",
                                           "/clan/kick",
                                           KickClan{config<uint32_t>("host"), clan.id(), kicker, target});

    co_await this->on_clan_kick_member(resp);
}

async::task<void> context::broadcast(const clan& clan, const std::string& message, MESSAGE_TYPE type)
{
    auto&& resp = co_await this->http.post(
        "internal",
        "/clan/broadcast",
        BroadcastClan{config<uint32_t>("host"), clan.id(), message, static_cast<uint8_t>(type)});

    co_await this->on_clan_broadcast(resp);
}

async::task<void> context::on_clan_title_changed(const internal_resp::SetClanTitle& resp)
{
    this->assert_clan(resp.error);

    co_await this->upsert_clan_then(resp.clan, [&title = resp.title](auto& clan) -> async::task<void> {
        clan->title(title);
        co_return;
    });
}

async::task<void> context::on_clan_join_member(const internal_resp::JoinClan& resp)
{
    this->assert_clan(resp.error);

    co_await this->upsert_clan_then(resp.clan, [this, resp](auto& clan) -> async::task<void> {
        auto ch = this->characters.find(resp.member.name);
        if (ch == nullptr)
            co_return;

        if (ch->clan_id().has_value())
            co_return;

        auto weak    = ch->weak_from_this_as<character>();
        auto members = std::vector<std::shared_ptr<fb::game::character>>();
        for (auto& [_, weak_ptr] : clan->characters())
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                continue;

            members.push_back(shared_ptr);
        }

        co_await this->characters.foreach (
            [this, resp](auto& member) {
                member->message(std::format("{}님이 문파에 가입했습니다.", resp.member.name), MESSAGE_TYPE::NOTIFY);
            },
            members);

        auto cm = clan_member{resp.member.name, static_cast<CLAN_POSITION>(resp.member.position)};
        clan->join(cm);

        if (weak.expired() == false)
        {
            clan->attach_character(weak);
            ch->clan_id(clan->id());
            ch->update_external(false);
            ch->message(std::format("{} 문파에 가입되었습니다.", clan->name()), MESSAGE_TYPE::NOTIFY);
        }
    });
}

async::task<void> context::on_clan_leave_member(const internal_resp::LeaveClan& resp)
{
    this->assert_clan(resp.error);
    co_await this->upsert_clan_then(resp.clan, [this, resp](auto& clan) -> async::task<void> {
        auto ch = this->characters.find(resp.uname);
        if (ch != nullptr)
        {
            auto weak = ch->weak_from_this_as<character>();
            clan->detach(weak);
            ch->clan_reset();
            ch->update_external(false);
            ch->message("문파에서 탈퇴했습니다.", MESSAGE_TYPE::NOTIFY);
        }

        clan->leave(resp.uname);
        auto members = std::vector<std::shared_ptr<fb::game::character>>();
        for (auto& [_, weak_ptr] : clan->characters())
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                continue;

            members.push_back(shared_ptr);
        }

        auto message = std::format("{}님이 문파에서 탈퇴하였습니다.", resp.uname);
        co_await this->characters.foreach (
            [this, message](auto& member) {
                member->message(message, MESSAGE_TYPE::NOTIFY);
            },
            members);

        clan->leave(resp.uname);
        co_return;
    });
}

async::task<void> context::on_clan_kick_member(const internal_resp::KickClan& resp)
{
    this->assert_clan(resp.error);
    co_await this->upsert_clan_then(resp.clan, [this, resp](auto& clan) -> async::task<void> {
        auto ch = this->characters.find(resp.uname);
        if (ch != nullptr)
        {
            auto weak = ch->weak_from_this_as<character>();
            clan->detach(weak);
            ch->clan_reset();
            ch->update_external(false);
            ch->message("문파에서 추방당했습니다.", MESSAGE_TYPE::NOTIFY);
        }

        clan->leave(resp.uname);
        auto members = std::vector<std::shared_ptr<fb::game::character>>();
        for (auto& [_, weak_ptr] : clan->characters())
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                continue;

            members.push_back(shared_ptr);
        }

        auto message = std::format("{}님이 문파에서 추방당했습니다.", resp.uname);
        co_await this->characters.foreach (
            [this, message](auto& member) {
                member->message(message, MESSAGE_TYPE::NOTIFY);
            },
            members);

        clan->leave(resp.uname);
        co_return;
    });
}

async::task<void> context::on_clan_broadcast(const internal_resp::BroadcastClan& resp)
{
    this->assert_clan(resp.error);

    co_await this->upsert_clan_then(resp.clan, [this, resp](auto& clan) -> async::task<void> {
        auto members = std::vector<std::shared_ptr<fb::game::character>>();
        for (auto& [_, weak_ptr] : clan->characters())
        {
            auto shared_ptr = weak_ptr.lock();
            if (shared_ptr == nullptr)
                continue;

            members.push_back(shared_ptr);
        }

        co_await this->characters.foreach (
            [this, resp](auto& member) {
                member->message(resp.message, static_cast<MESSAGE_TYPE>(resp.type));
            },
            members);
    });
}