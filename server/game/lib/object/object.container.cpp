#include <fb/game/object.h>
#include <fb/thread.h>

using namespace fb::game;

object::container::container(fb::game::map& map) :
    owner(map)
{ }

uint32_t object::container::allocate_seq()
{
    // Use reusable sequence if available
    if (!this->_available_seq.empty())
    {
        auto seq = this->_available_seq.front();
        this->_available_seq.pop();
        return seq;
    }

    // Generate new sequence if no reusable sequence available
    for (int i = this->_oid; i < 0xFFFF; i++)
    {
        if (this->_ptrs.find(i) != this->_ptrs.end())
            continue;

        this->_oid = i + 1;
        return i;
    }

    for (int i = 1; i < this->_oid; i++)
    {
        if (this->_ptrs.find(i) != this->_ptrs.end())
            continue;

        this->_oid = i + 1;
        return i;
    }

    return 0xFFFF;
}

object::container::iterator object::container::begin()
{
    return this->_ptrs.begin();
}

object::container::iterator object::container::end()
{
    return this->_ptrs.end();
}

object::container::const_iterator object::container::begin() const
{
    return this->_ptrs.cbegin();
}

object::container::const_iterator object::container::end() const
{
    return this->_ptrs.cend();
}

uint32_t object::container::size() const
{
    return this->_ptrs.size();
}

std::shared_ptr<fb::game::object> object::container::at(uint32_t i)
{
    return this->_ptrs.at(i);
}

void object::container::push(fb::game::object& obj)
{
    auto seq = this->allocate_seq();
    obj.oid(seq);

    this->_ptrs.insert({seq, std::static_pointer_cast<fb::game::object>(obj.shared_from_this())});
}

fb::game::object& object::container::pop(uint32_t seq)
{
    auto raw = this->try_pop(seq);
    if (raw == nullptr)
        throw std::runtime_error("empty ptr");

    return *raw;
}

fb::game::object& object::container::pop(fb::game::object& obj)
{
    return this->pop(obj.oid());
}

fb::game::object* object::container::try_pop(uint32_t seq)
{
    if (this->_ptrs.find(seq) == this->_ptrs.end())
        return nullptr;

    auto& ptr    = this->_ptrs.at(seq);
    auto  raw    = ptr.get();
    auto  thread = raw->thread();

    ptr.reset();
    this->_ptrs.erase(seq);

    // Add removed sequence to reuse queue
    async::awaitable_then(thread->sleep(5s), [this, seq](auto result) {
        this->_available_seq.push(seq);
    });

    return raw;
}

fb::game::object* object::container::try_pop(fb::game::object& obj)
{
    return this->try_pop(obj.oid());
}

fb::game::object* object::container::operator[] (uint32_t seq)
{
    if (this->_ptrs.find(seq) == this->_ptrs.end())
        return nullptr;

    auto& ptr = this->_ptrs.at(seq);
    return ptr.get();
}

void object::container::foreach (OBJECT_TYPE type, const object::container::handle_predicate_type& fn)
{
    for (auto& [seq, obj] : *this)
    {
        if (obj->is(type) == false)
            continue;

        if (fn(*obj) == false)
            break;
    }
}

bool object::container::contains(const std::shared_ptr<fb::game::object>& x) const
{
    for (auto& [seq, obj] : *this)
    {
        if (obj == x)
            return true;
    }

    return false;
}

bool object::container::contains(uint32_t fd) const
{
    for (auto& [seq, obj] : *this)
    {
        if (obj->oid() == fd)
            return true;
    }

    return false;
}