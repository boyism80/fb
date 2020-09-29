#include "model/map/sector.h"
#include "model/map/map.h"
#include "model/session/session.h"
#include "model/mob/mob.h"

fb::game::sector::sector(uint32_t id, std::function<void(sector&)> state_changed) : 
    _id(id),
    _state_changed(state_changed),
    _activated(false)
{
}

fb::game::sector::~sector()
{
}

void fb::game::sector::push(fb::game::object& object)
{
    this->push_back(&object);
    if(object.is(fb::game::object::types::SESSION))
    {
        if(this->_activated)
            return;

        this->_activated = true;
        this->_state_changed(*this);
    }
}

// sector
void fb::game::sector::erase(fb::game::object& object)
{
    auto found = std::find(this->begin(), this->end(), &object);
    if(found == this->end())
        return;

    std::vector<fb::game::object*>::erase(found);

    // 섹터에서 세션을 지워낸 경우 active 상태를 검사한다.
    if(object.is(fb::game::object::types::SESSION))
    {
        this->_activated = std::find_if
        (
            this->begin(), this->end(),
            [] (fb::game::object* x) 
            {
                return x->is(fb::game::object::SESSION);
            }
        ) != this->end();
        
        if(this->_activated == false)
            this->_state_changed(*this);
    }
}

uint32_t fb::game::sector::id() const
{
    return this->_id;
}

bool fb::game::sector::activated() const
{
    return this->_activated;
}


// sectors
fb::game::sectors::sectors(const size16_t& map_size, const size16_t& size) : 
    _map_size(map_size),
    _size(size),
    _rows((map_size.height / size.height) + ((map_size.height % size.height) ? 1: 0)),
    _columns((map_size.width / size.width) + ((map_size.width % size.width) ? 1 : 0)),
    _count(_rows * _columns)
{
    auto& cache = this->_activated_cache;
    auto callback = [&cache] (fb::game::sector& x) 
    {
        if(x.activated())
            cache[x.id()] = &x;
        else
            cache.erase(x.id());
    };
    for(auto i = 0; i < this->_count; i++)
        this->_pool[i] = new sector(i, callback);
}

fb::game::sectors::~sectors()
{
    for(auto x : this->_pool)
        delete x.second;
}

uint32_t fb::game::sectors::index(const point16_t& position) const
{
    return (position.y / this->_size.height) * this->_columns + 
        (position.x / this->_size.width);
}

std::set<fb::game::sector*> fb::game::sectors::activated_sectors() const
{
    auto sectors = std::set<fb::game::sector*>();
    for(auto x : this->_activated_cache)
    {
        auto nears = this->nears(x.first);
        sectors.insert(nears.begin(), nears.end());
    }

    return sectors;
}


fb::game::sector* fb::game::sectors::at(const point16_t& position) const
{
    auto index = this->index(position);
    return this->at(index);
}

fb::game::sector* fb::game::sectors::at(uint32_t index) const
{
    if(index > this->_pool.size() - 1)
        return nullptr;

    return this->_pool.at(index);
}

uint32_t fb::game::sectors::push(fb::game::object& object)
{
    auto index = this->index(object.position());
    auto sector = this->at(index);
    object.sector(sector);

    return index;
}

std::vector<fb::game::sector*> fb::game::sectors::nears(uint32_t index) const
{
    auto sectors = std::vector<fb::game::sector*>();
    sectors.push_back(this->at(index));

    auto is_left = index % this->_columns == 0;
    if(!is_left)
        sectors.push_back(this->at(index - 1));

    auto is_right = index % this->_columns == this->_columns - 1;
    if(!is_right)
        sectors.push_back(this->at(index + 1));

    auto is_top = index < this->_columns;
    if(!is_top)
        sectors.push_back(this->at(index - this->_columns));

    auto is_bottom = index > this->_columns * (this->_rows-1) - 1;
    if(!is_bottom)
        sectors.push_back(this->at(index + this->_columns));

    if(!is_left && !is_top)
        sectors.push_back(this->at(index - this->_columns - 1));

    if(!is_right && !is_top)
        sectors.push_back(this->at(index - this->_columns + 1));

    if(!is_left && !is_bottom)
        sectors.push_back(this->at(index + this->_columns - 1));

    if(!is_right && !is_bottom)
        sectors.push_back(this->at(index + this->_columns + 1));

    return sectors;
}

std::vector<fb::game::sector*> fb::game::sectors::nears(const point16_t& pivot) const
{
    return this->nears(this->index(pivot));
}

std::vector<fb::game::object*> fb::game::sectors::objects(const point16_t& pivot, fb::game::object::types type) const
{
    auto sectors = this->nears(pivot);
    auto objects = std::vector<fb::game::object*>();
    for(auto sector : sectors)
        objects.insert(objects.end(), sector->begin(), sector->end());

    if(type == fb::game::object::types::UNKNOWN)
    {
        return objects;
    }
    else
    {
        auto filtered = std::vector<fb::game::object*>();
        std::copy_if
        (
            objects.begin(), objects.end(), std::back_inserter(filtered), 
            [type](fb::game::object* x)
            {
                return x->is(type);
            }
        );
        return filtered;
    }
}

std::vector<fb::game::object*> fb::game::sectors::activated_objects(fb::game::object::types type) const
{
    auto result = std::vector<fb::game::object*>();
    for(auto sector : this->activated_sectors())
    {
        std::copy_if
        (
            sector->begin(), sector->end(), std::back_inserter(result),
            [type] (fb::game::object* x)
            {
                return type == fb::game::object::types::UNKNOWN ||  x->is(type);
            }
        );
    }

    return result;
}
