#ifndef __DOOR_H__
#define __DOOR_H__

#include <vector>
#include "mmo.h"

namespace fb { namespace game {

class map;

class door
{
public:
    typedef struct _element
    {
        uint16_t                    open;
        uint16_t                    close;

        _element(uint16_t open, uint16_t close) : open(open), close(close) {}
        ~_element() {}
    } element;

    class core : private std::vector<element>
    {
    public:
        using std::vector<element>::begin;
        using std::vector<element>::end;
        using std::vector<element>::cbegin;
        using std::vector<element>::cend;
        using std::vector<element>::rbegin;
        using std::vector<element>::rend;
        using std::vector<element>::crbegin;
        using std::vector<element>::crend;
        using std::vector<element>::size;
        using std::vector<element>::push_back;
        using std::vector<element>::operator[];

    private:
        bool                    matched(const map& map, const point16_t& position, bool open) const;

    public:
        bool                    find(const map& map, point16_t& position, bool open) const;
    };

private:
    game::map*                  _owner;
    core&                       _core;
    bool                        _opened;

public:
    const point16_t             position;

public:
    door(fb::game::map* owner, door::core& core, const point16_t position, bool opened);
    ~door();

public:
    const door::core&           based() const;
    bool                        toggle();
    bool                        opened() const;
};

class doors : private std::vector<door*>
{
public:
    using std::vector<door*>::begin;
    using std::vector<door*>::end;
    using std::vector<door*>::cbegin;
    using std::vector<door*>::cend;
    using std::vector<door*>::rbegin;
    using std::vector<door*>::rend;

public:
    doors();
    ~doors();

public:
    void                        add(map* map, fb::game::door::core& core, const point16_t position, bool opened);
    door*                       find(const point16_t position);
};

} }

#endif // !__DOOR_H__