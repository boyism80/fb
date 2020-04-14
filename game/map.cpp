#include "map.h"
#include "session.h"
#include "fb_game.h"

IMPLEMENT_LUA_EXTENSION(fb::game::map, "fb.game.map")
{"name",        fb::game::map::builtin_name},
{"objects",     fb::game::map::builtin_objects},
{"width",       fb::game::map::builtin_width},
{"height",      fb::game::map::builtin_height},
{"area",        fb::game::map::builtin_area},
{"movable",     fb::game::map::builtin_movable},
END_LUA_EXTENSION

static const uint16_t crc16tab[256] = 
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};



fb::game::objects::objects() : _sequence(1)
{
}

fb::game::objects::~objects()
{
}

uint16_t fb::game::objects::empty_seq()
{
    for(int i = this->_sequence; i < 0xFFFF; i++)
    {
        if(this->at(i) != NULL)
            continue;

        this->_sequence = i + 1;
        return i;
    }

    for(int i = 1; i < this->_sequence; i++)
    {
        if(this->at(i) != NULL)
            continue;

        this->_sequence = i + 1;
        return i;
    }

    return 0xFFFF;
}

std::vector<fb::game::session*> fb::game::objects::sessions()
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

fb::game::object* fb::game::objects::at(uint16_t id)
{
    for(const auto object : *this)
    {
        if(object->id() != id)
            continue;

        return object;
    }

    return NULL;
}

uint16_t fb::game::objects::add(fb::game::object& object)
{
    auto                    seq = this->empty_seq();
    auto                    found = this->at(seq);
    if(found != NULL)
    {
        this->erase(std::find(this->begin(), this->end(), found));
        delete found;
    }

    this->push_back(&object);
    object.id(seq);
    return seq;
}

uint16_t fb::game::objects::add(fb::game::object& object, const point16_t position)
{
    auto seq = this->add(object);
    object.position(position);

    return seq;
}

bool fb::game::objects::remove(fb::game::object& object)
{
    auto i = std::find(this->begin(), this->end(), &object);
    if(i == this->end())
        return false;

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

    return NULL;
}



fb::game::map::map(uint16_t id, uint16_t parent, uint8_t bgm, const std::string& name, fb::game::map::options option, fb::game::map::effects effect, const void* data, uint32_t size) :
    _id(id),
    _parent(parent),
    _bgm(bgm),
    _name(name),
    _option(option),
    _effect(effect),
    _tiles(NULL)
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
    if(this->_tiles == NULL)
        throw std::runtime_error(std::string("cannot allocate map memory : ") + name);

    memset(this->_tiles, 0, sizeof(tile) * map_size);
    for(uint32_t i = 0; i < map_size; i++)
    {
        this->_tiles[i].id = istream.read_u16();
        this->_tiles[i].object = istream.read_u16();
    }


    // compare linear doors
    point16_t position;
    for(const auto door : db::doors())
    {
        position.x = position.y = 0;
        while(this->find_door(*door, position, true))
        {
            this->doors.add(this, *door, position, true);
            position.x += door->size();
        }

        position.x = position.y = 0;
        while(this->find_door(*door, position, false))
        {
            this->doors.add(this, *door, position, false);
            position.x += door->size();
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

bool fb::game::map::matched_door(const fb::game::door& door, const point16_t& position, bool open) const
{
    auto door_size = door.size();
    for(int i = 0; i < door_size; i++)
    {
        auto now = (*this)(position.x + i, position.y)->object;
        auto dst = open ? door[i].open : door[i].close;

        if(now != dst)
            return false;
    }

    return true;
}

bool fb::game::map::find_door(const fb::game::door& door, point16_t& position, bool open) const
{
    bool init = true;
    auto door_size = door.size();
    for(int offset = position.y * this->_size.width + position.x; offset < this->_size.width * this->_size.height; offset++)
    {
        auto row = offset / this->_size.width;
        auto col = offset % this->_size.width;

        auto pos = point16_t(col, row);
        if(this->matched_door(door, pos, open) == false)
            continue;

        position = pos;
        return true;
    }

    return false;
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
        if(object->alive() == false)
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

    return NULL;
}

void fb::game::map::handle_timer(uint64_t elapsed_milliseconds)
{
    
}

fb::game::map::tile* fb::game::map::operator()(uint16_t x, uint16_t y) const
{
    if(x > this->_size.width)
        return NULL;

    if(y > this->_size.height)
        return NULL;

    return this->_tiles + (y * this->_size.width + x);
}

fb::ostream fb::game::map::make_config_stream() const
{
    fb::ostream             ostream;
    ostream.write_u8(0x15)
        .write_u16(this->_id) // this id
        .write_u16(this->_size.width) // width
        .write_u16(this->_size.height) // height
        .write_u8(this->_option & map::options::BUILD_IN ? 0x04 : 0x05) // this.building ? 0x04 : 0x05
        .write(this->_name, true);

    return ostream;
}

fb::ostream fb::game::map::make_bgm_stream() const
{
    fb::ostream             ostream;
    ostream.write_u8(0x19)
        .write_u8(0x01)
        .write_u8(0x05)
        .write_u16(this->_bgm)
        .write_u16(this->_bgm)
        .write_u8(100) // volume
        .write_u16(512)
        .write_u16(512)
        .write_u8(0x00)
        .write_u8(0x00)
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::map::make_update_stream(uint16_t begin_x, uint16_t begin_y, uint8_t width, uint8_t height, uint16_t crc) const
{
    fb::ostream             ostream;

    ostream.write_u8(0x06);

    if(this->_effect == map::effects::NO_EFFECT)
        ostream.write_u8(0x00);
    else
        ostream.write_u8(0x04).write_u8(this->_effect);

    ostream.write_u16(begin_x)
        .write_u16(begin_y)
        .write_u8(width)
        .write_u8(height);

    uint16_t now_crc = 0;
    uint32_t map_size = width * height * sizeof(uint16_t)*3; // tile id, block, object
    for(int row = begin_y; row < begin_y + height; row++)
    {
        for(int col = begin_x; col < begin_x + width; col++)
        {
            map::tile* tile = (*this)(col, row);
            if(tile == NULL)
                continue;

            ostream.write_u16(tile->id)
                .write_u16(tile->blocked)
                .write_u16(tile->object);

            now_crc = (now_crc << 8) ^ crc16tab[now_crc >> 8] ^ tile->id;
            now_crc = (now_crc << 8) ^ crc16tab[now_crc >> 8] ^ uint16_t(tile->blocked);
            now_crc = (now_crc << 8) ^ crc16tab[now_crc >> 8] ^ tile->object;
        }
    }

    if(crc == now_crc)
        return fb::ostream();

    return ostream;
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
        position.x = lua_tointeger(lua, -1);
        lua_remove(lua, -1);

        lua_rawgeti(lua, 2, 2);
        position.y = lua_tointeger(lua, -1);
        lua_remove(lua, -1);
    }
    else if(lua_isnumber(lua, 2) && lua_isnumber(lua, 3))
    {
        position.x = lua_tointeger(lua, 2);
        position.y = lua_tointeger(lua, 3);
    }
    else
    {
        lua_pushboolean(lua, false);
        return 1;
    }

    lua_pushboolean(lua, map->movable(position));
    return 1;
}

fb::game::map::door::door(fb::game::map* owner, fb::game::door& core, const point16_t position, bool opened) : 
    _owner(owner),
    _core(core), 
    position(position), 
    _opened(opened)
{
}

fb::game::map::door::~door()
{
}

const fb::game::door& fb::game::map::door::core() const
{
    return this->_core;
}

bool fb::game::map::door::toggle()
{
    this->_opened = !this->_opened;

    for(int i = 0; i < this->_core.size(); i++)
    {
        auto& object = (*this->_owner)(this->position.x + i, this->position.y)->object;
        auto  door_e = this->_opened ? this->_core[i].open : this->_core[i].close;
        
        object = door_e;
    }

    return true;
}

bool fb::game::map::door::opened() const
{
    return this->_opened;
}

fb::game::map::doors::doors()
{
}

fb::game::map::doors::~doors()
{
    for(auto door : *this)
        delete door;
}

void fb::game::map::doors::add(map* map, fb::game::door& core, const point16_t position, bool opened)
{
    this->push_back(new map::door(map, core, position, opened));
}

fb::game::map::door* fb::game::map::doors::find(const point16_t position)
{
    for(auto door : *this)
    {
        const auto& core = door->core();
        for(int i = 0; i < core.size(); i++)
        {
            if(position == point16_t(door->position.x + i, door->position.y))
                return door;
        }
    }
}
