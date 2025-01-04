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
        throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR), error));
    }
}

void context::foreach_ch(const clan& clan, const std::function<void(fb::game::character&)>& fn)
{
    auto names = std::vector<std::string>();
    for (auto& [name, _] : clan.members())
    {
        names.push_back(name);
    }

    this->foreach_ch(names, fn);
}

void context::upsert_clan_then(uint32_t id, const std::function<void(shared_clan_lock&)>& fn)
{
    this->_shard[id]->clans.lock([this, id, fn](auto& clans) {
        if (clans.contains(id) == false)
        {
            clans.insert({id, std::make_shared<fb::locker<fb::game::clan>>(*this, id)});
            async::awaitable_then(
                this->get<internal_resp::GetClan>("internal", std::format("/clan/{}", id)),
                [this, id](auto result) {
                    try
                    {
                        auto&& resp = result();
                        switch (static_cast<ERROR_CODE>(resp.error))
                        {
                        case ERROR_CODE::NONE:
                            break;

                        default:
                            throw std::runtime_error(std::format("cannot get clan (error : {})", resp.error));
                        }

                        this->_shard[id]->clans.lock([this, id, &resp](fb::game::shard_params::clan_container& clans) {
                            if (!clans.contains(id))
                                return;

                            auto& clan_lock_ptr = clans.at(id);
                            clan_lock_ptr->lock([this, &resp](fb::game::clan& clan) {
                                this->update_clan(clan, resp.clan, resp.members);
                            });
                        });
                    }
                    catch (std::exception& e)
                    {
                        fb::logger::fatal(e.what());
                    }
                });
        }

        fn(clans.at(id));
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

async::task<void> context::create_clan(character& me, const std::string& name)
{
    if (me.clan() != nullptr)
        throw std::runtime_error("클랜 이미 있음");

    auto   fd   = me.fd();
    auto&& resp = co_await this->post<internal_reqs::CreateClan, internal_resp::CreateClan>(
        "internal",
        "/clan/create",
        internal_reqs::CreateClan{me.id(), name});

    this->assert_clan(resp.error);

    auto id = resp.clan.id;
    this->_shard[id]->clans.lock([this, id = resp.clan.id, fd, &resp, &me](auto& clans) {
        auto id = resp.clan.id;
        if (clans.contains(id) == false)
        {
            auto clan_ptr   = new fb::game::clan(*this, id);
            auto shared_ptr = std::make_shared<fb::locker<fb::game::clan>>(*this, id);
            shared_ptr->lock([this, &resp](auto& clan) {
                this->update_clan(clan, resp.clan, resp.members);
            });
            clans.insert({id, std::move(shared_ptr)});
        }

        this->_shard[id]->clans.lock([this, id, fd, &me](fb::game::shard_params::clan_container& clans) {
            if (!clans.contains(id))
                return;

            auto& clan_lock_ptr = clans.at(id);

            if (this->assert_socket(fd))
            {
                me.clan(clan_lock_ptr);
                clan_lock_ptr->lock([this, &me](fb::game::clan& clan) {
                    clan.attach_character(me);
                });
            }
        });
    });
}

async::task<void> context::destroy_clan(character& me)
{
    auto& clan_lock = me.clan();
    if (clan_lock == nullptr)
        throw std::runtime_error("클랜이 없음");

    auto clan_name = std::string{};
    auto clan_id   = uint32_t{};
    clan_lock->lock([&clan_name, &clan_id](auto& clan) {
        clan_name = clan.name();
        clan_id   = clan.id();
    });

    auto   fd   = me.fd();
    auto&& resp = co_await this->post<internal_reqs::DestroyClan, internal_resp::DestroyClan>(
        "internal",
        "/clan/destroy",
        internal_reqs::DestroyClan{me.id()});

    this->assert_clan(resp.error);
    this->_shard[clan_id]->clans.lock([clan_id](auto& clans) {
        if (clans.contains(clan_id))
        {
            clans.at(clan_id)->lock([](auto& clan) {
                auto character_set = std::unordered_map<fb::thread*, std::vector<character*>>{};
                for (auto& [uid, ch] : clan.characters())
                {
                    auto thread = ch->thread();
                    if (!character_set.contains(thread))
                        character_set.insert({thread, {}});

                    character_set.at(thread).push_back(ch);
                }

                for (auto& [thread, characters] : character_set)
                {
                    std::ignore = thread->dispatch([characters](auto&) -> async::task<void> {
                        for (auto ch : characters)
                        {
                            ch->clan().reset();
                        }

                        co_return;
                    });
                }
            });

            clans.erase(clan_id);
        }
    });
}

async::task<void> context::set_clan_title(const clan& clan, std::string title)
{
    auto   name = std::string{clan.name()};
    auto&& resp = co_await this->post<internal_reqs::SetClanTitle, internal_resp::SetClanTitle>(
        "internal",
        "/clan/title",
        internal_reqs::SetClanTitle{config<uint32_t>("host"), clan.id(), title});

    this->on_clan_title_changed(resp);
}

async::task<void> context::join_clan_member(const clan& clan, character& ch)
{
    auto&& resp = co_await this->post<internal_reqs::JoinClan, internal_resp::JoinClan>(
        "internal",
        "/clan/join",
        internal_reqs::JoinClan{config<uint32_t>("host"), clan.id(), ch.id()});

    this->on_clan_join_member(resp);
}

async::task<void> context::leave_clan_member(const clan& clan, const std::string& name, bool kick)
{
    auto&& resp = co_await this->post<internal_reqs::LeaveClan, internal_resp::LeaveClan>(
        "internal",
        "/clan/leave",
        internal_reqs::LeaveClan{config<uint32_t>("host"), clan.id(), name, kick});

    this->on_clan_leave_member(resp);
}

async::task<void> context::broadcast(const clan& clan, const std::string& message, MESSAGE_TYPE type)
{
    auto&& resp = co_await this->post<internal_reqs::BroadcastClan, internal_resp::BroadcastClan>(
        "internal",
        "/clan/broadcast",
        internal_reqs::BroadcastClan{config<uint32_t>("host"), clan.id(), message, static_cast<uint8_t>(type)});

    this->on_clan_broadcast(resp);
}

void context::on_clan_title_changed(const internal_resp::SetClanTitle& resp)
{
    this->assert_clan(resp.error);

    this->upsert_clan_then(resp.clan, [&title = resp.title](auto& clan_lock) {
        clan_lock->lock([&title](auto& clan) {
            clan.title(title);
        });
    });
}

void context::on_clan_join_member(const internal_resp::JoinClan& resp)
{
    this->assert_clan(resp.error);

    this->upsert_clan_then(resp.clan, [this, resp](auto& clan_lock) {
        auto ch =
            this->_shard[resp.member.name]->names.template lock<character*>([&resp](auto& ch_names) -> character* {
                if (ch_names.contains(resp.member.name) == false)
                    return nullptr;

                return ch_names.at(resp.member.name);
            });

        if (ch != nullptr)
        {
            auto thread = ch->thread();
            std::ignore = thread->dispatch([=, this, &clan_lock](auto&) -> async::task<void> {
                clan_lock->lock([=, this, &clan_lock](auto& clan) {
                    this->foreach_ch(clan, [resp](auto& ch) {
                        ch.message(std::format("{}님이 문파에 가입했습니다.", resp.member.name), MESSAGE_TYPE::NOTIFY);
                    });

                    auto cm = clan_member{resp.member.name, static_cast<CLAN_POSITION>(resp.member.position)};
                    clan.join(cm);
                    clan.attach_character(*ch);
                    ch->clan(clan_lock);
                    ch->update_external(true);
                    ch->message(std::format("{} 문파에 가입되었습니다.", clan.name()), MESSAGE_TYPE::NOTIFY);
                });
                co_return;
            });
        }
    });
}

void context::on_clan_leave_member(const internal_resp::LeaveClan& resp)
{
    this->assert_clan(resp.error);

    this->upsert_clan_then(resp.clan, [this, resp](auto& clan_lock) {
        auto ch = this->_shard[resp.uname]->names.template lock<character*>([&resp](auto& ch_names) -> character* {
            if (ch_names.contains(resp.uname) == false)
                return nullptr;

            return ch_names.at(resp.uname);
        });

        if (ch != nullptr)
        {
            std::ignore = ch->thread()->dispatch([this, ch, resp, &clan_lock](auto&) -> async::task<void> {
                clan_lock->lock([this, ch, &resp](auto& clan) {
                    clan.leave(ch->name());
                    clan.detach_character(*ch);
                    ch->clan().reset();
                    ch->update_external(true);

                    if (resp.kick)
                    {
                        ch->message("문파에서 추방당했습니다.", MESSAGE_TYPE::NOTIFY);
                        this->foreach_ch(clan, [name = resp.uname](auto& ch) {
                            ch.message(std::format("{}님이 문파에서 추방당했습니다.", name), MESSAGE_TYPE::NOTIFY);
                        });
                    }
                    else
                    {
                        ch->message("문파에서 탈퇴했습니다.", MESSAGE_TYPE::NOTIFY);
                        this->foreach_ch(clan, [name = resp.uname](auto& ch) {
                            ch.message(std::format("{}님이 문파에서 탈퇴하였습니다.", name), MESSAGE_TYPE::NOTIFY);
                        });
                    }
                });
                co_return;
            });
        }
        else
        {
            clan_lock->lock([this, &resp](auto& clan) {
                if (resp.kick)
                    this->foreach_ch(clan, [name = resp.uname](auto& ch) {
                        ch.message(std::format("{}님이 문파에서 추방당했습니다.", name), MESSAGE_TYPE::NOTIFY);
                    });
                else
                    this->foreach_ch(clan, [name = resp.uname](auto& ch) {
                        ch.message(std::format("{}님이 문파에서 탈퇴하였습니다.", name), MESSAGE_TYPE::NOTIFY);
                    });
            });
        }
    });
}

void context::on_clan_broadcast(const internal_resp::BroadcastClan& resp)
{

    this->assert_clan(resp.error);

    this->upsert_clan_then(resp.clan, [this, message = resp.message, type = resp.type](auto& clan_lock) {
        clan_lock->lock([this, message, type](auto& clan) {
            auto names = std::vector<std::string>{};
            for (auto& [name, member] : clan.members())
            {
                names.push_back(name);
            }

            this->foreach_ch(names, [message, type](auto& ch) {
                ch.message(message, static_cast<MESSAGE_TYPE>(type));
            });
        });
    });
}