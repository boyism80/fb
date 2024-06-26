#include <fb/game/spell.h>
#include <fb/game/life.h>

fb::game::spells::spells(life& owner) : inventory(owner)
{ }

fb::game::spells::~spells()
{ }

uint8_t fb::game::spells::add(const fb::model::spell& element)
{
    auto index = fb::game::inventory<const fb::model::spell>::add(element);
    auto listener = this->owner().get_listener<fb::game::spells>();

    if(index != 0xFF && listener != nullptr)
        listener->on_spell_update(this->owner(), index);

    return index;
}

uint8_t fb::game::spells::add(const fb::model::spell& element, uint8_t index)
{
    auto listener = this->owner().get_listener<fb::game::spells>();

    if(fb::game::inventory<const fb::model::spell>::add(element, index) != 0xFF)
    {
        if(listener != nullptr)
        {
            listener->on_spell_update(this->owner(), index);
        }
    }

    return index;
}

bool fb::game::spells::remove(uint8_t index)
{
    auto success = fb::game::inventory<const fb::model::spell>::remove(index);
    auto listener = this->owner().get_listener<fb::game::spells>();

    if(success)
        if(listener != nullptr)
            listener->on_spell_remove(this->owner(), index);

    return success;
}

bool fb::game::spells::swap(uint8_t src, uint8_t dst)
{
    if(fb::game::inventory<const fb::model::spell>::swap(src, dst) == false)
        return false;

    auto listener = this->owner().get_listener<fb::game::spells>();
    if(listener != nullptr)
    {
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
    }

    return true;
}

fb::game::buff::buff(const fb::model::spell& model, uint32_t seconds) : 
    model(model),
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

fb::game::buffs::buffs(fb::game::object& owner) : 
    _owner(owner)
{ }

fb::game::buffs::~buffs()
{ }

bool fb::game::buffs::contains(const fb::model::spell& model) const
{
    return this->contains(model.name);
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
    if(this->contains(buff->model))
        return false;

    std::vector<std::unique_ptr<fb::game::buff>>::push_back(std::move(buff));
    return true;
}

fb::game::buff* fb::game::buffs::push_back(const fb::model::spell& spell, uint32_t seconds)
{
    if(this->contains(spell))
        return nullptr;

    auto created = std::make_unique<buff>(spell, seconds);
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
    if(listener != nullptr)
        listener->on_unbuff(this->_owner, *buff);

    this->erase(found);
    return true;
}

bool fb::game::buffs::remove(const fb::model::spell& spell)
{
    return this->remove(spell.name);
}

fb::game::buff* fb::game::buffs::operator[](const std::string& name) const
{
    for(auto& buff : *this)
    {
        if(buff->model.name == name)
            return buff.get();
    }

    return nullptr;
}
