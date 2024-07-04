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

std::vector<fb::game::object*> fb::game::objects::filter(OBJECT_TYPE type) const
{
    auto result = std::vector<fb::game::object*>();
    for(auto& [key, value] : *this)
    {
        if(value->is(OBJECT_TYPE::SESSION))
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



fb::game::map::map(const fb::game::context& context, const fb::model::map& model, bool active, const void* data, size_t size) :
    context(context),
    model(model),
    active(active),
    doors(*this)
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
    for(auto& [id, door] : context.model.door)
    {
        position.x = position.y = 0;
        while (door.find(*this, position, true))
        {
            this->doors.add(position, door, true);
            position.x += (uint16_t)door.width;
        }

        position.x = position.y = 0;
        while (door.find(*this, position, false))
        {
            this->doors.add(position, door, true);
            position.x += (uint16_t)door.width;
        }
    }

    // sectors
    this->_sectors = std::make_unique<fb::game::sectors>(this->_size, size16_t(MAX_SCREEN_WIDTH, MAX_SCREEN_HEIGHT));
}

fb::game::map::~map()
{ }

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
    if(this->existable(position) == false)
        return false;

    if((*this)(position.x, position.y)->blocked)
        return false;

    for(const auto& [key, value] : this->objects)
    {
        if(value->visible() == false)
            continue;

        if(value->is(OBJECT_TYPE::ITEM))
            continue;

        if(value->position() == position)
            return false;
    }

    return true;
}

bool fb::game::map::movable(const fb::game::object& object, DIRECTION direction) const
{
    point16_t               position = object.position();

    switch(direction)
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

    if(this->movable(position) == false)
        return false;

    return true;
}

bool fb::game::map::movable_forward(const fb::game::object& object, uint16_t step) const
{
    return this->movable(object, object.direction());
}

const fb::model::warp* fb::game::map::warpable(const point16_t& position) const
{
    for (auto& warp : this->context.model.warp[this->model.id])
    {
        if (warp.before == position)
            return &warp;
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

std::vector<fb::game::object*> fb::game::map::nears(const point16_t& pivot, OBJECT_TYPE type) const
{
    if(this->_sectors == nullptr)
        return std::vector<fb::game::object*> { };
    else
        return this->_sectors->objects(pivot, type);
}

std::vector<fb::game::object*> fb::game::map::belows(const point16_t& pivot, OBJECT_TYPE type) const
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
                    (type == OBJECT_TYPE::UNKNOWN || x->is(type)) && 
                    x->position() == pivot;
            }
        );
    }
    catch(std::exception&)
    { }

    return std::move(objects);
}

std::vector<fb::game::object*> fb::game::map::activateds(OBJECT_TYPE type)
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

    auto i = this->index(fb::model:: point16_t(x, y));
    return &this->_tiles[i];
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
        thread->pushobject(door.second);
        lua_rawseti(lua, -2, i+1);
    }
    
    return 1;
}

fb::game::maps::maps(const fb::game::context& context, uint32_t host) : context(context), host(host)
{ }

fb::game::maps::~maps()
{ }

bool fb::game::maps::load_data(uint32_t id, std::vector<char>& buffer)
{
    auto                    fname = fb::format("maps/%06d.map", id);
    auto                    file = std::ifstream(fname, std::ios::binary);
    if(file.is_open() == false)
        return false;

    buffer = std::vector<char>(std::istreambuf_iterator<char>(file), { });
    file.close();

    return true;
}

bool fb::game::maps::load_block(uint32_t id, Json::Value& buffer)
{
    auto                    fname = fb::format("maps/%06d.block", id);
    std::ifstream           file(fname);
    if(file.is_open() == false)
        return false;

    Json::Reader reader;
    if(reader.parse(file, buffer) == false)
        return false;

    file.close();
    return true;
}

void fb::game::maps::load(const fb::model::map& model)
{
    auto                active = (model.host == this->host);
    auto                binary = std::vector<char>();
    auto                blocks = Json::Value();
    if(active)
    {
        if(load_data(model.id, binary) == false)
            throw std::runtime_error(const_value::string::MESSAGE_ASSET_CANNOT_LOAD_MAP_DATA);

        if(load_block(model.id, blocks) == false)
            throw std::runtime_error(const_value::string::MESSAGE_ASSET_CANNOT_LOAD_MAP_BLOCK);
    }

    auto map = new fb::game::map(this->context, model, active, binary.data(), binary.size());
    for (const auto& block : blocks)
    {
        map->block(block["x"].asInt(), block["y"].asInt(), true);
    }

    {
        auto _ = std::lock_guard(this->_mutex);
        this->push(model.id, map);
    }
}

fb::game::map* fb::game::maps::name2map(const std::string& name) const
{
    for(const auto& [id, map] : *this)
    {
       if (map.model.name == name)
           return &map;
    }

    return nullptr;
}