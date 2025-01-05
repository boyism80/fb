#include <fb/game/character.h>
#include <fb/game/map.h>
#include <fb/game/mob.h>
#include <fb/game/sector.h>

using namespace fb::game;

sector::sector(uint32_t id) :
    _id(id)
{ }

sector::~sector()
{ }

void sector::push(object& object)
{
    this->push_back(&object);
    if (object.is(OBJECT_TYPE::CHARACTER))
        this->_character_count++;
}

void sector::erase(object& object)
{
    auto found = std::find(this->begin(), this->end(), &object);
    if (found == this->end())
        return;

    std::vector<fb::game::object*>::erase(found);
    if (object.is(OBJECT_TYPE::CHARACTER))
        this->_character_count--;
}

uint32_t sector::id() const
{
    return this->_id;
}

bool sector::is_active() const
{
    return this->_character_count > 0;
}

sectors::sectors(const fb::model::size16_t& map_size, const fb::model::size16_t& size) :
    _map_size(map_size),
    _size(size),
    _rows((map_size.height / size.height) + ((map_size.height % size.height) ? 1 : 0)),
    _columns((map_size.width / size.width) + ((map_size.width % size.width) ? 1 : 0)),
    _count(_rows * _columns)
{
    for (auto i = 0; i < this->_count; i++)
    {
        this->_pool.push_back(std::make_unique<sector>(i));
    }
}

uint32_t sectors::index(const fb::model::point16_t& position) const
{
    auto x = std::max(0, std::min((int)position.x, this->_map_size.width - 1));
    auto y = std::max(0, std::min((int)position.y, this->_map_size.height - 1));
    return (y / this->_size.height) * this->_columns + (x / this->_size.width);
}

std::set<sector*> sectors::active_sectors() const
{
    auto sectors = std::set<sector*>();
    for (auto& sector_ptr : this->_pool)
    {
        if (sector_ptr->is_active() == false)
            continue;

        auto&& nears = this->nears(sector_ptr->id());
        sectors.insert(nears.begin(), nears.end());
    }

    return sectors;
}

sector* sectors::at(const fb::model::point16_t& position) const
{
    auto index = this->index(position);
    return this->at(index);
}

sector* sectors::at(uint32_t index) const
{
    if (index > this->_pool.size() - 1)
        return nullptr;

    return this->_pool[index].get();
}

std::vector<sector*> sectors::nears(uint32_t index) const
{
    auto sectors = std::vector<sector*>();
    auto center  = this->at(index);
    if (center == nullptr)
        throw std::runtime_error("center sector cannot be null");
    sectors.push_back(center);

    auto is_left = index % this->_columns == 0;
    if (!is_left)
    {
        auto sector = this->at(index - 1);
        if (sector != nullptr)
            sectors.push_back(sector);
    }

    auto is_right = index % this->_columns == this->_columns - 1;
    if (!is_right)
    {
        auto sector = this->at(index + 1);
        if (sector != nullptr)
            sectors.push_back(sector);
    }

    auto is_top = index < this->_columns;
    if (!is_top)
    {
        auto sector = this->at(index - this->_columns);
        if (sector != nullptr)
            sectors.push_back(sector);
    }

    auto is_bottom = index > this->_columns * (this->_rows - 1) - 1;
    if (!is_bottom)
    {
        auto sector = this->at(index + this->_columns);
        if (sector != nullptr)
            sectors.push_back(sector);
    }

    if (!is_left && !is_top)
    {
        auto sector = this->at(index - this->_columns - 1);
        if (sector != nullptr)
            sectors.push_back(sector);
    }

    if (!is_right && !is_top)
    {
        auto sector = this->at(index - this->_columns + 1);
        if (sector != nullptr)
            sectors.push_back(sector);
    }

    if (!is_left && !is_bottom)
    {
        auto sector = this->at(index + this->_columns - 1);
        if (sector != nullptr)
            sectors.push_back(sector);
    }

    if (!is_right && !is_bottom)
    {
        auto sector = this->at(index + this->_columns + 1);
        if (sector != nullptr)
            sectors.push_back(sector);
    }

    return std::move(sectors);
}

std::vector<sector*> sectors::nears(const fb::model::point16_t& pivot) const
{
    return this->nears(this->index(pivot));
}

std::vector<object*> sectors::objects(const fb::model::point16_t& pivot, OBJECT_TYPE type) const
{
    auto&& sectors = this->nears(pivot);
    auto   objects = std::vector<object*>();
    for (auto sector : sectors)
    {
        for (auto obj : *sector)
        {
            if (obj->available())
                objects.push_back(obj);
        }
    }

    if (type == OBJECT_TYPE::UNKNOWN)
    {
        return std::move(objects);
    }
    else
    {
        auto filtered = std::vector<object*>();
        std::copy_if(objects.begin(), objects.end(), std::back_inserter(filtered), [type](auto x) {
            return x->is(type);
        });
        return std::move(filtered);
    }
}

std::vector<object*> sectors::objects(OBJECT_TYPE type) const
{
    auto result = std::vector<object*>();
    for (auto& sector : this->active_sectors())
    {
        std::copy_if(sector->begin(), sector->end(), std::back_inserter(result), [type](auto x) {
            return type == OBJECT_TYPE::UNKNOWN || x->is(type);
        });
    }

    return std::move(result);
}

bool sectors::is_active() const
{
    for (auto& sector_ptr : this->_pool)
    {
        if (sector_ptr->is_active())
            return true;
    }

    return false;
}
