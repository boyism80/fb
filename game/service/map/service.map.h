#ifndef __SERVICE_MAP_H__
#define __SERVICE_MAP_H__

#include "module/service/service.h"
#include "model/map/map.h"
#include "model/object/object.h"

namespace fb { namespace game { namespace service {

class map : public service
{
public:
    map();
    ~map();

public:
    bool                    existable(fb::game::map& map, const point16_t& position) const;
    bool                    movable(fb::game::map& map, const point16_t& position) const;
    bool                    movable(fb::game::map& map, const fb::game::object& object, fb::game::direction direction) const;
    bool                    movable(fb::game::map& map, fb::game::object& object) const;
};

} } }

#endif // !__SERVICE_MAP_H__