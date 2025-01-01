#include <fb/game/context.h>

using namespace fb::game;

async::task<void> context::handle_amqp_Pong(const internal_resp::Pong& response)
{
    co_return;
}

async::task<void> context::handle_amqp_KickOut(const internal_resp::KickOut& response)
{
    auto ch = this->_shard[response.name]->characters.template lock<character*>(
        [&name = response.name](shard_params::character_container& container) -> character* {
            if (!container.contains(name))
                return nullptr;

            return container.at(name);
        });

    if (ch == nullptr)
        co_return;

    auto& socket = static_cast<fb::socket<character>&>(*ch);
    socket.close();
    co_return;
}

async::task<void> context::handle_amqp_Whisper(const internal_resp::Whisper& response)
{
    if (response.host == fb::config<uint16_t>("id"))
        co_return;

    try
    {
        this->assert_whisper(response);
        this->foreach_ch(response.to, [&response](auto& you) {
            you.message(std::format("{}> {}", response.from, response.message), MESSAGE_TYPE::NOTIFY);
        });
    }
    catch (std::exception& e)
    {
        this->foreach_ch(response.from, [&response, error = e.what()](auto& me) {
            me.message(error, MESSAGE_TYPE::NOTIFY);
        });
    }
}

async::task<void> context::handle_amqp_EnterGroup(const internal_resp::EnterGroup& response)
{
    if (response.host == fb::config<uint32_t>("id"))
        co_return;

    this->on_enter_group(response);
};
async::task<void> context::handle_amqp_LeaveGroup(const internal_resp::LeaveGroup& response)
{
    if (response.host == fb::config<uint32_t>("id"))
        co_return;

    this->on_leave_group(response);
};
async::task<void> context::handle_amqp_SetClanTitle(const internal_resp::SetClanTitle& response)
{
    if (response.host == fb::config<uint32_t>("id"))
        co_return;

    this->on_clan_title_changed(response);
};
async::task<void> context::handle_amqp_JoinClan(const internal_resp::JoinClan& response)
{
    if (response.host == fb::config<uint32_t>("id"))
        co_return;

    this->on_clan_join_member(response);
};

async::task<void> context::handle_amqp_LeaveClan(const internal_resp::LeaveClan& response)
{
    if (response.host == fb::config<uint32_t>("id"))
        co_return;

    this->on_clan_leave_member(response);
};

async::task<void> context::handle_amqp_BroadcastClan(const internal_resp::BroadcastClan& response)
{
    if (response.host == fb::config<uint32_t>("id"))
        co_return;

    this->on_clan_broadcast(response);
};
