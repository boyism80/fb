#include <object/container.h>

using namespace fb::game;

object_container::object_container(fb::game::map& map) :
    owner(map)
{ }

uint32_t object_container::empty_seq()
{

    for (int i = this->_sequence; i < 0xFFFF; i++)
    {
        if (this->_refs.contains(i))
            continue;

        this->_sequence = i + 1;
        return i;
    }

    for (int i = 1; i < this->_sequence; i++)
    {
        if (this->_refs.contains(i))
            continue;

        this->_sequence = i + 1;
        return i;
    }

    return 0xFFFF;
}

object_container::iterator object_container::begin()
{
    return this->_refs.begin();
}

object_container::iterator object_container::end()
{
    return this->_refs.end();
}

object_container::const_iterator object_container::begin() const
{
    return this->_refs.cbegin();
}

object_container::const_iterator object_container::end() const
{
    return this->_refs.cend();
}

uint32_t object_container::size() const
{
    return this->_refs.size();
}

fb::game::object& object_container::at(uint32_t i)
{
    return this->_refs.at(i);
}

void object_container::push(fb::game::object& obj)
{
    auto seq = this->empty_seq();
    obj.sequence(seq);

    this->_ptrs.insert({seq, std::unique_ptr<fb::game::object>(&obj)});
    this->_refs.insert({seq, obj});
}

fb::game::object& object_container::pop(uint32_t seq)
{
    auto raw = this->try_pop(seq);
    if (raw == nullptr)
        throw std::runtime_error("empty ptr");

    return *raw;
}

fb::game::object& object_container::pop(fb::game::object& obj)
{
    return this->pop(obj.sequence());
}

fb::game::object* object_container::try_pop(uint32_t seq)
{
    if (this->_ptrs.contains(seq) == false)
        return nullptr;

    auto& ptr = this->_ptrs.at(seq);
    auto  raw = ptr.get();

    ptr.release();
    this->_ptrs.erase(seq);
    this->_refs.erase(seq);
    return raw;
}

fb::game::object* object_container::try_pop(fb::game::object& obj)
{
    return this->try_pop(obj.sequence());
}

fb::game::object* object_container::operator[] (uint32_t seq)
{
    if (this->_ptrs.contains(seq) == false)
        return nullptr;

    auto& ptr = this->_ptrs.at(seq);
    return ptr.get();
}

void object_container::foreach (OBJECT_TYPE type, const object_container::handle_predicate_type& fn)
{
    for (auto& [seq, obj] : *this)
    {
        if (obj.is(type) == false)
            continue;

        if (fn(obj) == false)
            break;
    }
}

bool object_container::contains(const fb::game::object& x) const
{
    for (auto& [seq, obj] : *this)
    {
        if (obj == x)
            return true;
    }

    return false;
}

bool object_container::contains(uint32_t fd) const
{
    for (auto& [seq, obj] : *this)
    {
        if (obj.sequence() == fd)
            return true;
    }

    return false;
}