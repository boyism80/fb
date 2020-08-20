#ifndef __PROTOCOL_RESPONSE_GAME_MAP_H__
#define __PROTOCOL_RESPONSE_GAME_MAP_H__

#include <protocol/protocol.h>

namespace fb { namespace protocol { namespace response { namespace game { namespace map {

class update : public fb::protocol::base::response
{
public:
    const point16_t         position;
    const size16_t          size;

public:
    update(const point16_t& position, const size16_t& size) : 
        position(position), size(size)
    {}

public:
    void serialize(fb::ostream& out_stream)
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
    const uint16_t id;
    const uint8_t volume;

public:
    bgm(uint16_t id, uint8_t volume = 100) : 
        id(id), volume(volume)
    {}

public:
    void serialize(fb::ostream& out_stream)
    {
        out_stream.write_u8(0x19)
                  .write_u8(0x01)
                  .write_u8(0x05)
                  .write_u16(this->id)
                  .write_u16(this->id)
                  .write_u8(this->volume) // volume
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
    const uint8_t           id;
    const size16_t          size;
    const uint8_t           building;
    const std::string       name;

public:
    config(uint8_t id, const size16_t& size, uint8_t building, const std::string& name) : 
        id(id), size(size), building(building), name(name)
    {}

public:
    void serialize(fb::ostream& out_stream)
    {
        out_stream.write_u8(0x15)
                  .write_u16(this->id) // this id
                  .write_u16(this->size.width) // width
                  .write_u16(this->size.height) // height
                  .write_u8(this->building) // this.building ? 0x04 : 0x05
                  .write(this->name, true);
    }
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_GAME_MAP_H__