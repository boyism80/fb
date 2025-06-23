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

void context::foreach_ch(const clan& clan, const std::function<void(fb::game::character&)>& fn)
{
    auto names = std::vector<std::string>();
    for (auto& [name, _] : clan.members())
    {
        names.push_back(name);
    }

    this->foreach_ch(names, fn);
}

void context::upsert_clan_then(uint32_t id, std::function<void(shared_clan_lock&)> fn)
{
    this->_shard[id]->clans.write([this, id, fn](auto& clans) {
        if (clans.contains(id) == false)
        {
            clans.insert({id, std::make_shared<fb::locker<fb::game::clan>>(*this, id)});
            async::awaitable_then(this->http.get<internal_resp::GetClan>("internal", std::format("/clan/{}", id)),
                                  [this, id](auto result) {
                                      try
                                      {
                                          auto&& resp = result();
                                          switch (static_cast<ERROR_CODE>(resp.error))
                                          {
                                          case ERROR_CODE::NONE:
                                              break;

                                          default:
                                              throw std::runtime_error(
                                                  std::format("cannot get clan (error : {})", resp.error));
                                          }

                                          this->_shard[id]->clans.read([this, id, &resp](auto& clans) {
                                              if (!clans.contains(id))
                                                  return;

                                              auto& clan_lock_ptr = clans.at(id);
                                              clan_lock_ptr->write([this, &resp](auto& clan) {
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
        throw std::runtime_error(_TEXT(MESSAGE_ALREADY_JOINED_CLAN));

    auto   weak = me.weak_from_this_as<character>();
    auto   fd   = me.fd();
    auto&& resp = co_await this->http.post("internal", "/clan/create", CreateClan{me.id(), name});
    this->switch_thread(weak);

    this->assert_clan(resp.error);

    auto id = resp.clan.id;
    this->_shard[id]->clans.write([this, id = resp.clan.id, fd, &resp, weak](auto& clans) {
        auto id = resp.clan.id;
        if (clans.contains(id) == false)
        {
            auto clan_ptr   = new fb::game::clan(*this, id);
            auto shared_ptr = std::make_shared<fb::locker<fb::game::clan>>(*this, id);
            shared_ptr->write([this, &resp](auto& clan) {
                this->update_clan(clan, resp.clan, resp.members);
            });
            clans.insert({id, std::move(shared_ptr)});
        }

        auto& clan_lock_ptr = clans.at(id);
        if (weak.expired() == false)
        {
            auto me = weak.lock();
            me->clan(clan_lock_ptr);
            clan_lock_ptr->write([this, weak](auto& clan) {
                clan.attach_character(weak);
            });
        }
    });
}

async::task<void> context::destroy_clan(character& me)
{
    auto  weak      = me.weak_from_this_as<character>();
    auto& clan_lock = me.clan();
    if (clan_lock == nullptr)
        throw std::runtime_error(_TEXT(MESSAGE_NOT_JOINED_CLAN));

    auto clan_name = std::string{};
    auto clan_id   = uint32_t{};
    clan_lock->read([&clan_name, &clan_id](auto& clan) {
        clan_name = clan.name();
        clan_id   = clan.id();
    });

    auto   fd   = me.fd();
    auto&& resp = co_await this->http.post("internal", "/clan/destroy", DestroyClan{me.id()});
    this->switch_thread(weak);

    this->assert_clan(resp.error);
    this->_shard[clan_id]->clans.write([this, clan_id](auto& clans) {
        if (clans.contains(clan_id))
        {
            clans.at(clan_id)->read([this](auto& clan) {
                auto characters = std::vector<std::shared_ptr<character>>{};
                for (auto& [uid, ch] : clan.characters())
                    characters.push_back(ch.lock());

                for (auto& ch : characters)
                {
                    auto weak   = ch->weak_from_this_as<character>();
                    std::ignore = this->threads.dispatch(weak, [ch](auto&) -> async::task<void> {
                        ch->clan().reset();
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
    auto&& resp =
        co_await this->http.post("internal", "/clan/title", SetClanTitle{config<uint32_t>("host"), clan.id(), title});

    this->on_clan_title_changed(resp);
}

async::task<void> context::join_clan_member(const clan& clan, character& ch)
{
    auto&& resp =
        co_await this->http.post("internal", "/clan/join", JoinClan{config<uint32_t>("host"), clan.id(), ch.id()});

    this->on_clan_join_member(resp);
}

async::task<void> context::leave_clan_member(const clan& clan, const std::string& name, bool kick)
{
    auto&& resp =
        co_await this->http.post("internal", "/clan/leave", LeaveClan{config<uint32_t>("host"), clan.id(), name, kick});

    this->on_clan_leave_member(resp);
}

async::task<void> context::broadcast(const clan& clan, const std::string& message, MESSAGE_TYPE type)
{
    auto&& resp = co_await this->http.post(
        "internal",
        "/clan/broadcast",
        BroadcastClan{config<uint32_t>("host"), clan.id(), message, static_cast<uint8_t>(type)});

    this->on_clan_broadcast(resp);
}

void context::on_clan_title_changed(const internal_resp::SetClanTitle& resp)
{
    this->assert_clan(resp.error);

    this->upsert_clan_then(resp.clan, [&title = resp.title](auto& clan_lock) {
        clan_lock->write([&title](auto& clan) {
            clan.title(title);
        });
    });
}

void context::on_clan_join_member(const internal_resp::JoinClan& resp)
{
    this->assert_clan(resp.error);

    this->upsert_clan_then(resp.clan, [this, resp](auto& clan_lock) {
        auto ch = this->_shard[resp.member.name]->names.template read<std::shared_ptr<fb::game::character>>(
            [&resp](auto& ch_names) -> std::shared_ptr<fb::game::character> {
                if (ch_names.contains(resp.member.name) == false)
                    return nullptr;

                return ch_names.at(resp.member.name);
            });
        auto weak = ch->weak_from_this_as<character>();

        if (ch != nullptr)
        {
            std::ignore = this->threads.dispatch(weak, [this, resp, weak, &clan_lock](auto&) -> async::task<void> {
                clan_lock->write([=, this, &clan_lock](auto& clan) {
                    this->foreach_ch(clan, [resp](auto& ch) {
                        ch.message(std::format("{}님이 문파에 가입했습니다.", resp.member.name), MESSAGE_TYPE::NOTIFY);
                    });

                    auto cm = clan_member{resp.member.name, static_cast<CLAN_POSITION>(resp.member.position)};
                    clan.join(cm);
                    clan.attach_character(weak);

                    auto ch = weak.lock();
                    if (ch == nullptr)
                        return;

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
        auto ch = this->_shard[resp.uname]->names.template write<std::shared_ptr<fb::game::character>>(
            [&resp](auto& ch_names) -> std::shared_ptr<fb::game::character> {
                if (ch_names.contains(resp.uname) == false)
                    return nullptr;

                return ch_names.at(resp.uname);
            });

        if (ch != nullptr)
        {
            auto weak   = ch->weak_from_this_as<character>();
            std::ignore = this->threads.dispatch(weak, [this, weak, resp, &clan_lock](auto&) -> async::task<void> {
                clan_lock->write([this, weak, &resp](auto& clan) {
                    auto ch = weak.lock();
                    clan.leave(ch->name());
                    clan.detach_character(weak);

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
            clan_lock->read([this, &resp](auto& clan) {
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
        clan_lock->read([this, message, type](auto& clan) {
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