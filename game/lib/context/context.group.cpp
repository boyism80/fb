#include <context.h>

using namespace fb::game;

void context::upsert_group_then(uint32_t gid, const std::function<void(shared_group_lock&)>& fn)
{
    this->_shard[gid]->groups.lock([this, gid, &fn](auto& groups) {
        if (groups.contains(gid) == false)
        {
            groups.insert({gid, std::make_shared<fb::locker<fb::game::group>>(*this, gid)});
            async::awaitable_then(this->get<internal_resp::GetGroup>("internal", std::format("/group/{}", gid)),
                                  [this, gid](auto result) {
                                      try
                                      {
                                          auto&& resp = result();
                                          switch (static_cast<ERROR_CODE>(resp.error))
                                          {
                                          case ERROR_CODE::NONE:
                                              break;

                                          default:
                                              throw std::runtime_error(
                                                  std::format("cannot get group (error : {})", resp.error));
                                          }

                                          this->_shard[gid]->groups.lock([this, gid, &resp](auto& groups) {
                                              if (!groups.contains(gid))
                                                  return;

                                              auto& group_lock_ptr = groups.at(gid);
                                              group_lock_ptr->lock([&resp](auto& group) {
                                                  group.update(resp.group.master, resp.group.members);
                                              });
                                          });
                                      }
                                      catch (std::exception& e)
                                      {
                                          fb::logger::fatal(e.what());
                                      }
                                  });
        }

        fn(groups.at(gid));
    });
}

void context::upsert_group_then(uint32_t                                       gid,
                                const std::string&                             master,
                                const std::vector<std::string>&                members,
                                const std::function<void(shared_group_lock&)>& fn)
{
    this->_shard[gid]->groups.lock([this, gid, &fn, &master, &members](auto& groups) {
        if (groups.contains(gid) == false)
        {
            groups.insert({gid, std::make_shared<fb::locker<fb::game::group>>(*this, gid)});
        }

        auto& group_lock_ptr = groups.at(gid);
        group_lock_ptr->lock([&master, &members](auto& group) {
            group.update(master, members);
        });
        fn(group_lock_ptr);
    });
}


async::task<bool> context::create_group(character& me, const std::string& target)
{
    try
    {
        if (me.option(SETTING::GROUP) == false)
            throw std::runtime_error(message::group::DISABLED_MINE);

        auto&& resp = co_await this->post<internal_reqs::EnterGroup, internal_resp::EnterGroup>(
            "internal",
            "/group/create",
            internal_reqs::EnterGroup{me.id(), target});

        this->on_enter_group(resp);
        co_return true;
    }
    catch (std::exception& e)
    {
        me.message(e.what(), MESSAGE_TYPE::STATE);
        co_return false;
    }
}

void context::assert_group(uint32_t error, const std::string& actor) const
{
    switch (static_cast<ERROR_CODE>(error))
    {
    case ERROR_CODE::NONE:
        return;

    case ERROR_CODE::CANNOT_GROUP_SELF:
        throw std::runtime_error("자기 자신과는 그룹할 수 없습니다.");

    case ERROR_CODE::GROUP_ALREADY_JOINED:
        throw std::runtime_error("이미 그룹에 참여중입니다.");

    case ERROR_CODE::OFFLINE:
        throw std::runtime_error(std::format("{}님은 바람의나라에 없습니다.", actor));

    case ERROR_CODE::GROUP_TARGET_ALREADY_JOINED:
        throw std::runtime_error(std::format("{}님은 이미 그룹에 참여중입니다.", actor));

    case ERROR_CODE::DISABLED_GROUP:
        throw std::runtime_error("그룹 참여 거부중입니다.");

    case ERROR_CODE::DISABLED_GROUP_TARGET:
        throw std::runtime_error(std::format("{}님은 그룹 참여 거부중입니다.", actor));

    case ERROR_CODE::NOT_GROUP_MASTER:
        throw std::runtime_error("당신은 그룹장이 아닙니다.");

    default:
        throw std::runtime_error(std::format("알 수 없는 에러가 발생했습니다. (에러코드 : {})", error));
    }
}

void context::on_enter_group(internal_resp::EnterGroup resp)
{
    this->assert_group(resp.error, resp.member);

    auto gid = resp.group.id;
    this->upsert_group_then(gid, resp.group.master, resp.group.members, [this, &resp](auto& group_lock_ptr) {
        auto members = std::vector<std::string>{resp.group.members};
        members.push_back(resp.group.master);
        if (resp.action == GroupAction::Kick)
            members.push_back(resp.member);

        this->foreach_ch(members, [action = resp.action, member = resp.member, &group_lock_ptr](auto& character) {
            switch (action)
            {
            case GroupAction::Create:
                group_lock_ptr->lock([&character](auto& group) {
                    group.enter(character);
                });
                character.group(group_lock_ptr);
                if (character.name() == member)
                {
                    character.message("그룹에 참여했습니다.");
                }
                else
                {
                    character.message(std::format("{}님 그룹 참여", member));
                }
                break;

            case GroupAction::Enter:
                if (character.name() == member)
                {
                    character.message("그룹에 참여했습니다.");
                    group_lock_ptr->lock([&character](auto& group) {
                        group.enter(character);
                    });
                    character.group(group_lock_ptr);
                }
                else
                {
                    character.message(std::format("{}님 그룹 참여", member));
                }
                break;

            case GroupAction::Kick:
                if (character.name() == member)
                {
                    group_lock_ptr->lock([&character](auto& group) {
                        group.leave(character);
                    });
                    character.group().reset();
                    character.message("그룹에서 추방당했습니다.");
                }
                else
                {
                    character.message(std::format("{}님 그룹 탈퇴", member));
                }
                break;
            }
        });
    });
}

void context::on_leave_group(const internal_resp::LeaveGroup& resp)
{
    this->assert_group(resp.error, resp.member);

    auto gid = resp.group.id;
    switch (resp.action)
    {
    case GroupAction::Leave:
    {
        this->upsert_group_then(gid, resp.group.master, resp.group.members, [this, &resp, gid](auto& group_lock_ptr) {
            this->foreach_ch(resp.member, [&group_lock_ptr](auto& ch) {
                ch.group().reset();

                group_lock_ptr->lock([&ch](auto& group) {
                    group.leave(ch);
                });
            });

            auto members = std::vector<std::string>{resp.member};
            members.push_back(resp.group.master);
            this->foreach_ch(members, [member = resp.member](auto& ch) {
                if (ch.name() == member)
                    ch.message("그룹 탈퇴", MESSAGE_TYPE::STATE);
                else
                    ch.message(std::format("{}님 그룹에서 탈퇴", member), MESSAGE_TYPE::STATE);
            });
        });
    }
    break;

    case GroupAction::BreakUp:
    {
        this->_shard[gid]->groups.lock([this, gid, &resp](auto& groups) {
            auto members = std::vector<std::string>{resp.group.members};
            members.push_back(resp.group.master);

            this->foreach_ch(members, [](auto& ch) {
                ch.group().reset();
                ch.message("그룹 해체", MESSAGE_TYPE::STATE);
            });

            groups.erase(gid);
        });
    }
    break;
    }
}