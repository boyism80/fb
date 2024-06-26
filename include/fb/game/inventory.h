#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#include <stdlib.h>
#include <memory>

namespace fb { namespace game {

static constexpr uint8_t CONTAINER_CAPACITY = 52;

class life;

template <typename T>
class inventory
{
public:
    using unique_set = std::array<T*, CONTAINER_CAPACITY>;

private:
    life&                   _owner;
    unique_set              _elements = { nullptr, };

protected:
    inventory(life& owner);
    ~inventory();

protected:
    T*                      set(T* element, int position);

public:
    uint8_t                 next() const;
    life&                   owner();
    const life&             owner() const;
    virtual T*              at(uint8_t index) const;
    virtual uint8_t         add(T& element);
    virtual uint8_t         add(T& element, uint8_t index);
    virtual bool            remove(uint8_t index);
    virtual bool            swap(uint8_t src, uint8_t dst);

    bool                    free() const;
    uint8_t                 free_size() const;

public:
    T*                      operator [] (int index);
    const T*                operator [] (int index) const;
};

#include "inventory.hpp"

} }

#endif // !__CONTAINER_H__
