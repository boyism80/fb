#ifndef __LISTENER_H__
#define __LISTENER_H__

#include "model/mob/mob.h"
#include "model/session/session.h"
#include "model/item/item.h"

namespace fb { namespace game {

interface listener : public session::listener,
    public mob::listener
{
public:
    // game
    virtual void on_save(session& me) = 0;
};


} }

#endif // !__LISTENER_H__
