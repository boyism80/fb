#include <fb/game/crowd_control.h>
#include <fb/game/life.h>

using namespace fb::game;

crowd_control::crowd_control(life& owner) :
    _owner(owner)
{ }

crowd_control::crowd_control(life& owner, CROWD_CONTROL value) :
    _owner(owner),
    _value(value)
{ }

crowd_control::crowd_control(life& owner, const crowd_control& other) :
    _owner(owner),
    _value(other._value)
{ }

crowd_control::crowd_control(life& owner, crowd_control&& other) :
    _owner(owner),
    _value(std::move(other._value))
{ }

CROWD_CONTROL crowd_control::add(CROWD_CONTROL value)
{
    this->_owner.assert_thread();
    this->_value = CROWD_CONTROL(this->_value | value);
    this->_owner.update(UPDATE_STATE_LEVEL::CROWD_CONTROL);
    return this->_value;
}

CROWD_CONTROL crowd_control::remove(CROWD_CONTROL value)
{
    this->_owner.assert_thread();
    this->_value = CROWD_CONTROL(this->_value & ~value);
    this->_owner.update(UPDATE_STATE_LEVEL::CROWD_CONTROL);
    return this->_value;
}

bool crowd_control::contains(CROWD_CONTROL value) const
{
    this->_owner.assert_thread();
    return uint32_t(this->_value) & uint32_t(value);
}

void crowd_control::clear()
{
    this->_owner.assert_thread();
    this->_value = CROWD_CONTROL::NONE;
    this->_owner.update(UPDATE_STATE_LEVEL::CROWD_CONTROL);
}

void crowd_control::set(CROWD_CONTROL value)
{
    this->_owner.assert_thread();
    this->_value = value;
    this->_owner.update(UPDATE_STATE_LEVEL::CROWD_CONTROL);
}

crowd_control::operator CROWD_CONTROL () const
{
    this->_owner.assert_thread();
    return this->_value;
}

crowd_control::operator uint32_t () const
{
    this->_owner.assert_thread();
    return static_cast<uint32_t>(this->_value);
}