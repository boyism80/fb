#include "spell.h"

fb::game::spell::spell(uint8_t type, const std::string& name, const std::string& cast, const std::string& uncast, const std::string& concast, const std::string& message) : 
    _type(type),
    _name(name),
    _cast(cast),
    _uncast(uncast),
    _concast(concast),
    _message(message)
{
}

fb::game::spell::~spell()
{
}

uint8_t fb::game::spell::type() const
{
    return this->_type;
}

const std::string& fb::game::spell::name() const
{
    return this->_name;
}

const std::string& fb::game::spell::cast() const
{
    return this->_cast;
}

const std::string& fb::game::spell::uncast() const
{
    return this->_uncast;
}

const std::string& fb::game::spell::concast() const
{
    return this->_concast;
}

const std::string& fb::game::spell::message() const
{
    return this->_message;
}

fb::ostream fb::game::spell::make_show_stream(uint8_t slot) const
{
    ostream                 ostream;

    ostream.write_u8(0x17)
        .write_u8(slot) // one-based
        .write_u8(this->_type)
        .write(this->_name);

    if(this->_type < 3)
        ostream.write(this->_message);

    return ostream;
}

fb::ostream fb::game::spell::make_delete_stream(uint8_t slot)
{
    ostream                 ostream;
    
    ostream.write_u8(0x18)
        .write_u8(slot) // one-based
        .write_u8(0x00);

    return ostream;
}
