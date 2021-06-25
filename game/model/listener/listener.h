#ifndef __LISTENER_H__
#define __LISTENER_H__

#include "model/mob/mob.h"
#include "model/session/session.h"
#include "model/item/item.h"

namespace fb { namespace game {

interface listener : public virtual session::listener,
    public virtual mob::listener
{
public:
    // game
    virtual void on_save(session& me, std::function<void(fb::game::session&)> fn) = 0;
};


} }

#endif // !__LISTENER_H__
