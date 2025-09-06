#ifndef __DOOR_H__
#define __DOOR_H__

#include <fb/lua.h>
#include <fb/model/model.h>

namespace fb::game {

class map;
class character;

class door : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    class model;

private:
    bool _opened = false;
    bool _locked = false;

public:
    const fb::game::map&       map;
    const fb::model::door&     model;
    const fb::model::point16_t pivot;
    const uint16_t             width;

public:
    door(const fb::game::map& map, const fb::model::door& model, const fb::model::point16_t& pivot, bool opened);
    door(const door&) = delete;
    ~door();

public:
    bool toggle();
    bool opened() const;
    bool locked() const;
    void lock(bool value);
};

} // namespace fb::game

#endif // !__DOOR_H__