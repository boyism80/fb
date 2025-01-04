#include <fb/game/context.h>

using namespace fb::game;

void context::on_trade_begin(character& me, character& you)
{
    me.send(fb_resp::trade_dialog(you, this->model));
}

void context::on_trade_bundle(character& me)
{
    me.send(fb_resp::trade_bundle());
}

void context::on_trade_money(character& me, character& you, uint32_t money)
{
    me.send(fb_resp::trade_money(money, true));
    you.send(fb_resp::trade_money(money, false));
}

void context::on_trade_cancel(character& me, character& you)
{
    me.send(fb_resp::trade_close(_TEXT(MESSAGE_TRADE_CANCELLED_BY_ME)));
    you.send(fb_resp::trade_close(_TEXT(MESSAGE_TRADE_CANCELLED_BY_PARTNER)));
}

void context::on_trade_lock(character& me, character& you)
{
    me.send(fb_resp::trade_lock());
    you.send(fb_resp::message(_TEXT(MESSAGE_TRADE_NOTIFY_LOCK_TO_PARTNER), MESSAGE_TYPE::POPUP));
}

void context::on_trade_failed(character& me, character& you)
{
    me.send(fb_resp::trade_close(_TEXT(MESSAGE_TRADE_FAILED)));
    you.send(fb_resp::trade_close(_TEXT(MESSAGE_TRADE_FAILED)));
}

void context::on_trade_success(character& me, character& you)
{
    me.send(fb_resp::trade_close(_TEXT(MESSAGE_TRADE_SUCCESS)));
    me.update(STATE_LEVEL::EXP_MONEY);

    you.send(fb_resp::trade_close(_TEXT(MESSAGE_TRADE_SUCCESS)));
    you.update(STATE_LEVEL::EXP_MONEY);
}

void context::on_trade_item(character& me, character& you, uint8_t index, const fb::game::item& item)
{
    me.send(fb_resp::trade_upload(index, item, true));
    you.send(fb_resp::trade_upload(index, item, false));
}