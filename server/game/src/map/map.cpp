#include <fb/game/server.h>
#include <fb/game/map.h>
#include <fb/game/thread_params.h>
#include <fb/logger.h>
#include <algorithm>

using namespace fb::game;
using table = fb::model::table;

map::map(fb::game::server&     server,
         uint32_t              id,
         const fb::model::map& model,
         bool                  active,
         const void*           data,
         size_t                size) :
    id(id),
    server(server),
    model(model),
    active(active),
    doors(*this)
{
    if (this->active == false)
        return;

    if (data == nullptr || size == 0)
        return;

    this->load_tiles(data, size);
}

void map::load_tiles(const void* data, size_t size)
{
    if (this->loaded())
        return;

    auto stream = fb::stream((uint8_t*)data, size);
    auto reader = fb::stream_reader<big_endian>(stream);

    this->_size.width = reader.read<uint16_t>();
    if (this->_size.width == 0)
        throw std::runtime_error(std::format(_TEXT(MESSAGE_MAP_INVALID_DATA), this->model.name));

    this->_size.height = reader.read<uint16_t>();
    if (this->_size.height == 0)
        throw std::runtime_error(std::format(_TEXT(MESSAGE_MAP_INVALID_DATA), this->model.name));

    uint32_t map_size = this->_size.width * this->_size.height;
    this->_tiles      = std::make_unique<tile[]>(map_size);
    if (this->_tiles == nullptr)
        throw std::runtime_error(std::format(_TEXT(MESSAGE_MAP_TILE_ALLOCATION_FAILED), this->model.name));

    for (uint32_t i = 0; i < map_size; i++)
    {
        this->_tiles[i].id     = reader.read<uint16_t>();
        this->_tiles[i].object = reader.read<uint16_t>();
    }

    this->sectors.load(this->_size, fb::model::size16_t(MAX_SCREEN_WIDTH, MAX_SCREEN_HEIGHT));
    this->update_door();
}

void map::copy_tiles(const fb::game::map& source)
{
    if (this->loaded())
        return;

    if (source.loaded() == false || source._tiles == nullptr)
        throw std::runtime_error(std::format(_TEXT(MESSAGE_MAP_INVALID_DATA), this->model.name));

    this->_size   = source._size;
    auto map_size = static_cast<uint32_t>(this->_size.width) * static_cast<uint32_t>(this->_size.height);
    this->_tiles  = std::make_unique<tile[]>(map_size);
    if (this->_tiles == nullptr)
        throw std::runtime_error(std::format(_TEXT(MESSAGE_MAP_TILE_ALLOCATION_FAILED), this->model.name));

    std::copy_n(source._tiles.get(), map_size, this->_tiles.get());

    this->sectors.load(this->_size, fb::model::size16_t(MAX_SCREEN_WIDTH, MAX_SCREEN_HEIGHT));
    this->update_door();
}

map::~map()
{ }

void map::update_door()
{
    auto pivot = fb::model::point16_t{0, 0};
    while (pivot.y < this->_size.height)
    {
        pivot.x = 0;
        while (pivot.x < this->_size.width)
        {
            auto found  = false;
            auto opened = false;
            for (auto& [id, door] : table::door)
            {
                if (door.matched(*this, pivot, &opened))
                {
                    auto ptr = std::make_shared<fb::game::door>(*this, door, pivot, opened);
                    this->doors.add(ptr);

                    found    = true;
                    pivot.x += door.width;
                    break;
                }
            }

            if (!found)
            {
                pivot.x++;
            }
        }
        pivot.y++;
    }
}

uint64_t map::index(const fb::model::point16_t& p) const
{
    return (uint64_t)p.y * (uint64_t)this->_size.width + (uint64_t)p.x;
}

fb::model::point16_t map::point(uint64_t i) const
{
    auto y = uint16_t(i / this->_size.width);
    auto x = uint16_t(i % this->_size.width);

    return fb::model::point16_t(x, y);
}

bool map::blocked(uint16_t x, uint16_t y) const
{
    if (x >= this->_size.width)
        return true;

    if (y >= this->_size.height)
        return true;

    return this->_tiles[y * this->_size.width + x].blocked;
}

bool map::block(uint16_t x, uint16_t y, bool option)
{
    if (this->_tiles == nullptr)
        return false;

    if (x >= this->_size.width)
        return false;

    if (y >= this->_size.height)
        return false;

    this->_tiles[y * this->_size.width + x].blocked = option;
    return true;
}

// map::EFFECT map::effect() const
// {
//     return this->_effect;
// }

// MAP_OPTION map::option() const
// {
//     return this->_option;
// }

uint16_t map::width() const
{
    return this->_size.width;
}

uint16_t map::height() const
{
    return this->_size.height;
}

fb::model::size16_t map::size() const
{
    return this->_size;
}

// uint8_t map::bgm() const
// {
//     return this->_bgm;
// }

bool map::loaded() const
{
    return this->_size.width > 0 && this->_size.height > 0;
}

bool map::in_ground(const fb::model::point16_t position) const
{
    return position.x < this->_size.width && position.y < this->_size.height;
}

bool map::movable(const fb::model::point16_t& position, const std::function<bool(const object&)>& predicate) const
{
    if (this->in_ground(position) == false)
        return false;

    if ((*this)(position.x, position.y)->blocked)
        return false;

    auto index = this->index(position);
    if (this->doors.contains(index) && this->doors.at(index)->opened() == false)
        return false;

    if (this->sectors.loaded())
    {
        if (auto sector = this->sectors.at(position))
        {
            for (const auto& obj : *sector)
            {
                if (obj->position() != position)
                    continue;

                if (obj->is(OBJECT_TYPE::ITEM))
                    continue;

                if (predicate(*obj) == false)
                    continue;

                return false;
            }
        }

        return true;
    }

    for (const auto& [key, value] : this->objects)
    {
        if (value->is(OBJECT_TYPE::ITEM))
            continue;

        if (predicate(*value) == false)
            continue;

        if (value->position() == position)
            return false;
    }

    return true;
}

bool map::movable(const object& object, const fb::model::point16_t position) const
{
    return this->movable(position, [&object](const auto& x) {
        return !x.hidden(object);
    });
}

bool map::movable(const object& object, DIRECTION direction) const
{
    fb::model::point16_t position = object.position();

    switch (direction)
    {
    case DIRECTION::BOTTOM:
        position.y++;
        break;

    case DIRECTION::TOP:
        position.y--;
        break;

    case DIRECTION::LEFT:
        position.x--;
        break;

    case DIRECTION::RIGHT:
        position.x++;
        break;
    }

    if (this->movable(object, position) == false)
        return false;

    return true;
}

bool map::movable_forward(const object& object, uint16_t step) const
{
    return this->movable(object, object.direction());
}

const fb::model::warp* map::warpable(const fb::model::point16_t& position) const
{
    auto& warps = table::warp;
    if (warps.contains(this->model.id) == false)
        return nullptr;

    for (auto& warp : warps[this->model.id])
    {
        if (warp.before == position)
            return &warp;
    }

    return nullptr;
}

bool map::is_active() const
{
    if (this->sectors.loaded() == false)
        return false;

    if (this->objects.size() == 0)
        return false;

    return this->sectors.any_occupied();
}

std::vector<std::shared_ptr<fb::game::object>> map::nears(const fb::model::point16_t& pivot, OBJECT_TYPE type) const
{
    if (this->sectors.loaded() == false)
        return std::vector<std::shared_ptr<fb::game::object>>{};

    return this->sectors.objects(pivot, type);
}

std::vector<std::shared_ptr<fb::game::object>> map::belows(const fb::model::point16_t& pivot, OBJECT_TYPE type) const
{
    auto objects = std::vector<std::shared_ptr<fb::game::object>>();
    try
    {
        auto sector = this->sectors.at(pivot);
        if (sector == nullptr)
            throw std::exception();

        for (auto& obj : *sector)
        {
            if (obj->is(type))
            {
                if (obj->position() == pivot)
                    objects.push_back(obj);
            }
        }
    }
    catch (std::exception&)
    { }

    std::reverse(objects.begin(), objects.end());
    return objects;
}

async::task<void> map::bulk_update(const std::vector<uint32_t>& oids)
{
    if (oids.empty())
        co_return;

    // Use unordered_set for better performance (O(1) vs O(log n))
    auto affected = std::unordered_set<const sector*>();
    auto targets  = std::unordered_set<std::shared_ptr<fb::game::character>>();
    auto oid_set  = std::unordered_set<uint32_t>(oids.begin(), oids.end());

    // Collect sectors containing changed objects
    for (auto oid : oids)
    {
        auto obj = this->objects.at(oid);
        if (obj == nullptr)
            continue;

        for (const sector* sector : this->sectors.nears(obj->position()))
            affected.insert(sector);
    }

    // Collect all characters in affected sectors
    for (const auto& sector : affected)
    {
        for (const auto& obj : *sector)
        {
            if (obj->is(OBJECT_TYPE::CHARACTER))
                targets.insert(std::static_pointer_cast<fb::game::character>(obj));
        }
    }

    // Send updates to each character
    for (const auto& target : targets)
    {
        auto changed_objects = std::vector<object*>();
        changed_objects.reserve(oids.size()); // Pre-allocate for efficiency

        for (const auto& nearby_obj : target->nears())
        {
            if (oid_set.contains(nearby_obj->oid()))
            {
                changed_objects.push_back(nearby_obj.get());
            }
        }

        if (!changed_objects.empty())
        {
            try
            {
                co_await target->bulk_objects_update(changed_objects);
            }
            catch (const std::exception& e)
            {
                fb::logger::fatal("bulk_update failed for {}: {}", target->name(), e.what());
            }
        }
    }
    co_return;
}

void map::rezen_force() const
{
    auto thread = this->thread();
    if (thread == nullptr)
        return;

    auto builder = thread->new_builder<void>();
    builder.func = [map_id = this->id](auto& thread) -> async::task<void> {
        auto params = thread.template data<thread_params>();
        for (auto& rezen : params->rezens)
        {
            if (rezen->map_id() == map_id)
                rezen->force_spawn(thread.id());
        }
        co_return;
    };
    builder.enqueue();
}

fb::thread* map::thread() const
{
    return this->server.threads.modular(this->id);
}

bool map::is_instance() const
{
    return false;
}

std::shared_ptr<fb::game::map> map::source() const
{
    return nullptr;
}

uint32_t map::slot() const
{
    return 0;
}

bool map::closing() const
{
    return false;
}

void map::on_character_enter()
{
    this->_character_count.fetch_add(1, std::memory_order_relaxed);
}

void map::on_character_leave()
{
    auto previous = this->_character_count.load(std::memory_order_relaxed);
    while (previous > 0)
    {
        if (this->_character_count.compare_exchange_weak(previous, previous - 1, std::memory_order_relaxed))
            return;
    }
}

bool map::begin_destroy()
{
    return false;
}

uint32_t map::character_count() const
{
    return this->_character_count.load(std::memory_order_relaxed);
}

map::tile* map::operator() (uint16_t x, uint16_t y) const
{
    if (x >= this->_size.width)
        return nullptr;

    if (y >= this->_size.height)
        return nullptr;

    auto i = this->index(fb::model::point16_t(x, y));
    return &this->_tiles[i];
}