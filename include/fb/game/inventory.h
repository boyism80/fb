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
    using array_type                = std::array<T*, CONTAINER_CAPACITY>;
    using iterator                  = array_type::iterator;
    using const_iterator            = array_type::const_iterator;

private:
    life&                           _owner;
    array_type                      _elements = { nullptr, };

protected:
    inventory(life& owner);
    ~inventory();

protected:
    T*                              set(T* element, int position);

public:
    uint8_t                         next() const;
    life&                           owner();
    const life&                     owner() const;
    virtual T*                      at(uint8_t index) const;
    virtual async::task<uint8_t>       add(T& element);
    virtual async::task<uint8_t>       add(T& element, uint8_t index);
    virtual bool                    remove(uint8_t index);
    virtual bool                    swap(uint8_t src, uint8_t dst);

    bool                            free() const;
    uint8_t                         free_size() const;

public:
    iterator                        begin();
    iterator                        end();
    const_iterator                  cbegin() const;
    const_iterator                  cend() const;

public:
    T*                              operator [] (int index);
    const T*                        operator [] (int index) const;
};

#include "inventory.hpp"

} }

#endif // !__CONTAINER_H__
