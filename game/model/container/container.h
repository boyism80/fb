#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#include <stdlib.h>

namespace fb { namespace game {

class life;

template <typename T>
class base_container
{
private:
    life&                   _owner;
    T**                     _elements;
    uint8_t                 _size;
    bool                    _auto_release;
    uint64_t                _modified;

protected:
    base_container(life& owner, uint8_t size, bool auto_release = true);
    ~base_container();

protected:
    T*                      set(T* element, int position);
    void                    modify(uint8_t index);
    bool                    modified() const;
    bool                    modified(uint8_t index) const;
    bool                    modified_reset();

public:
    uint8_t                 next() const;
    life&                   owner();
    const life&             owner() const;
    virtual T*              at(uint8_t index) const;
    virtual uint8_t         add(T& element);
    virtual uint8_t         add(T* element);
    virtual uint8_t         add(T& element, uint8_t index);
    virtual uint8_t         add(T* element, uint8_t index);
    virtual bool            remove(uint8_t index);
    virtual bool            swap(uint8_t src, uint8_t dst);

    bool                    free() const;
    uint8_t                 free_size() const;

public:
    T*                      operator [] (int index);
    const T*                operator [] (int index) const;
};

#include "container.hpp"

} }

#endif // !__CONTAINER_H__
