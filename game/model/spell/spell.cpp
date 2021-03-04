#include "model/spell/spell.h"
#include "model/listener/listener.h"

fb::game::spell::spell(uint16_t id, types type, const std::string& name, const std::string& cast, const std::string& uncast, const std::string& concast, const std::string& message) : 
    _id(id),
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

uint16_t fb::game::spell::id() const
{
    return this->_id;
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
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto spell = thread->touserdata<fb::game::spell>(1);
    if(spell == nullptr)
        return 0;

    thread->pushinteger(spell->type());
    return 1;
}

int fb::game::spell::builtin_name(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto spell = thread->touserdata<fb::game::spell>(1);
    if(spell == nullptr)
        return 0;

    thread->pushstring(spell->_name);
    return 1;
}

int fb::game::spell::builtin_message(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto spell = thread->touserdata<fb::game::spell>(1);
    if(spell == nullptr)
        return 0;

    thread->pushstring(spell->_message);
    return 1;
}

fb::game::spells::spells(life& owner, listener* listener) : 
    base_container(owner, spell::MAX_SLOT, false),
    _listener(listener)
{
}

fb::game::spells::~spells()
{
}

uint8_t fb::game::spells::add(spell& element)
{
    auto index = fb::game::base_container<fb::game::spell>::add(element);
    if(index != 0xFF && this->_listener != nullptr)
        this->_listener->on_spell_update(this->owner(), index);

    return index;
}

uint8_t fb::game::spells::add(spell& element, uint8_t index)
{
    if(fb::game::base_container<fb::game::spell>::add(element, index) != 0xFF && this->_listener != nullptr)
        this->_listener->on_spell_update(this->owner(), index);

    return index;
}

uint8_t fb::game::spells::add(spell* element)
{
    if(element == nullptr)
        return 0xFF;

    return this->add(*element);
}

uint8_t fb::game::spells::add(spell* element, uint8_t index)
{
    if(element == nullptr)
        return 0xFF;

    return this->add(*element, index);
}

bool fb::game::spells::remove(uint8_t index)
{
    auto success = fb::game::base_container<fb::game::spell>::remove(index);
    if(success && this->_listener != nullptr)
        this->_listener->on_spell_remove(this->owner(), index);

    return success;
}

bool fb::game::spells::swap(uint8_t src, uint8_t dst)
{
    if(fb::game::base_container<fb::game::spell>::swap(src, dst) == false)
        return false;

    if(this->_listener != nullptr)
    {
        const auto              right = this->at(src);
        if(right != nullptr)
            this->_listener->on_spell_update(this->owner(), src);
        else
            this->_listener->on_spell_remove(this->owner(), src);

        const auto              left = this->at(dst);
        if(left != nullptr)
            this->_listener->on_spell_update(this->owner(), dst);
        else
            this->_listener->on_spell_remove(this->owner(), dst);
    }

    return true;
}

fb::game::buff::buff(const fb::game::spell* spell, uint32_t time) : 
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

fb::game::buff* fb::game::buffs::push_back(const fb::game::spell* spell, uint32_t time)
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

bool fb::game::buffs::remove(const fb::game::spell* spell)
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
