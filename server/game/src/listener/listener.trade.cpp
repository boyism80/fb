#include <fb/game/server.h>
#include <fb/protocol/client_version.h>

using namespace fb::game;

namespace game_resp = fb::protocol::game::response;

void listener_impl::on_trade_begin(character& me, character& you)
{
    fb::protocol::visit_client_version(me.client_version, [&]<fb::protocol::CLIENT_VERSION V> {
        me.send(game_resp::trade_dialog<V>(you));
    });
}

void listener_impl::on_trade_bundle(character& me)
{
    me.send(game_resp::trade_bundle());
}

void listener_impl::on_trade_money(character& me, character& you, uint64_t money)
{
    me.send(game_resp::trade_money(money, true));
    you.send(game_resp::trade_money(money, false));
}

void listener_impl::on_trade_cancel(character& me, character& you)
{
    me.send(game_resp::trade_close(_TEXT(MESSAGE_TRADE_CANCELLED_BY_ME)));
    you.send(game_resp::trade_close(_TEXT(MESSAGE_TRADE_CANCELLED_BY_PARTNER)));
}

void listener_impl::on_trade_lock(character& me, character& you)
{
    me.send(game_resp::trade_lock());
    you.send(game_resp::message(_TEXT(MESSAGE_TRADE_NOTIFY_LOCK_TO_PARTNER), MESSAGE_TYPE::POPUP));
}

void listener_impl::on_trade_failed(character& me, character& you)
{
    me.send(game_resp::trade_close(_TEXT(MESSAGE_TRADE_FAILED)));
    you.send(game_resp::trade_close(_TEXT(MESSAGE_TRADE_FAILED)));
}

void listener_impl::on_trade_success(character& me, character& you)
{
    me.send(game_resp::trade_close(_TEXT(MESSAGE_TRADE_SUCCESS)));
    you.send(game_resp::trade_close(_TEXT(MESSAGE_TRADE_SUCCESS)));
}

void listener_impl::on_trade_item(character& me, character& you, uint8_t index, const fb::game::item& item)
{
    me.send(game_resp::trade_upload(index, item, true));
    you.send(game_resp::trade_upload(index, item, false));
}