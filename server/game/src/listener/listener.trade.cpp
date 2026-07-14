#include <fb/game/server.h>
#include <tuple>

using namespace fb::game;

namespace game_resp = fb::protocol::game::response;

async::task<void> listener_impl::on_trade_begin(character& me, character& you)
{
    std::ignore = co_await me.send(game_resp::trade_dialog(you));
    co_return;
}

async::task<void> listener_impl::on_trade_bundle(character& me)
{
    std::ignore = co_await me.send(game_resp::trade_bundle());
    co_return;
}

async::task<void> listener_impl::on_trade_money(character& me, character& you, uint32_t money)
{
    std::ignore = co_await me.send(game_resp::trade_money(money, true));
    std::ignore = co_await you.send(game_resp::trade_money(money, false));
    co_return;
}

async::task<void> listener_impl::on_trade_cancel(character& me, character& you)
{
    std::ignore = co_await me.send(game_resp::trade_close(_TEXT(MESSAGE_TRADE_CANCELLED_BY_ME)));
    std::ignore = co_await you.send(game_resp::trade_close(_TEXT(MESSAGE_TRADE_CANCELLED_BY_PARTNER)));
    co_return;
}

async::task<void> listener_impl::on_trade_lock(character& me, character& you)
{
    std::ignore = co_await me.send(game_resp::trade_lock());
    std::ignore =
        co_await you.send(game_resp::message(_TEXT(MESSAGE_TRADE_NOTIFY_LOCK_TO_PARTNER), MESSAGE_TYPE::POPUP));
    co_return;
}

async::task<void> listener_impl::on_trade_failed(character& me, character& you)
{
    std::ignore = co_await me.send(game_resp::trade_close(_TEXT(MESSAGE_TRADE_FAILED)));
    std::ignore = co_await you.send(game_resp::trade_close(_TEXT(MESSAGE_TRADE_FAILED)));
    co_return;
}

async::task<void> listener_impl::on_trade_success(character& me, character& you)
{
    std::ignore = co_await me.send(game_resp::trade_close(_TEXT(MESSAGE_TRADE_SUCCESS)));
    std::ignore = co_await you.send(game_resp::trade_close(_TEXT(MESSAGE_TRADE_SUCCESS)));
    co_return;
}

async::task<void> listener_impl::on_trade_item(character& me, character& you, uint8_t index, const fb::game::item& item)
{
    std::ignore = co_await me.send(game_resp::trade_upload(index, item, true));
    std::ignore = co_await you.send(game_resp::trade_upload(index, item, false));
    co_return;
}
