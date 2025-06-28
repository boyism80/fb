#include <fb/game/context.h>

using namespace fb::game;

async::task<void> context::handle_amqp_shutdown(const internal_resp::Shutdown& response)
{
    this->exit();
    co_return;
}

async::task<void> context::handle_amqp_Broadcast(const internal_resp::Broadcast& resp)
{
    if (resp.host == fb::config<uint32_t>("id"))
        co_return;

    this->on_broadcast(resp);
}

async::task<void> context::handle_amqp_KickOut(const internal_resp::KickOut& resp)
{
    auto ch = this->characters.find(resp.name);
    if (ch == nullptr)
        co_return;

    auto socket = ch->socket();
    if (socket != nullptr)
        socket->close();
    co_return;
}

async::task<void> context::handle_amqp_Whisper(const internal_resp::Whisper& resp)
{
    if (resp.host == fb::config<uint16_t>("id"))
        co_return;

    this->on_whisper(resp);
}

async::task<void> context::handle_amqp_EnterGroup(const internal_resp::EnterGroup& resp)
{
    if (resp.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->on_enter_group(resp);
};
async::task<void> context::handle_amqp_LeaveGroup(const internal_resp::LeaveGroup& resp)
{
    if (resp.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->on_leave_group(resp);
};
async::task<void> context::handle_amqp_SetClanTitle(const internal_resp::SetClanTitle& resp)
{
    if (resp.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->on_clan_title_changed(resp);
};
async::task<void> context::handle_amqp_JoinClan(const internal_resp::JoinClan& resp)
{
    if (resp.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->on_clan_join_member(resp);
};

async::task<void> context::handle_amqp_LeaveClan(const internal_resp::LeaveClan& resp)
{
    if (resp.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->on_clan_leave_member(resp);
};

async::task<void> context::handle_amqp_BroadcastClan(const internal_resp::BroadcastClan& resp)
{
    if (resp.host == fb::config<uint32_t>("id"))
        co_return;

    co_await this->on_clan_broadcast(resp);
};

async::task<void> context::handle_amqp_WriteMail(const internal_resp::WriteMail& resp)
{
    if (resp.host == fb::config<uint32_t>("id"))
        co_return;

    this->on_write_mail(resp);
};