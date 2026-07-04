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

void sector::push(std::shared_ptr<fb::game::object> object)
{
    this->push_back(object);
    if (object->is(OBJECT_TYPE::CHARACTER))
        this->_character_count++;
}

void sector::erase(std::shared_ptr<fb::game::object> object)
{
    object->assert_thread();

    for (auto it = this->begin(); it != this->end(); ++it)
    {
        if (*it == object)
        {
            super::erase(it);
            break;
        }
    }

    if (object->is(OBJECT_TYPE::CHARACTER))
        this->_character_count--;
}

uint32_t sector::id() const
{
    return this->_id;
}

bool sector::occupied() const
{
    return this->_character_count > 0;
}

sector::container::container(fb::game::map& map) :
    _map(map)
{ }

sector::container::iterator sector::container::begin()
{
    return this->_pool.begin();
}

sector::container::iterator sector::container::end()
{
    return this->_pool.end();
}

sector::container::const_iterator sector::container::begin() const
{
    return this->_pool.begin();
}

sector::container::const_iterator sector::container::end() const
{
    return this->_pool.end();
}

size_t sector::container::size() const
{
    return this->_pool.size();
}

bool sector::container::loaded() const
{
    return this->_pool.empty() == false;
}

void sector::container::load(const fb::model::size16_t& map_size, const fb::model::size16_t& cell_size)
{
    this->_map_size  = map_size;
    this->_cell_size = cell_size;
    this->_rows      = (map_size.height / cell_size.height) + ((map_size.height % cell_size.height) ? 1 : 0);
    this->_columns   = (map_size.width / cell_size.width) + ((map_size.width % cell_size.width) ? 1 : 0);

    const auto count = this->_rows * this->_columns;
    this->_pool.clear();
    this->_pool.reserve(count);

    for (uint32_t i = 0; i < count; i++)
        this->_pool.emplace_back(i);
}

bool sector::container::activated(uint32_t index) const
{
    for (const sector* sector_ptr : this->nears(index))
    {
        if (sector_ptr->occupied())
            return true;
    }

    return false;
}

bool sector::container::activated(const sector& sector_ref) const
{
    return this->activated(sector_ref.id());
}

bool sector::container::any_occupied() const
{
    for (const auto& sector_ref : this->_pool)
    {
        if (sector_ref.occupied())
            return true;
    }

    return false;
}

std::vector<const sector*> sector::container::actives() const
{
    auto result = std::vector<const sector*>();
    if (this->_pool.empty())
        return result;

    auto visited = std::vector<bool>(this->_pool.size(), false);
    for (const auto& sector_ref : this->_pool)
    {
        if (sector_ref.occupied() == false)
            continue;

        for (const sector* neighbor : this->nears(sector_ref.id()))
        {
            if (visited[neighbor->id()])
                continue;

            visited[neighbor->id()] = true;
            result.push_back(neighbor);
        }
    }

    return result;
}

uint32_t sector::container::index(const fb::model::point16_t& position) const
{
    auto x = std::max(0, std::min((int)position.x, this->_map_size.width - 1));
    auto y = std::max(0, std::min((int)position.y, this->_map_size.height - 1));
    return (y / this->_cell_size.height) * this->_columns + (x / this->_cell_size.width);
}

sector* sector::container::at(const fb::model::point16_t& position)
{
    return this->at(this->index(position));
}

const sector* sector::container::at(const fb::model::point16_t& position) const
{
    return this->at(this->index(position));
}

sector* sector::container::at(uint32_t index)
{
    if (index > this->_pool.size() - 1)
        return nullptr;

    return &this->_pool[index];
}

const sector* sector::container::at(uint32_t index) const
{
    if (index > this->_pool.size() - 1)
        return nullptr;

    return &this->_pool[index];
}

std::vector<const sector*> sector::container::nears(uint32_t index) const
{
    auto result = std::vector<const sector*>();
    auto center = this->at(index);
    if (center == nullptr)
        throw std::runtime_error("center sector cannot be null");

    result.push_back(center);

    auto is_left = index % this->_columns == 0;
    if (!is_left)
    {
        if (auto sector_ptr = this->at(index - 1))
            result.push_back(sector_ptr);
    }

    auto is_right = index % this->_columns == this->_columns - 1;
    if (!is_right)
    {
        if (auto sector_ptr = this->at(index + 1))
            result.push_back(sector_ptr);
    }

    auto is_top = index < this->_columns;
    if (!is_top)
    {
        if (auto sector_ptr = this->at(index - this->_columns))
            result.push_back(sector_ptr);
    }

    auto is_bottom = index > this->_columns * (this->_rows - 1) - 1;
    if (!is_bottom)
    {
        if (auto sector_ptr = this->at(index + this->_columns))
            result.push_back(sector_ptr);
    }

    if (!is_left && !is_top)
    {
        if (auto sector_ptr = this->at(index - this->_columns - 1))
            result.push_back(sector_ptr);
    }

    if (!is_right && !is_top)
    {
        if (auto sector_ptr = this->at(index - this->_columns + 1))
            result.push_back(sector_ptr);
    }

    if (!is_left && !is_bottom)
    {
        if (auto sector_ptr = this->at(index + this->_columns - 1))
            result.push_back(sector_ptr);
    }

    if (!is_right && !is_bottom)
    {
        if (auto sector_ptr = this->at(index + this->_columns + 1))
            result.push_back(sector_ptr);
    }

    return result;
}

std::vector<const sector*> sector::container::nears(const fb::model::point16_t& pivot) const
{
    return this->nears(this->index(pivot));
}

std::vector<std::shared_ptr<fb::game::object>> sector::container::objects(const fb::model::point16_t& pivot,
                                                                          OBJECT_TYPE                 type) const
{
    auto   result       = std::vector<std::shared_ptr<fb::game::object>>();
    auto&& near_sectors = this->nears(pivot);
    for (const sector* sector_ptr : near_sectors)
    {
        for (auto& obj : *sector_ptr)
        {
            if (obj->available())
                result.push_back(obj);
        }
    }

    if (type == OBJECT_TYPE::OBJECT)
        return result;

    auto filtered = std::vector<std::shared_ptr<fb::game::object>>();
    std::copy_if(result.begin(),
                 result.end(),
                 std::back_inserter(filtered),
                 [type](const std::shared_ptr<fb::game::object>& x) {
                     return x->is(type);
                 });
    return filtered;
}

std::vector<std::shared_ptr<fb::game::object>> sector::container::objects(OBJECT_TYPE type) const
{
    auto result = std::vector<std::shared_ptr<fb::game::object>>();
    for (const sector* sector_ptr : this->actives())
    {
        std::copy_if(sector_ptr->begin(),
                     sector_ptr->end(),
                     std::back_inserter(result),
                     [type](const std::shared_ptr<fb::game::object>& x) {
                         return x->is(type);
                     });
    }

    return result;
}
