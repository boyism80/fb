#ifndef __PROTOCOL_RESPONSE_GAME_MAP_H__
#define __PROTOCOL_RESPONSE_GAME_MAP_H__

#include <protocol/protocol.h>
#include <model/map/map.h>

namespace fb { namespace protocol { namespace response { namespace game { namespace map {

class update : public fb::protocol::base::response
{
public:
    const point16_t         position;
    const size8_t           size;

public:
    update(const point16_t& position, const size8_t& size) : 
        position(position), size(size)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u16(this->position.x)
                  .write_u16(this->position.y)
                  .write_u8(this->size.width)
                  .write_u8(this->size.height);
    }
};

class bgm : public fb::protocol::base::response
{
public:
    const fb::game::map&            map;
    const uint16_t                  volume;

public:
    bgm(const fb::game::map& map, uint16_t volume = 100) : 
        map(map), volume(volume)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x19)
                  .write_u8(0x01)
                  .write_u8(0x05)
                  .write_u16(this->map.id())
                  .write_u16(this->map.id())
                  .write_u8(volume) // volume
                  .write_u16(512)
                  .write_u16(512)
                  .write_u8(0x00)
                  .write_u8(0x00)
                  .write_u8(0x00);
    }
};

class config : public fb::protocol::base::response
{
public:
    const fb::game::map&            map;

public:
    config(const fb::game::map& map) : 
        map(map)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x15)
                  .write_u16(this->map.id()) // id
                  .write_u16(this->map.width()) // width
                  .write_u16(this->map.height()) // height
                  .write_u8(this->map.option() & fb::game::map::options::BUILD_IN ? 0x04 : 0x05) // this.building ? 0x04 : 0x05
                  .write(this->map.name(), true);
    }
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_GAME_MAP_H__