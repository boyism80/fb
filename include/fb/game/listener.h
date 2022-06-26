#ifndef __LISTENER_H__
#define __LISTENER_H__

#include <fb/game/mob.h>
#include <fb/game/session.h>
#include <fb/game/item.h>

namespace fb { namespace game {

interface listener : public virtual session::listener,
    public virtual mob::listener
{ };


} }

#endif // !__LISTENER_H__
