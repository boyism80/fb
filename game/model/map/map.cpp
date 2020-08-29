#include <model/map/map.h>
#include <model/session/session.h>
#include <model/mob/mob.h>
#include <model/acceptor/acceptor.game.h>

fb::game::objects::objects(fb::game::map* owner) : 
    _sequence(1),
    _owner(owner)
{
}

fb::game::objects::~objects()
{
}

uint16_t fb::game::objects::empty_seq()
{
    for(int i = this->_sequence; i < 0xFFFF; i++)
    {
        if(this->find(i) != nullptr)
            continue;

        this->_sequence = i + 1;
        return i;
    }

    for(int i = 1; i < this->_sequence; i++)
    {
        if(this->find(i) != nullptr)
            continue;

        this->_sequence = i + 1;
        return i;
    }

    return 0xFFFF;
}

std::vector<fb::game::session*> fb::game::objects::sessions() const
{
    std::vector<session*> sessions;

    for(auto e : *this)
    {
        if(e->type() != object::types::SESSION)
            continue;

        sessions.push_back(static_cast<session*>(e));
    }

    return sessions;
}

std::vector<fb::game::mob*> fb::game::objects::active_mobs() const
{
    std::map<uint32_t, fb::game::mob*> active_table;
    auto sessions = this->sessions();

    for(auto x : *this)
    {
        if(x->is(object::types::MOB) == false)
            continue;

        auto mob = static_cast<game::mob*>(x);

        if(mob->alive() == false)
            continue;

        if(std::any_of(sessions.begin(), sessions.end(), [&mob] (fb::game::session* session) { return session->sight(*mob); }) == false)
            continue;

        active_table.insert(std::pair<uint32_t, fb::game::mob*>(mob->id(), mob));
    }

    std::vector<fb::game::mob*> actives;
    std::transform(active_table.begin(), active_table.end(), std::back_inserter(actives), [] (auto& pair) { return pair.second; });
    return actives;
}

fb::game::object* fb::game::objects::find(uint16_t id)
{
    auto i = std::find_if(this->begin(), this->end(), [id] (object* x) { return x->id() == id; });
    return i != this->end() ? *i : nullptr;
}

uint16_t fb::game::objects::add(fb::game::object& object)
{
    return add(object, point16_t());
}

uint16_t fb::game::objects::add(fb::game::object& object, const point16_t& position)
{
    auto                    map = object.map();
    if(map == this->_owner)
    {
        object.position(position);
        return object.id();
    }
    else
    {
        if(map != nullptr)
            map->objects.remove(object);

        auto                    seq = this->empty_seq();
        auto                    found = this->find(seq);
        if(found != nullptr)
        {
            this->erase(std::find(this->begin(), this->end(), found));
            delete found;
        }

        this->push_back(&object);
        object.id(seq);
        object.map(this->_owner, position);
        return seq;
    }
}

bool fb::game::objects::remove(fb::game::object& object)
{
    auto i = std::find(this->begin(), this->end(), &object);
    if(i == this->end())
        return false;

    object.map(nullptr);
    this->erase(i);
    return true;
}

fb::game::object* fb::game::objects::exists(point16_t position) const
{
    for(auto object : *this)
    {
        if(object->position() == position)
            return object;
    }

    return nullptr;
}



fb::game::map::map(uint16_t id, uint16_t parent, const std::string& host_id, uint8_t bgm, const std::string& name, fb::game::map::options option, fb::game::map::effects effect, const void* data, size_t size) :
    _id(id),
    _parent(parent),
    _host_id(host_id),
    _bgm(bgm),
    _name(name),
    _option(option),
    _effect(effect),
    _tiles(nullptr),
    objects(this)
{

    std::string what;

    fb::istream istream((uint8_t*)data, size);
    this->_size.width = istream.read_u16();
    if(this->_size.width == 0)
        throw std::runtime_error(std::string("cannot read map size : ") + name);

    this->_size.height = istream.read_u16();
    if(this->_size.height == 0)
        throw std::runtime_error(std::string("cannot read map size : ") + name);

    uint32_t map_size = this->_size.width * this->_size.height;
    this->_tiles = new tile[map_size];
    if(this->_tiles == nullptr)
        throw std::runtime_error(std::string("cannot allocate map memory : ") + name);

    memset(this->_tiles, 0, sizeof(tile) * map_size);
    for(uint32_t i = 0; i < map_size; i++)
    {
        this->_tiles[i].id = istream.read_u16();
        this->_tiles[i].object = istream.read_u16();
    }


    // compare linear doors
    point16_t position;
    for(const auto door : game::master::get().doors)
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
}

fb::game::map::~map()
{
    delete[] this->_tiles;

    for(auto object : this->objects)
        delete object;

    for(auto warp : this->_warps)
        delete warp;
}

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
    if(x > this->_size.width)
        return true;

    if(y > this->_size.height)
        return true;

    return this->_tiles[y * this->_size.width + x].blocked;
}

bool fb::game::map::block(uint16_t x, uint16_t y, bool option)
{
    if(x > this->_size.width)
        return false;

    if(y > this->_size.height)
        return false;

    this->_tiles[y * this->_size.width + x].blocked = option;
    return true;
}

fb::game::map::effects fb::game::map::effect() const
{
    return this->_effect;
}

fb::game::map::options fb::game::map::option() const
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

    for(const auto object : this->objects)
    {
        if(object->visible() == false)
            continue;

        if(object->type() == fb::game::object::types::ITEM)
            continue;

        if(object->position() == position)
            return false;
    }

    return true;
}

bool fb::game::map::movable(const fb::game::object& object, fb::game::direction direction) const
{
    point16_t               position = object.position();

    switch(direction)
    {
    case direction::BOTTOM:
        position.y++;
        break;

    case direction::TOP:
        position.y--;
        break;

    case direction::LEFT:
        position.x--;
        break;

    case direction::RIGHT:
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

void fb::game::map::warp_add(map* map, const point16_t& before, const point16_t& after, const range8_t& limit)
{
    this->_warps.push_back(new warp(map, before, after, limit));
}

const fb::game::map::warp* fb::game::map::warpable(const point16_t& position) const
{
    for(const auto warp : this->_warps)
    {
        if(warp->before != position)
            continue;

        return warp;
    }

    return nullptr;
}

const std::string& fb::game::map::host_id() const
{
    return this->_host_id;
}

void fb::game::map::handle_timer(uint64_t elapsed_milliseconds)
{
    
}

fb::game::map::tile* fb::game::map::operator()(uint16_t x, uint16_t y) const
{
    if(x > this->_size.width)
        return nullptr;

    if(y > this->_size.height)
        return nullptr;

    return this->_tiles + (y * this->_size.width + x);
}

int fb::game::map::builtin_name(lua_State* lua)
{
    auto map = *(fb::game::map**)lua_touserdata(lua, 1);

    lua_pushstring(lua, map->name().c_str());
    return 1;
}

int fb::game::map::builtin_objects(lua_State* lua)
{
    auto map = *(fb::game::map**)lua_touserdata(lua, 1);

    lua_newtable(lua);
    const auto& objects = map->objects;

    for(int i = 0; i < map->objects.size(); i++)
    {
        map->objects[i]->to_lua(lua);
        lua_rawseti(lua, -2, i+1);
    }

    return 1;
}

int fb::game::map::builtin_width(lua_State* lua)
{
    auto map = *(fb::game::map**)lua_touserdata(lua, 1);

    lua_pushinteger(lua, map->width());
    return 1;
}

int fb::game::map::builtin_height(lua_State* lua)
{
    auto map = *(fb::game::map**)lua_touserdata(lua, 1);

    lua_pushinteger(lua, map->height());
    return 1;
}

int fb::game::map::builtin_area(lua_State* lua)
{
    auto map = *(fb::game::map**)lua_touserdata(lua, 1);

    lua_pushinteger(lua, map->width());
    lua_pushinteger(lua, map->height());
    return 2;
}

int fb::game::map::builtin_movable(lua_State* lua)
{
    auto map = *(fb::game::map**)lua_touserdata(lua, 1);
    auto position = point16_t();

    if(lua_istable(lua, 2))
    {
        lua_rawgeti(lua, 2, 1);
        position.x = (uint16_t)lua_tointeger(lua, -1);
        lua_remove(lua, -1);

        lua_rawgeti(lua, 2, 2);
        position.y = (uint16_t)lua_tointeger(lua, -1);
        lua_remove(lua, -1);
    }
    else if(lua_isnumber(lua, 2) && lua_isnumber(lua, 3))
    {
        position.x = (uint16_t)lua_tointeger(lua, 2);
        position.y = (uint16_t)lua_tointeger(lua, 3);
    }
    else
    {
        lua_pushboolean(lua, false);
        return 1;
    }

    lua_pushboolean(lua, map->movable(position));
    return 1;
}

int fb::game::map::builtin_door(lua_State* lua)
{
    auto map = *(fb::game::map**)lua_touserdata(lua, 1);
    auto session = *(fb::game::session**)lua_touserdata(lua, 2);

    auto door = map->doors.find(*session);
    if(door == nullptr)
        lua_pushnil(lua);
    else
        door->to_lua(lua);

    return 1;
}

int fb::game::map::builtin_doors(lua_State* lua)
{
    auto map = *(fb::game::map**)lua_touserdata(lua, 1);

    lua_newtable(lua);

    auto i = 0;
    for(const auto& door : map->doors)
    {
        door->to_lua(lua);
        lua_rawseti(lua, -2, i+1);
    }
    
    return 1;
}
