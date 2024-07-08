#include <fb/game/map.h>

using namespace fb::game;

objects::objects(fb::game::map& map) : owner(map)
{ }

uint32_t objects::empty_seq()
{
    
    for(int i = this->_sequence; i < 0xFFFF; i++)
    {
        if(this->_refs.contains(i))
            continue;

        this->_sequence = i + 1;
        return i;
    }

    for(int i = 1; i < this->_sequence; i++)
    {
        if(this->_refs.contains(i))
            continue;

        this->_sequence = i + 1;
        return i;
    }

    return 0xFFFF;
}

objects::iterator objects::begin()
{
    return this->_refs.begin();
}

objects::iterator objects::end()
{
    return this->_refs.end();
}

objects::const_iterator objects::begin() const
{
    return this->_refs.cbegin();
}

objects::const_iterator objects::end() const
{
    return this->_refs.cend();
}

uint32_t objects::size() const
{
    return this->_refs.size();
}

fb::game::object& objects::at(uint32_t i)
{
    return this->_refs.at(i);
}

void objects::push(fb::game::object& obj)
{
    auto seq = this->empty_seq();
    obj.sequence(seq);

    this->_ptrs.insert({seq, std::unique_ptr<fb::game::object>(&obj)});
    this->_refs.insert({seq, obj});
}

fb::game::object& objects::pop(uint32_t seq)
{
    auto raw = this->try_pop(seq);
    if(raw == nullptr)
        throw std::runtime_error("empty ptr");

    return *raw;
}

fb::game::object& objects::pop(fb::game::object& obj)
{
    return this->pop(obj.sequence());
}

fb::game::object* objects::try_pop(uint32_t seq)
{
    if(this->_ptrs.contains(seq) == false)
        return nullptr;

    auto& ptr = this->_ptrs.at(seq);
    auto raw = ptr.get();

    ptr.reset();
    this->_ptrs.erase(seq);
    this->_refs.erase(seq);
    return raw;
}

fb::game::object* objects::try_pop(fb::game::object& obj)
{
    return this->try_pop(obj.sequence());
}

fb::game::object* objects::operator [] (uint32_t seq)
{
    if(this->_ptrs.contains(seq) == false)
        return nullptr;

    auto& ptr = this->_ptrs.at(seq);
    return ptr.get();
}

void objects::foreach(OBJECT_TYPE type, const objects::filter_func& fn)
{
    for(auto& [seq, obj] : *this)
    {
        if(obj.is(type) == false)
            continue;

        if(fn(obj) == false)
            break;
    }
}

bool objects::contains(const fb::game::object& x) const
{
    for(auto& [seq, obj] : *this)
    {
        if(obj == x)
            return true;
    }

    return false;
}

bool objects::contains(uint32_t fd) const
{
    for(auto& [seq, obj] : *this)
    {
        if(obj.sequence() == fd)
            return true;
    }

    return false;
}