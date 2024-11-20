#ifndef __LISTENER_H__
#define __LISTENER_H__

#include <character.h>

namespace fb { namespace game {

/**
 * @brief      { struct_description }
 */
struct listener : public virtual character::listener, public virtual mob::listener
{ };

}} // namespace fb::game

#endif // !__LISTENER_H__
