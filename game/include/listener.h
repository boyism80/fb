#ifndef __LISTENER_H__
#define __LISTENER_H__

#include <mob.h>
#include <session.h>
#include <item.h>

namespace fb { namespace game {

interface listener : public virtual session::listener,
    public virtual mob::listener
{ };


} }

#endif // !__LISTENER_H__
