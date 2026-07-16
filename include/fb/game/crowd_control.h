#ifndef __CROWD_CONTROL_H__
#define __CROWD_CONTROL_H__

#include <fb/model/model.h>

namespace fb::game {

using namespace fb::model::enum_value;

class life;

class crowd_control
{
private:
    life&         _owner;
    CROWD_CONTROL _value = CROWD_CONTROL::NONE;

public:
    // clang-format off
    crowd_control(life& owner);
    crowd_control(life& owner, CROWD_CONTROL value);
    crowd_control(life& owner, const crowd_control& other);
    crowd_control(life& owner, crowd_control&& other);
    // clang-format on
    ~crowd_control() = default;

public:
    // clang-format off
    CROWD_CONTROL add(CROWD_CONTROL value);
    CROWD_CONTROL remove(CROWD_CONTROL value);
    bool          contains(CROWD_CONTROL value) const;
    void          clear();
    void          set(CROWD_CONTROL value);
    // clang-format on

    // clang-format off
    operator CROWD_CONTROL () const;
    operator uint32_t () const;
    // clang-format on
};

} // namespace fb::game

#endif // __CROWD_CONTROL_H__
