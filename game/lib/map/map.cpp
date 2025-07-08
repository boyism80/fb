#include <fb/game/context.h>
#include <fb/game/map.h>

using namespace fb::game;

map::map(fb::game::context& context, const fb::model::map& model, bool active, const void* data, size_t size) :
    context(context),
    model(model),
    active(active),
    doors(*this)
{
    if (this->active == false)
        return;

    auto what   = std::string();
    auto stream = fb::stream((uint8_t*)data, size);
    auto reader = fb::stream_reader<big_endian>(stream);

    this->_size.width = reader.read<uint16_t>();
    if (this->_size.width == 0)
        throw std::runtime_error(std::format("맵 데이터가 올바르지 않습니다. ({})", model.name));

    this->_size.height = reader.read<uint16_t>();
    if (this->_size.height == 0)
        throw std::runtime_error(std::format("맵 데이터가 올바르지 않습니다. ({})", model.name));

    uint32_t map_size = this->_size.width * this->_size.height;
    this->_tiles      = std::make_unique<tile[]>(map_size);
    if (this->_tiles == nullptr)
        throw std::runtime_error(std::format("맵 타일 메모리를 할당할 수 없습니다. ({})", model.name));

    for (uint32_t i = 0; i < map_size; i++)
    {
        this->_tiles[i].id     = reader.read<uint16_t>();
        this->_tiles[i].object = reader.read<uint16_t>();
    }

    this->_sectors = std::make_unique<sectors>(this->_size, fb::model::size16_t(MAX_SCREEN_WIDTH, MAX_SCREEN_HEIGHT));
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
            for (auto& [id, door] : this->context.model.door)
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
        return !object.hidden(x);
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
    auto& warps = this->context.model.warp;
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
    if (this->_sectors == nullptr)
        return false;

    if (this->objects.size() == 0)
        return false;

    return this->_sectors->is_active();
}

std::shared_ptr<fb::game::sector> map::sector_at(const fb::model::point16_t& position)
{
    if (this->_sectors == nullptr)
        return nullptr;

    return this->_sectors->at(position);
}

std::vector<std::shared_ptr<fb::game::object>> map::nears(const fb::model::point16_t& pivot, OBJECT_TYPE type) const
{
    if (this->_sectors == nullptr)
        return std::vector<std::shared_ptr<fb::game::object>>{};
    else
        return this->_sectors->objects(pivot, type);
}

std::vector<std::shared_ptr<fb::game::object>> map::belows(const fb::model::point16_t& pivot, OBJECT_TYPE type) const
{
    auto objects = std::vector<std::shared_ptr<fb::game::object>>();
    try
    {
        if (this->_sectors == nullptr)
            throw std::exception();

        auto sector = this->_sectors->at(pivot);
        std::copy_if(sector->begin(), sector->end(), std::back_inserter(objects), [type, &pivot](auto x) {
            return (type == OBJECT_TYPE::UNKNOWN || x->is(type)) && x->position() == pivot;
        });

        std::sort(objects.begin(), objects.end(), [](auto obj1, auto obj2) {
            return obj1->oid() > obj2->oid();
        });
    }
    catch (std::exception&)
    { }

    return std::move(objects);
}

void map::bulk_update(const std::vector<uint32_t>& oids)
{
    if (oids.empty())
        return;

    // Use unordered_set for better performance (O(1) vs O(log n))
    auto sectors = std::unordered_set<std::shared_ptr<fb::game::sector>>();
    auto targets = std::unordered_set<std::shared_ptr<fb::game::character>>();
    auto oid_set = std::unordered_set<uint32_t>(oids.begin(), oids.end());

    // Collect sectors containing changed objects
    for (auto oid : oids)
    {
        auto obj = this->objects.at(oid);
        if (obj == nullptr)
            continue;

        for (const auto& sector : this->_sectors->nears(obj->position()))
        {
            sectors.insert(sector);
        }
    }

    // Collect all characters in affected sectors
    for (const auto& sector : sectors)
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
            target->send(fb::protocol::game::response::update(changed_objects));
        }
    }
}

fb::thread* map::thread() const
{
    return this->context.threads.modular(this->model.id);
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