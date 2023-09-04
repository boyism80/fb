#include <fb/game/spell.h>
#include <fb/game/life.h>

fb::game::spell::spell(uint16_t id, types type, const std::string& name, const std::string& cast, const std::string& uncast, const std::string& concast, const std::string& message) : 
    id(id),
    type(type),
    name(name),
    cast(cast),
    uncast(uncast),
    concast(concast),
    message(message)
{ }

fb::game::spell::~spell()
{ }

int fb::game::spell::builtin_type(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto spell = thread->touserdata<fb::game::spell>(1);
    if(spell == nullptr)
        return 0;

    thread->pushinteger(spell->type);
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

    thread->pushstring(spell->name);
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

    thread->pushstring(spell->message);
    return 1;
}

fb::game::spells::spells(life& owner) : base_container(owner)
{ }

fb::game::spells::~spells()
{ }

uint8_t fb::game::spells::add(spell& element)
{
    auto index = fb::game::base_container<fb::game::spell>::add(element);
    auto listener = this->owner().get_listener<fb::game::spell>();

    if(index != 0xFF)
        listener->on_spell_update(this->owner(), index);

    return index;
}

uint8_t fb::game::spells::add(spell& element, uint8_t index)
{
    auto listener = this->owner().get_listener<fb::game::spell>();

    if(fb::game::base_container<fb::game::spell>::add(element, index) != 0xFF)
        listener->on_spell_update(this->owner(), index);

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
    auto listener = this->owner().get_listener<fb::game::spell>();

    if(success)
        listener->on_spell_remove(this->owner(), index);

    return success;
}

bool fb::game::spells::swap(uint8_t src, uint8_t dst)
{
    if(fb::game::base_container<fb::game::spell>::swap(src, dst) == false)
        return false;

    auto listener = this->owner().get_listener<fb::game::spell>();
    const auto              right = this->at(src);
    if(right != nullptr)
        listener->on_spell_update(this->owner(), src);
    else
        listener->on_spell_remove(this->owner(), src);

    const auto              left = this->at(dst);
    if(left != nullptr)
        listener->on_spell_update(this->owner(), dst);
    else
        listener->on_spell_remove(this->owner(), dst);

    return true;
}

fb::game::buff::buff(const fb::game::spell& spell, uint32_t seconds) : 
    spell(spell),
    _time(seconds * 1000)
{ }

fb::game::buff::~buff()
{ }


std::chrono::milliseconds fb::game::buff::time() const
{
    return this->_time;
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
    return this->spell;
}

fb::game::buff::operator const fb::game::spell* () const
{
    return &this->spell;
}

fb::game::buffs::buffs(fb::game::object& owner) : 
    _owner(owner)
{ }

fb::game::buffs::~buffs()
{ }

bool fb::game::buffs::contains(const buff* buff) const
{
    return this->contains(buff->spell.name);
}

bool fb::game::buffs::contains(const spell* spell) const
{
    return this->contains(spell->name);
}

fb::game::buff* fb::game::buffs::operator[](int index) const
{
    // TODO: range assert
    return std::vector<std::unique_ptr<buff>>::operator[](index).get();
}

bool fb::game::buffs::contains(const std::string& name) const
{
    return this->operator[](name) != nullptr;
}

bool fb::game::buffs::push_back(std::unique_ptr<buff>&&  buff)
{
    if(this->contains(*buff))
        return false;

    std::vector<std::unique_ptr<fb::game::buff>>::push_back(std::move(buff));
    return true;
}

fb::game::buff* fb::game::buffs::push_back(const fb::game::spell* spell, uint32_t seconds)
{
    if(this->contains(spell))
        return nullptr;

    auto created = std::make_unique<buff>(*spell, seconds);
    auto ptr = created.get();
    this->push_back(std::move(created));
    return ptr;
}

bool fb::game::buffs::remove(const std::string& name)
{
    auto buff = this->operator[](name);
    auto found = std::find_if(this->begin(), this->end(), [buff] (const auto& ptr) { return ptr.get() == buff; });
    if(found == this->end())
        return false;
    
    auto listener = this->_owner.get_listener<fb::game::object>();
    listener->on_unbuff(this->_owner, *buff);

    this->erase(found);
    return true;
}

bool fb::game::buffs::remove(const fb::game::spell* spell)
{
    return this->remove(spell->name);
}

void fb::game::buffs::remove(buff* buff)
{
    this->remove(buff->spell.name);
}

fb::game::buff* fb::game::buffs::operator[](const std::string& name) const
{
    for(auto& buff : *this)
    {
        if(buff->spell.name == name)
            return buff.get();
    }

    return nullptr;
}
