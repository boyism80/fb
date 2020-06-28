#include <model/spell/spell.h>
#include <model/acceptor/acceptor.game.h>
#include <model/object/object.h>

fb::game::spell::spell(types type, const std::string& name, const std::string& cast, const std::string& uncast, const std::string& concast, const std::string& message) : 
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

fb::game::spell::types fb::game::spell::type() const
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

int fb::game::spell::builtin_type(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto spell = *(fb::game::spell**)lua_touserdata(lua, 1);

    lua_pushinteger(lua, spell->type());
    return 1;
}

int fb::game::spell::builtin_name(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto spell = *(fb::game::spell**)lua_touserdata(lua, 1);

    lua_pushstring(lua, spell->_name.c_str());
    return 1;
}

int fb::game::spell::builtin_message(lua_State* lua)
{
    auto acceptor = lua::env<fb::game::acceptor>("acceptor");
    auto spell = *(fb::game::spell**)lua_touserdata(lua, 1);

    lua_pushstring(lua, spell->_message.c_str());
    return 1;
}

fb::game::spells::spells(life& owner) : 
    container(owner, spell::MAX_SLOT, false)
{
}

fb::game::spells::~spells()
{
}

fb::ostream fb::game::spells::make_update_stream(uint8_t index) const
{
    ostream                 ostream;
    auto                    spell = this->at(index);
    if(spell == nullptr)
        return ostream;

    ostream.write_u8(0x17)
        .write_u8(index + 1)
        .write_u8(spell->type())
        .write(spell->name());

    if(spell->type() < 3)
        ostream.write(spell->message());

    return ostream;
}

fb::ostream fb::game::spells::make_delete_stream(uint8_t index) const
{
    ostream                 ostream;
    auto                    spell = this->at(index);
    if(spell != nullptr)
        return ostream;

    ostream.write_u8(0x18)
        .write_u8(index + 1)
        .write_u8(0x00);

    return ostream;
}

fb::game::buff::buff(const game::spell* spell, uint32_t time) : 
    _spell(spell),
    _time(time)
{
}

fb::game::buff::~buff()
{
}

const fb::game::spell& fb::game::buff::spell() const
{
    return *this->_spell;
}

uint32_t fb::game::buff::time() const
{
    return this->_time;
}

void fb::game::buff::time(uint32_t value)
{
    this->_time = value;
}

void fb::game::buff::time_inc(uint32_t inc)
{
    this->_time++;
}

void fb::game::buff::time_dec(uint32_t dec)
{
    this->_time--;
}

fb::ostream fb::game::buff::make_stream() const
{
    fb::ostream             ostream;

    ostream.write_u8(0x3A)
        .write(this->_spell->name())
        .write_u32(this->_time);

    return ostream;
}

fb::ostream fb::game::buff::make_clear_stream() const
{
    fb::ostream             ostream;

    ostream.write_u8(0x3A)
        .write(this->_spell->name())
        .write_u32(0x00);

    return ostream;
}

fb::game::buff::operator const fb::game::spell& () const
{
    return *this->_spell;
}

fb::game::buff::operator const fb::game::spell* () const
{
    return this->_spell;
}

fb::game::buffs::buffs(fb::game::object& owner) : 
    _owner(owner)
{
}

fb::game::buffs::~buffs()
{
    for(auto buff : *this)
        delete buff;
}

bool fb::game::buffs::contains(const buff* buff) const
{
    return this->contains(buff->spell().name());
}

bool fb::game::buffs::contains(const spell* spell) const
{
    return this->contains(spell->name());
}

bool fb::game::buffs::contains(const std::string& name) const
{
    return this->operator[](name) != nullptr;
}

bool fb::game::buffs::push_back(buff* buff)
{
    if(this->contains(buff))
        return false;

    std::vector<game::buff*>::push_back(buff);
    return true;
}

fb::game::buff* fb::game::buffs::push_back(const game::spell* spell, uint32_t time)
{
    if(this->contains(spell))
        return nullptr;

    auto created = new buff(spell, time);
    this->push_back(created);
    return created;
}

bool fb::game::buffs::remove(const std::string& name)
{
    auto buff = this->operator[](name);
    auto found = std::find(this->begin(), this->end(), buff);
    if(found == this->end())
        return false;

    this->erase(found);
    if(this->_owner._listener != nullptr)
        this->_owner._listener->on_unbuff(this->_owner, *buff);
    return true;
}

bool fb::game::buffs::remove(const game::spell* spell)
{
    return this->remove(spell->name());
}

void fb::game::buffs::remove(buff* buff)
{
    this->remove(buff->spell().name());
}

fb::game::buff* fb::game::buffs::operator[](const std::string& name) const
{
    for(auto buff : *this)
    {
        if(buff->spell().name() == name)
            return buff;
    }

    return nullptr;
}
