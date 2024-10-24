#include <context.h>
#include <life.h>
#include <spell.h>

fb::game::spells::spells(life& owner) :
    inventory(owner)
{ }

fb::game::spells::~spells()
{ }

async::task<uint8_t> fb::game::spells::add(const fb::model::spell& element)
{
    auto index    = co_await fb::game::inventory<const fb::model::spell>::add(element);
    auto listener = this->owner().get_listener<fb::game::spells>();

    if (index != 0xFF && listener != nullptr)
        listener->on_spell_update(this->owner(), index);

    co_return index;
}

async::task<uint8_t> fb::game::spells::add(const fb::model::spell& element, uint8_t index)
{
    auto listener = this->owner().get_listener<fb::game::spells>();

    if (co_await fb::game::inventory<const fb::model::spell>::add(element, index) != 0xFF)
    {
        if (listener != nullptr)
        {
            listener->on_spell_update(this->owner(), index);
        }
    }

    co_return index;
}

bool fb::game::spells::remove(uint8_t index)
{
    auto success  = fb::game::inventory<const fb::model::spell>::remove(index);
    auto listener = this->owner().get_listener<fb::game::spells>();

    if (success)
        if (listener != nullptr)
            listener->on_spell_remove(this->owner(), index);

    return success;
}

bool fb::game::spells::swap(uint8_t src, uint8_t dst)
{
    if (fb::game::inventory<const fb::model::spell>::swap(src, dst) == false)
        return false;

    auto listener = this->owner().get_listener<fb::game::spells>();
    if (listener != nullptr)
    {
        const auto right = this->at(src);
        if (right != nullptr)
            listener->on_spell_update(this->owner(), src);
        else
            listener->on_spell_remove(this->owner(), src);

        const auto left = this->at(dst);
        if (left != nullptr)
            listener->on_spell_update(this->owner(), dst);
        else
            listener->on_spell_remove(this->owner(), dst);
    }

    return true;
}

fb::game::buff::buff(const fb::game::context& context, const fb::model::spell& model, uint32_t seconds) :
    context(context),
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
    return this->contains(model.id);
}

bool fb::game::buffs::push_back(buff& buff)
{
    auto& model = buff.model;
    if (this->contains(model.id))
        return false;

    this->insert({model.id, &buff});
    return true;
}

async::task<fb::game::buff*> fb::game::buffs::push_back(const fb::model::spell& model, uint32_t seconds)
{
    if (this->contains(model.id))
        co_return nullptr;

    auto& context = this->_owner.context;
    auto  created = context.make<fb::game::buff>(model, seconds);
    if (this->push_back(*created) == false)
    {
        co_await context.destroy(*created);
        co_return nullptr;
    }
    else
    {
        co_return created;
    }
}

bool fb::game::buffs::remove(uint32_t id)
{
    auto buff = this->operator[] (id);
    if (buff == nullptr)
        return false;

    auto listener = this->_owner.get_listener<fb::game::object>();
    if (listener != nullptr)
        listener->on_unbuff(this->_owner, *buff);

    this->erase(id);
    this->_owner.context.destroy(*buff);
    return true;
}

bool fb::game::buffs::remove(const fb::model::spell& spell)
{
    return this->remove(spell.id);
}

fb::game::buff* fb::game::buffs::operator[] (uint32_t id) const
{
    if (this->contains(id) == false)
        return nullptr;

    return std::unordered_map<uint32_t, buff*>::at(id);
}
