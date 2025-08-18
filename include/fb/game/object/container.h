#ifndef __OBJECT_CONTAINER_H__
#define __OBJECT_CONTAINER_H__

#include <stdint.h>
#include <string>
#include <stdexcept>
#include <sstream>
#include <zlib.h>
#include <memory>
#include <queue>
#include <fb/game/door.h>
#include <fb/game/sector.h>
#include <fb/stream.h>
#include <fb/config.h>
#include <fb/container.h>
#include <async/awaitable_then.h>

using namespace std::chrono_literals;

namespace fb::game {

class object;
class map;
class mob;
class character;
class server;

class object_container
{
public:
    using ptrs                  = std::unordered_map<uint32_t, std::shared_ptr<object>>;
    using iterator              = ptrs::iterator;
    using const_iterator        = ptrs::const_iterator;
    using handle_predicate_type = std::function<bool(object&)>;

private:
    ptrs                 _ptrs;
    uint32_t             _oid = 1;
    std::queue<uint32_t> _available_seq;

public:
    fb::game::map& owner;

public:
    object_container(fb::game::map& map);
    ~object_container() = default;

private:
    uint32_t allocate_seq();

public:
    iterator                begin();
    iterator                end();
    const_iterator          begin() const;
    const_iterator          end() const;
    uint32_t                size() const;
    std::shared_ptr<object> at(uint32_t i);
    void                    push(object& obj);
    object&                 pop(uint32_t fd);
    object&                 pop(object& obj);
    object*                 try_pop(uint32_t fd);
    object*                 try_pop(object& obj);

    void foreach (OBJECT_TYPE type, const handle_predicate_type& fn);
    bool contains(const std::shared_ptr<object>& obj) const;
    bool contains(uint32_t fd) const;

public:
    object* operator[] (uint32_t fd);
};

} // namespace fb::game

#endif