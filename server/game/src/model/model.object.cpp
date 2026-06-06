#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/game/map.h>
#include <fb/game/mob.h>
#include <fb/model/model.h>
#include <fb/game/object.h>

uint8_t fb::model::object::dialog_look_type() const
{
    return this->look > 0xBFFF ? 0x02 : 0x01;
}

fb::model::enum_value::OBJECT_TYPE fb::model::object::what() const
{
    return fb::model::enum_value::OBJECT_TYPE::UNKNOWN;
}

bool fb::model::object::operator== (const fb::model::object& r) const
{
    return this == &r;
}

bool fb::model::object::operator!= (const fb::model::object& r) const
{
    return this != &r;
}