#include <fb/game/context.h>

using namespace fb::game;

void context::on_trade_begin(character& me, character& you)
{
    this->send(me, fb_resp::trade_dialog(you, this->model), scope::SELF);
}

void context::on_trade_bundle(character& me)
{
    this->send(me, fb_resp::trade_bundle(), scope::SELF);
}

void context::on_trade_money(character& me, character& you, uint32_t money)
{
    this->send(me, fb_resp::trade_money(money, true), scope::SELF);
    this->send(you, fb_resp::trade_money(money, false), scope::SELF);
}

void context::on_trade_cancel(character& me, character& you)
{
    this->send(me, fb_resp::trade_close(_TEXT(MESSAGE_TRADE_CANCELLED_BY_ME)), scope::SELF);
    this->send(you, fb_resp::trade_close(_TEXT(MESSAGE_TRADE_CANCELLED_BY_PARTNER)), scope::SELF);
}

void context::on_trade_lock(character& me, character& you)
{
    this->send(me, fb_resp::trade_lock(), scope::SELF);
    this->send(you, fb_resp::message(_TEXT(MESSAGE_TRADE_NOTIFY_LOCK_TO_PARTNER), MESSAGE_TYPE::POPUP), scope::SELF);
}

void context::on_trade_failed(character& me, character& you)
{
    this->send(me, fb_resp::trade_close(_TEXT(MESSAGE_TRADE_FAILED)), scope::SELF);
    this->send(you, fb_resp::trade_close(_TEXT(MESSAGE_TRADE_FAILED)), scope::SELF);
}

void context::on_trade_success(character& me, character& you)
{
    this->send(me, fb_resp::trade_close(_TEXT(MESSAGE_TRADE_SUCCESS)), scope::SELF);
    me.update(STATE_LEVEL::EXP_MONEY);

    this->send(you, fb_resp::trade_close(_TEXT(MESSAGE_TRADE_SUCCESS)), scope::SELF);
    you.update(STATE_LEVEL::EXP_MONEY);
}

void context::on_trade_item(character& me, character& you, uint8_t index, const fb::game::item& item)
{
    this->send(me, fb_resp::trade_upload(index, item, true), scope::SELF);
    this->send(you, fb_resp::trade_upload(index, item, false), scope::SELF);
}