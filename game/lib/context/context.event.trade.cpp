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

void context::on_trade_money(character& me, character& from)
{
    bool mine = (&me == &from);
    this->send(me, fb_resp::trade_money(from, mine), scope::SELF);
}

void context::on_trade_cancel(character& me, character& from)
{
    bool mine = (&me == &from);
    this->send(me,
               fb_resp::trade_close(mine ? message::trade::CANCELLED_BY_ME : message::trade::CANCELLED_BY_PARTNER),
               scope::SELF);
}

void context::on_trade_lock(character& me, bool mine)
{
    if (mine)
    {
        this->send(me, fb_resp::trade_lock(), scope::SELF);
    }
    else
    {
        this->send(me, fb_resp::message(message::trade::NOTIFY_LOCK_TO_PARTNER, MESSAGE_TYPE::POPUP), scope::SELF);
    }
}

void context::on_trade_failed(character& me)
{
    this->send(me, fb_resp::trade_close(message::trade::FAILED), scope::SELF);
}

void context::on_trade_success(character& me)
{
    this->send(me, fb_resp::trade_close(message::trade::SUCCESS), scope::SELF);
}

void context::on_trade_item(character& me, character& from, uint8_t index)
{
    bool mine = (&me == &from);
    this->send(me, fb_resp::trade_upload(from, index, mine), scope::SELF);
}