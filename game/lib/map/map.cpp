#include <fb/game/map.h>
#include <fb/game/context.h>

fb::game::objects::objects(fb::game::map* owner) : 
    _owner(owner)
{ }

fb::game::objects::~objects()
{ }

uint16_t fb::game::objects::empty_seq()
{
    for(int i = this->_sequence; i < 0xFFFF; i++)
    {
        if(this->contains(i))
            continue;

        this->_sequence = i + 1;
        return i;
    }

    for(int i = 1; i < this->_sequence; i++)
    {
        if(this->contains(i))
            continue;

        this->_sequence = i + 1;
        return i;
    }

    return 0xFFFF;
}

std::vector<fb::game::object*> fb::game::objects::filter(fb::game::object::types type) const
{
    auto result = std::vector<fb::game::object*>();
    for(auto& [key, value] : *this)
    {
        if(value->is(object::types::SESSION))
            result.push_back(value);
    }

    return std::move(result);
}

uint16_t fb::game::objects::add(fb::game::object& object)
{
    return add(object, point16_t());
}

uint16_t fb::game::objects::add(fb::game::object& object, const point16_t& position)
{
    auto                    seq = this->empty_seq();
    if(this->contains(seq))
        this->erase(seq);

    object.sequence(seq);
    this->insert(std::make_pair(seq, &object));
    return object.sequence();
}

bool fb::game::objects::remove(fb::game::object& object)
{
    if(this->contains(object.sequence()) == false)
        return false;

    this->erase(object.sequence());
    return true;
}

fb::game::object* fb::game::objects::exists(point16_t position) const
{
    auto found = std::find_if
    (
        this->cbegin(), this->cend(),
        [&position] (const auto& pair)
        {
            return pair.second->position() == position;
        }
    );

    if(found == this->cend())
        return nullptr;
    else
        return found->second;
}

fb::game::object* fb::game::objects::operator[](uint32_t seq) const
{
    const auto& found = this->find(seq);
    if(found != this->cend())
        return found->second;

    return nullptr;
}



fb::game::map::map(uint16_t id, uint16_t parent, uint8_t bgm, const std::string& name, fb::game::map::OPTION option, fb::game::map::EFFECT effect, uint32_t group, const void* data, size_t size) :
    _id(id),
    _parent(parent),
    _bgm(bgm),
    _name(name),
    _option(option),
    _effect(effect),
    group(group),
    active(fb::config::get()["group"].asUInt() == group)
{
    if (this->active == false)
        return;

    std::string what;

    fb::istream istream((uint8_t*)data, size);
    this->_size.width = istream.read_u16();
    if(this->_size.width == 0)
        throw std::runtime_error("맵 데이터가 올바르지 않습니다.");

    this->_size.height = istream.read_u16();
    if(this->_size.height == 0)
        throw std::runtime_error("맵 데이터가 올바르지 않습니다.");

    uint32_t map_size = this->_size.width * this->_size.height;
    this->_tiles = std::make_unique<tile[]>(map_size);
    if(this->_tiles == nullptr)
        throw std::runtime_error("맵 타일 메모리를 할당할 수 없습니다.");

    for(uint32_t i = 0; i < map_size; i++)
    {
        this->_tiles[i].id = istream.read_u16();
        this->_tiles[i].object = istream.read_u16();
    }


    // compare linear doors
    point16_t position;
    for(const auto& door : fb::game::old_model::doors)
    {
        position.x = position.y = 0;
        while(door->find(*this, position, true))
        {
            this->doors.add(this, *door, position, true);
            position.x += (uint16_t)door->size();
        }

        position.x = position.y = 0;
        while(door->find(*this, position, false))
        {
            this->doors.add(this, *door, position, false);
            position.x += (uint16_t)door->size();
        }
    }

    // sectors
    this->_sectors = std::make_unique<fb::game::sectors>(this->_size, size16_t(MAX_SCREEN_WIDTH, MAX_SCREEN_HEIGHT));
}

fb::game::map::~map()
{ }

uint16_t fb::game::map::id() const
{
    return this->_id;
}

uint16_t fb::game::map::parent() const
{
    return this->_parent;
}

const std::string& fb::game::map::name() const
{
    return this->_name;
}

bool fb::game::map::blocked(uint16_t x, uint16_t y) const
{
    if(x >= this->_size.width)
        return true;

    if(y >= this->_size.height)
        return true;

    return this->_tiles[y * this->_size.width + x].blocked;
}

bool fb::game::map::block(uint16_t x, uint16_t y, bool option)
{
    if(this->_tiles == nullptr)
        return false;

    if(x >= this->_size.width)
        return false;

    if(y >= this->_size.height)
        return false;

    this->_tiles[y * this->_size.width + x].blocked = option;
    return true;
}

fb::game::map::EFFECT fb::game::map::effect() const
{
    return this->_effect;
}

fb::game::map::OPTION fb::game::map::option() const
{
    return this->_option;
}

uint16_t fb::game::map::width() const
{
    return this->_size.width;
}

uint16_t fb::game::map::height() const
{
    return this->_size.height;
}

fb::game::size16_t fb::game::map::size() const
{
    return this->_size;
}

uint8_t fb::game::map::bgm() const
{
    return this->_bgm;
}

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
    if(this->existable(position) == false)
        return false;

    if((*this)(position.x, position.y)->blocked)
        return false;

    for(const auto& [key, value] : this->objects)
    {
        if(value->visible() == false)
            continue;

        if(value->type() == fb::game::object::types::ITEM)
            continue;

        if(value->position() == position)
            return false;
    }

    return true;
}

bool fb::game::map::movable(const fb::game::object& object, fb::game::DIRECTION_TYPE direction) const
{
    point16_t               position = object.position();

    switch(direction)
    {
    case fb::game::DIRECTION_TYPE::BOTTOM:
        position.y++;
        break;

    case fb::game::DIRECTION_TYPE::TOP:
        position.y--;
        break;

    case fb::game::DIRECTION_TYPE::LEFT:
        position.x--;
        break;

    case fb::game::DIRECTION_TYPE::RIGHT:
        position.x++;
        break;
    }

    if(this->movable(position) == false)
        return false;

    return true;
}

bool fb::game::map::movable_forward(const fb::game::object& object, uint16_t step) const
{
    return this->movable(object, object.direction());
}

void fb::game::map::push_warp(fb::game::map* map, const point16_t& before, const point16_t& after, const range8_t& condition)
{
    this->_warps.push_back(std::make_unique<warp>(map, before, after, condition));
}

void fb::game::map::push_warp(fb::game::wm::offset* offset, const point16_t& before)
{
    this->_warps.push_back(std::make_unique<warp>(offset, before));
}

const fb::game::map::warp* fb::game::map::warpable(const point16_t& position) const
{
    for(const auto& warp : this->_warps)
    {
        if(warp->before != position)
            continue;

        return warp.get();
    }

    return nullptr;
}

bool fb::game::map::update(fb::game::object& object)
{
    if(this->_sectors == nullptr)
        return false;

    auto sector = this->_sectors->at(object.position());
    if(sector == nullptr)
        return false;

    return object.sector(sector);
}

bool fb::game::map::activated() const
{
    if(this->_sectors == nullptr)
        return false;

    return this->_sectors->activated();
}

std::vector<fb::game::object*> fb::game::map::nears(const point16_t& pivot, fb::game::object::types type) const
{
    if(this->_sectors == nullptr)
        return std::vector<fb::game::object*> { };
    else
        return this->_sectors->objects(pivot, type);
}

std::vector<fb::game::object*> fb::game::map::belows(const point16_t& pivot, fb::game::object::types type) const
{
    auto objects = std::vector<fb::game::object*>();
    try
    {
        if(this->_sectors == nullptr)
            throw std::exception();

        auto sector = this->_sectors->at(pivot);
        std::copy_if
        (
            sector->begin(), sector->end(), std::back_inserter(objects),
            [type, &pivot] (auto x)
            {
                return 
                    (type == fb::game::object::types::UNKNOWN || x->is(type)) && 
                    x->position() == pivot;
            }
        );
    }
    catch(std::exception&)
    { }

    return std::move(objects);
}

std::vector<fb::game::object*> fb::game::map::activateds(fb::game::object::types type)
{
    if(this->_sectors == nullptr)
        return std::vector<fb::game::object*> { };
    else
        return this->_sectors->activated_objects(type);
}

void fb::game::map::on_timer(uint64_t elapsed_milliseconds)
{
    
}

fb::game::map::tile* fb::game::map::operator()(uint16_t x, uint16_t y) const
{
    if(x > this->_size.width)
        return nullptr;

    if(y > this->_size.height)
        return nullptr;

    return &this->_tiles[y * this->_size.width + x];
}

int fb::game::map::builtin_name(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto map = thread->touserdata<fb::game::map>(1);
    if(map == nullptr)
        return 0;

    thread->pushstring(map->name());
    return 1;
}

int fb::game::map::builtin_objects(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto map = thread->touserdata<fb::game::map>(1);
    if(map == nullptr)
        return 0;
    

    thread->new_table();
    const auto& objects = map->objects;

    for(int i = 0; i < objects.size(); i++)
    {
        thread->pushobject(map->objects[i]);
        lua_rawseti(lua, -2, i+1);
    }

    return 1;
}

int fb::game::map::builtin_width(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto map = thread->touserdata<fb::game::map>(1);
    if(map == nullptr)
        return 0;
    

    thread->pushinteger(map->width());
    return 1;
}

int fb::game::map::builtin_height(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto map = thread->touserdata<fb::game::map>(1);
    if(map == nullptr)
        return 0;
    

    thread->pushinteger(map->height());
    return 1;
}

int fb::game::map::builtin_area(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto map = thread->touserdata<fb::game::map>(1);
    if(map == nullptr)
        return 0;
    

    thread->pushinteger(map->width());
    thread->pushinteger(map->height());
    return 2;
}

int fb::game::map::builtin_movable(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto map = thread->touserdata<fb::game::map>(1);
    if(map == nullptr)
        return 0;
    
    auto position = point16_t();

    if(lua_istable(lua, 2))
    {
        lua_rawgeti(lua, 2, 1);
        position.x = (uint16_t)thread->tointeger(-1);
        lua_remove(lua, -1);

        lua_rawgeti(lua, 2, 2);
        position.y = (uint16_t)thread->tointeger(-1);
        lua_remove(lua, -1);
    }
    else if(lua_isnumber(lua, 2) && lua_isnumber(lua, 3))
    {
        position.x = (uint16_t)thread->tointeger(2);
        position.y = (uint16_t)thread->tointeger(3);
    }
    else
    {
        thread->pushboolean(false);
        return 1;
    }

    thread->pushboolean(map->movable(position));
    return 1;
}

int fb::game::map::builtin_door(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto map = thread->touserdata<fb::game::map>(1);
    if(map == nullptr)
        return 0;
    
    auto session = thread->touserdata<fb::game::session>(2);
    if(session == nullptr || context->exists(*session) == false)
        return 0;


    auto door = map->doors.find(*session);
    if(door == nullptr)
        thread->pushnil();
    else
        thread->pushobject(door);

    return 1;
}

int fb::game::map::builtin_doors(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto map = thread->touserdata<fb::game::map>(1);
    if(map == nullptr)
        return 0;
    

    thread->new_table();

    auto i = 0;
    for(const auto& door : map->doors)
    {
        thread->pushobject(*door);
        lua_rawseti(lua, -2, i+1);
    }
    
    return 1;
}




// world
void fb::game::wm::group::push(offset* offset)
{
    this->push_back(std::unique_ptr<fb::game::wm::offset>(offset));
}

bool fb::game::wm::group::contains(const offset& offset) const
{
    return std::find_if
    (
        this->cbegin(), this->cend(),
        [&] (const auto& x)
        {
            return x.get() == &offset;
        }
    ) != this->cend();
}

fb::game::wm::world::world(const std::string& name) : 
    name(name)
{ }

void fb::game::wm::world::push(group* group)
{
    this->push_back(std::unique_ptr<fb::game::wm::group>(group));
    for(auto& x : *group)
        this->_offsets.push_back(x.get());
}

const std::vector<fb::game::wm::offset*>& fb::game::wm::world::offsets() const
{
    return this->_offsets;
}

const fb::game::wm::group* fb::game::wm::world::find(const std::string& id) const
{
    auto found = std::find_if
    (
        this->cbegin(), this->cend(), 
        [&] (const auto& group)
        {
            auto x = std::find_if
            (
                group->cbegin(), group->cend(),
                [&] (const auto& offset)
                {
                    return offset->id == id;
                }
            );

            return x != group->cend();
        }
    );

    if(found == this->cend())
        return nullptr;

    return (*found).get();
}

const fb::game::wm::group* fb::game::wm::world::find(const fb::game::map& map) const
{
    auto found = std::find_if
    (
        this->cbegin(), this->cend(), 
        [&] (const auto& group)
        {
            auto x = std::find_if
            (
                group->cbegin(), group->cend(),
                [&] (const auto& offset)
                {
                    return offset->dst.map == &map;
                }
            );

            return x != group->cend();
        }
    );

    if(found == this->cend())
        return nullptr;

    return (*found).get();
}
