#include <fb/game/context.h>
#include <fb/game/map.h>

using namespace fb::game;

fb::game::map::map(fb::game::context&    context,
                   const fb::model::map& model,
                   bool                  active,
                   const void*           data,
                   size_t                size) :
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

    this->_sectors = std::make_unique<sectors>(this->_size, size16_t(MAX_SCREEN_WIDTH, MAX_SCREEN_HEIGHT));
    this->update_door();
}

fb::game::map::~map()
{ }

void fb::game::map::update_door()
{
    auto pivot = point16_t{0, 0};
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
                    for (uint16_t i = 0; i < door.width; i++)
                    {
                        this->doors.add(point16_t{uint16_t(pivot.x + i), uint16_t(pivot.y)}, pivot, door, opened);
                    }
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

uint64_t fb::game::map::index(const point16_t& p) const
{
    return (uint64_t)p.y * (uint64_t)this->_size.width + (uint64_t)p.x;
}

point16_t fb::game::map::point(uint64_t i) const
{
    auto y = uint16_t(i / this->_size.width);
    auto x = uint16_t(i % this->_size.width);

    return point16_t(x, y);
}

bool fb::game::map::blocked(uint16_t x, uint16_t y) const
{
    if (x >= this->_size.width)
        return true;

    if (y >= this->_size.height)
        return true;

    return this->_tiles[y * this->_size.width + x].blocked;
}

bool fb::game::map::block(uint16_t x, uint16_t y, bool option)
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

// fb::game::map::EFFECT fb::game::map::effect() const
// {
//     return this->_effect;
// }

// MAP_OPTION fb::game::map::option() const
// {
//     return this->_option;
// }

uint16_t fb::game::map::width() const
{
    return this->_size.width;
}

uint16_t fb::game::map::height() const
{
    return this->_size.height;
}

size16_t fb::game::map::size() const
{
    return this->_size;
}

// uint8_t fb::game::map::bgm() const
// {
//     return this->_bgm;
// }

bool fb::game::map::loaded() const
{
    return this->_size.width > 0 && this->_size.height > 0;
}

bool fb::game::map::existable(const point16_t position) const
{
    return position.x >= 0 && position.y >= 0 && position.x < this->_size.width && position.y < this->_size.height;
}

bool fb::game::map::movable(const point16_t position) const
{
    if (this->existable(position) == false)
        return false;

    if ((*this)(position.x, position.y)->blocked)
        return false;

    for (const auto& [key, value] : this->objects)
    {
        if (value.visible() == false)
            continue;

        if (value.is(OBJECT_TYPE::ITEM))
            continue;

        if (value.position() == position)
            return false;
    }

    return true;
}

bool fb::game::map::movable(const object& object, DIRECTION direction) const
{
    point16_t position = object.position();

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

    if (this->movable(position) == false)
        return false;

    return true;
}

bool fb::game::map::movable_forward(const object& object, uint16_t step) const
{
    return this->movable(object, object.direction());
}

const fb::model::warp* fb::game::map::warpable(const point16_t& position) const
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

bool fb::game::map::update(object& object)
{
    if (this->_sectors == nullptr)
        return false;

    auto sector = this->_sectors->at(object.position());
    if (sector == nullptr)
        return false;

    return object.sector(sector);
}

bool fb::game::map::activated() const
{
    if (this->_sectors == nullptr)
        return false;

    return this->_sectors->activated();
}

std::vector<fb::game::object*> fb::game::map::nears(const point16_t& pivot, OBJECT_TYPE type) const
{
    if (this->_sectors == nullptr)
        return std::vector<fb::game::object*>{};
    else
        return this->_sectors->objects(pivot, type);
}

std::vector<fb::game::object*> fb::game::map::belows(const point16_t& pivot, OBJECT_TYPE type) const
{
    auto objects = std::vector<fb::game::object*>();
    try
    {
        if (this->_sectors == nullptr)
            throw std::exception();

        auto sector = this->_sectors->at(pivot);
        std::copy_if(sector->begin(), sector->end(), std::back_inserter(objects), [type, &pivot](auto x) {
            return (type == OBJECT_TYPE::UNKNOWN || x->is(type)) && x->position() == pivot;
        });
    }
    catch (std::exception&)
    { }

    return std::move(objects);
}

std::vector<fb::game::object*> fb::game::map::activateds(OBJECT_TYPE type)
{
    if (this->_sectors == nullptr)
        return std::vector<fb::game::object*>{};
    else
        return this->_sectors->activated_objects(type);
}

fb::thread* fb::game::map::thread() const
{
    return this->context.threads.modular(this->model.id);
}

fb::game::map::tile* fb::game::map::operator() (uint16_t x, uint16_t y) const
{
    if (x > this->_size.width)
        return nullptr;

    if (y > this->_size.height)
        return nullptr;

    auto i = this->index(fb::model::point16_t(x, y));
    return &this->_tiles[i];
}