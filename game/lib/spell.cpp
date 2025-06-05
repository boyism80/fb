#include <fb/game/context.h>
#include <fb/game/life.h>
#include <fb/game/spell.h>

using namespace fb::game;

spell::spell(const fb::game::context& context, const life& owner, const fb::model::spell& model, uint16_t delay) :
    context(context),
    owner(owner),
    model(model),
    _next(fb::model::datetime() + std::chrono::seconds(delay))
{ }

spell::~spell()
{ }

void spell::delay(uint16_t value)
{
    this->_next = fb::model::datetime() + std::chrono::seconds(value);
}

uint16_t spell::delay() const
{
    auto diff = this->_next - fb::model::datetime();
    auto sec  = diff.seconds();
    if (sec < 0)
        return 0;

    return sec + (diff.milliseconds() > 0 ? 1 : 0);
}

const fb::model::datetime& fb::game::spell::next() const
{
    return this->_next;
}

spells::spells(life& owner) :
    inventory(owner)
{ }

spells::~spells()
{
    for (auto spell : *this)
    {
        if (spell != nullptr)
            delete spell;
    }
}

fb::game::spell* fb::game::spells::find(const std::string& name) const
{
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto spell = this->at(i);
        if (spell == nullptr)
            continue;

        if (spell->model.name == name)
            return spell;
    }

    return nullptr;
}

fb::game::spell* fb::game::spells::find(const fb::model::spell& model) const
{
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto spell = this->at(i);
        if (spell == nullptr)
            continue;

        if (spell->model.id == model.id)
            return spell;
    }

    return nullptr;
}

uint8_t spells::add(spell& element)
{
    auto& owner = this->owner();
    auto  index = super::add(element);

    if (index != 0xFF)
        owner.listener.on_spell_update(this->owner(), index);

    return index;
}

uint8_t spells::add(spell& element, uint8_t index)
{
    auto& owner = this->owner();

    if (super::add(element, index) != 0xFF)
        owner.listener.on_spell_update(this->owner(), index);

    return index;
}

uint8_t spells::add(const fb::model::spell& model, uint8_t slot, uint16_t delay)
{
    auto& owner   = this->owner();
    auto& context = owner.context;
    auto  created = context.make<spell>(owner, model, delay);
    return this->add(*created, slot);
}

uint8_t spells::add(const fb::model::spell& model)
{
    auto& owner   = this->owner();
    auto& context = owner.context;
    auto  created = context.make<spell>(owner, model, 0);
    return this->add(*created);
}

bool spells::remove(uint8_t index)
{
    auto& owner   = this->owner();
    auto  success = super::remove(index);

    if (success)
        owner.listener.on_spell_remove(this->owner(), index);

    return success;
}

bool spells::swap(uint8_t src, uint8_t dst)
{
    auto& owner = this->owner();

    if (super::swap(src, dst) == false)
        return false;

    const auto right = this->at(src);
    if (right != nullptr)
        owner.listener.on_spell_update(this->owner(), src);
    else
        owner.listener.on_spell_remove(this->owner(), src);

    const auto left = this->at(dst);
    if (left != nullptr)
        owner.listener.on_spell_update(this->owner(), dst);
    else
        owner.listener.on_spell_remove(this->owner(), dst);

    return true;
}

buff::buff(const fb::game::context& context, const fb::model::spell& model, const object* caster, uint32_t seconds) :
    context(context),
    model(model),
    caster(caster),
    _time(seconds * 1000)
{ }

buff::~buff()
{ }

std::chrono::milliseconds buff::time() const
{
    return this->_time;
}

void buff::time_inc(const std::chrono::steady_clock::duration& inc)
{
    this->_time += std::chrono::duration_cast<std::chrono::milliseconds>(inc);
}

void buff::time_dec(const std::chrono::steady_clock::duration& dec)
{
    this->_time -= std::chrono::duration_cast<std::chrono::milliseconds>(dec);
}

buffs::buffs(object& owner) :
    _owner(owner)
{ }

buffs::~buffs()
{
    for (auto& [_, buff] : *this)
    {
        if (buff != nullptr)
            delete buff;
    }
}

bool buffs::contains(const fb::model::spell& model) const
{
    return this->contains(model.id);
}

bool buffs::push_back(buff& buff)
{
    auto& model = buff.model;
    if (this->contains(model.id))
        return false;

    this->insert({model.id, &buff});
    this->_owner.listener.on_buff(this->_owner, buff);

    return true;
}

buff* buffs::push_back(const fb::model::spell& model, uint32_t seconds, const object* caster)
{
    if (this->contains(model.id))
    {
        auto buff = this->at(model.id);
        buff->time(std::chrono::seconds(seconds));
        return buff;
    }

    auto& context = this->_owner.context;
    auto  created = context.make<buff>(model, caster, seconds);
    if (this->push_back(*created) == false)
    {
        std::ignore = context.destroy(*created);
        return nullptr;
    }
    else
    {
        return created;
    }
}

bool buffs::remove(uint32_t id)
{
    auto buff = this->operator[] (id);
    if (buff == nullptr)
        return false;

    this->_owner.listener.on_unbuff(this->_owner, *buff);
    this->erase(id);
    std::ignore = this->_owner.context.destroy(*buff);
    return true;
}

bool buffs::remove(const fb::model::spell& spell)
{
    return this->remove(spell.id);
}

buff* buffs::operator[] (uint32_t id) const
{
    if (this->contains(id) == false)
        return nullptr;

    return super::at(id);
}
