#ifndef __LISTENER_H__
#define __LISTENER_H__

#include "model/mob/mob.h"
#include "model/session/session.h"
#include "model/item/item.h"

namespace fb { namespace game {

interface listener : public virtual session::listener,
    public virtual mob::listener
{ };


} }

#endif // !__LISTENER_H__
