#include <fb/game/character.h>
#include <fb/game/map.h>
#include <fb/game/mob.h>
#include <fb/game/sector.h>

using namespace fb::game;

sector::sector(uint32_t id, const std::function<void(sector&)>& state_changed) :
    _id(id),
    _state_changed(state_changed)
{ }

sector::~sector()
{ }

void sector::push(object& object)
{
    this->push_back(&object);
    if (object.is(OBJECT_TYPE::CHARACTER))
    {
        if (this->_activated)
            return;

        this->_activated = true;
        this->_state_changed(*this);
    }
}

// sector
void sector::erase(object& object)
{
    auto found = std::find(this->begin(), this->end(), &object);
    if (found == this->end())
        return;

    std::vector<fb::game::object*>::erase(found);

    // 섹터에서 세션을 지워낸 경우 active 상태를 검사한다.
    if (object.is(OBJECT_TYPE::CHARACTER))
    {
        this->_activated = std::find_if(this->begin(), this->end(), [](auto x) {
                               return x->is(OBJECT_TYPE::CHARACTER);
                           }) != this->end();

        if (this->_activated == false)
            this->_state_changed(*this);
    }
}

uint32_t sector::id() const
{
    return this->_id;
}

bool sector::activated() const
{
    return this->_activated;
}

// sectors
sectors::sectors(const fb::model::size16_t& map_size, const fb::model::size16_t& size) :
    _map_size(map_size),
    _size(size),
    _rows((map_size.height / size.height) + ((map_size.height % size.height) ? 1 : 0)),
    _columns((map_size.width / size.width) + ((map_size.width % size.width) ? 1 : 0)),
    _count(_rows * _columns)
{
    auto& cache    = this->_activated_cache;
    auto  callback = [&cache](sector& x) {
        if (x.activated())
            cache[x.id()] = &x;
        else
            cache.erase(x.id());
    };
    for (auto i = 0; i < this->_count; i++)
        this->_pool.push_back(std::make_unique<sector>(i, callback));
}

uint32_t sectors::index(const fb::model::point16_t& position) const
{
    auto x = std::max(0, std::min((int)position.x, this->_map_size.width - 1));
    auto y = std::max(0, std::min((int)position.y, this->_map_size.height - 1));
    return (y / this->_size.height) * this->_columns + (x / this->_size.width);
}

std::set<sector*> sectors::activated_sectors() const
{
    auto sectors = std::set<sector*>();
    for (auto& x : this->_activated_cache)
    {
        auto&& nears = this->nears(x.first);
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

uint32_t sectors::push(object& object)
{
    auto index  = this->index(object.position());
    auto sector = this->at(index);
    object.sector(sector);

    return index;
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

std::vector<object*> sectors::activated_objects(OBJECT_TYPE type) const
{
    auto result = std::vector<object*>();
    for (auto& sector : this->activated_sectors())
    {
        std::copy_if(sector->begin(), sector->end(), std::back_inserter(result), [type](auto x) {
            return type == OBJECT_TYPE::UNKNOWN || x->is(type);
        });
    }

    return std::move(result);
}

bool sectors::activated() const
{
    return this->_activated_cache.size() > 0;
}
