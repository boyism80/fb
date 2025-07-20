#include <fb/game/context.h>
#include <fb/game/group.h>
#include <fb/game/character.h>

using namespace fb::game;

async::task<void> context::upsert_group_then(uint32_t gid, const std::function<void(group_ptr&)>& fn)
{
    co_await this->groups.async_write(
        gid,
        [this, gid, fn](auto& group) -> async::task<void> {
            // Execute callback with existing group
            fn(group);
            co_return;
        },
        [this, gid]() -> async::task<std::shared_ptr<fb::game::group>> {
            // Fetch group data from internal server when not found locally
            auto&& resp = co_await this->http.get<internal_resp::GetGroup>("internal", std::format("/group/{}", gid));
            switch (static_cast<ERROR_CODE>(resp.error))
            {
            case ERROR_CODE::NONE:
                co_return this->make<fb::game::group>(gid, resp.group.master, resp.group.members);

            default:
                throw std::runtime_error(std::format("cannot get group (error : {})", resp.error));
            }
        });
}

async::task<void> context::upsert_group_then(uint32_t                               gid,
                                             const std::string&                     master,
                                             const std::vector<std::string>&        members,
                                             const std::function<void(group_ptr&)>& fn)
{
    co_await this->groups.async_write(
        gid,
        [this, fn, master, members](auto& group) -> async::task<void> {
            // Update existing group with new member data
            group->update(master, members);
            fn(group);
            co_return;
        },
        [this, gid, master, members]() -> std::shared_ptr<fb::game::group> {
            // Create new group with provided data
            return this->make<fb::game::group>(gid, master, members);
        });
}

async::task<bool> context::create_group(character& me, const std::string& target)
{
    auto weak = me.weak_from_this_as<fb::game::character>();
    try
    {
        if (me.option(OPTION::GROUP) == false)
            throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_MINE));

        auto&& resp = co_await this->http.post("internal", "/group/create", EnterGroup{me.id(), target});
        co_await this->threads.switching(weak);

        co_await this->on_enter_group(resp);
        co_return true;
    }
    catch (std::exception& e)
    {
        me.message(e.what(), MESSAGE_TYPE::STATE);
        co_return false;
    }
}

async::task<void> context::leave_group(character& me)
{
    auto group_id = me.group_id();
    if (group_id.has_value() == false)
        co_return;

    auto   weak     = me.weak_from_this_as<fb::game::character>();
    auto&& response = co_await this->http.post("internal", "/group/leave", LeaveGroup{me.name()});
    co_await this->threads.switching(weak);
    co_await this->on_leave_group(response);
}

void context::assert_group(uint32_t error, const std::string& actor) const
{
    // Convert error codes to localized error messages
    switch (static_cast<ERROR_CODE>(error))
    {
    case ERROR_CODE::NONE:
        return;

    case ERROR_CODE::CANNOT_GROUP_SELF:
        throw std::runtime_error(_TEXT(MESSAGE_CANNOT_GROUP_SELF));

    case ERROR_CODE::GROUP_ALREADY_JOINED:
        throw std::runtime_error(_TEXT(MESSAGE_ALREADY_JOINED_GROUP));

    case ERROR_CODE::OFFLINE:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_USER_NOT_LOGIN), actor));

    case ERROR_CODE::GROUP_TARGET_ALREADY_JOINED:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_GROUP_ALREADY_JOINED), actor));

    case ERROR_CODE::DISABLED_GROUP:
        throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_MINE));

    case ERROR_CODE::DISABLED_GROUP_TARGET:
        throw std::runtime_error(_TEXT(MESSAGE_GROUP_DISABLED_TARGET));

    case ERROR_CODE::NOT_GROUP_MASTER:
        throw std::runtime_error(_TEXT(MESSAGE_GROUP_NOT_OWNER));

    default:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), error));
    }
}

async::task<void> context::on_enter_group(const internal_resp::EnterGroup& resp)
{
    this->assert_group(resp.error, resp.member);

    auto gid = resp.group.id;
    co_await this->upsert_group_then(gid, resp.group.master, resp.group.members, [this, &resp](auto& group) {
        // Build complete member list including master
        auto members = std::vector<std::string>{resp.group.members};
        members.push_back(resp.group.master);

        if (resp.action == GroupAction::Kick)
            members.push_back(resp.member);

        // Process group action for all affected members
        this->characters.foreach (members, [resp, group](auto& ch) {
            switch (resp.action)
            {
            case GroupAction::Create:
                group->enter(ch);
                ch->group_id(group->id());
                if (ch->name() == resp.member)
                {
                    ch->message("그룹에 참여했습니다.");
                }
                else
                {
                    ch->message(std::format("{}님 그룹 참여", resp.member));
                }
                break;

            case GroupAction::Enter:
                if (ch->name() == resp.member)
                {
                    ch->message("그룹에 참여했습니다.");
                    group->enter(ch);
                    ch->group_id(group->id());
                }
                else
                {
                    ch->message(std::format("{}님 그룹 참여", resp.member));
                }
                break;

            case GroupAction::Kick:
                if (ch->name() == resp.member)
                {
                    group->detach(ch);
                    ch->group_reset();
                    ch->message("그룹에서 추방당했습니다.");
                }
                else
                {
                    ch->message(std::format("{}님 그룹 탈퇴", resp.member));
                }
                break;
            }
        });
    });
}

async::task<void> context::on_leave_group(const internal_resp::LeaveGroup& resp)
{
    this->assert_group(resp.error, resp.member);

    auto gid = resp.group.id;
    switch (resp.action)
    {
    case GroupAction::Leave:
    {
        co_await this->upsert_group_then(gid,
                                         resp.group.master,
                                         resp.group.members,
                                         [this, &resp, gid](auto& group) -> async::task<void> {
                                             co_await this->characters.invoke(resp.member, [group](auto& ch) {
                                                 ch->group_reset();
                                                 group->detach(ch);
                                             });

                                             auto members = std::vector<std::string>{resp.member};
                                             members.push_back(resp.group.master);
                                             this->characters.foreach (members, [member = resp.member](auto& ch) {
                                                 if (ch->name() == member)
                                                     ch->message("그룹 탈퇴", MESSAGE_TYPE::STATE);
                                                 else
                                                     ch->message(std::format("{}님 그룹에서 탈퇴", member),
                                                                 MESSAGE_TYPE::STATE);
                                             });
                                         });
    }
    break;

    case GroupAction::BreakUp:
    {
        co_await this->groups.async_write(gid, [this, gid, &resp](auto& groups) -> async::task<void> {
            auto members = std::vector<std::string>{resp.group.members};
            members.push_back(resp.group.master);

            co_await this->characters.foreach (members, [](auto& ch) {
                ch->group_reset();
                ch->message("그룹 해체", MESSAGE_TYPE::STATE);
            });
        });
        this->groups.erase(gid);
    }
    break;
    }
}

async::task<void> context::broadcast(const group& group, const std::string& message, MESSAGE_TYPE type)
{
    auto&& resp = co_await this->http.post(
        "internal",
        "/group/broadcast",
        BroadcastGroup{config<uint32_t>("host"), group.id(), message, static_cast<uint8_t>(type)});

    this->on_group_broadcast(resp);
}

void context::on_group_broadcast(const internal_resp::BroadcastGroup& resp)
{
    this->assert_group(resp.error, "");

    this->upsert_group_then(resp.group, [this, message = resp.message, type = resp.type](auto& group) {
        this->characters.foreach (group->members(), [message, type](auto& ch) {
            ch->message(message, static_cast<MESSAGE_TYPE>(type));
        });
    });
}