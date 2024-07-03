#ifndef __PROTOCOL_RESPONSE_GAME_MAP_H__
#define __PROTOCOL_RESPONSE_GAME_MAP_H__

#include <fb/protocol/protocol.h>
#include <fb/game/map.h>
#include <fb/game/model.h>

static constexpr uint16_t crc16tab[256] = 
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

namespace fb { namespace protocol { namespace game { namespace response { namespace map {

class update : public fb::protocol::base::header
{
public:
    const fb::game::map&            map;
    const point16_t                 position;
    const size8_t                   size;
    const uint16_t                  crc;

public:
    update(const fb::game::map& map, const point16_t& position, const size8_t& size, uint16_t crc = 0) : fb::protocol::base::header(0x06),
        map(map), position(position), size(size), crc(crc)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);

        if(this->map.model.effect == MAP_EFFECT_TYPE::NONE)
            out_stream.write_u8(0x00);
        else
            out_stream.write_u8(0x04).write_u8(this->map.model.effect);

        out_stream.write_u16(this->position.x)
            .write_u16(this->position.y)
            .write_u8(this->size.width)
            .write_u8(this->size.height);

        uint16_t now_crc = 0;
        uint32_t map_size = this->size.width * this->size.height * sizeof(uint16_t)*3; // tile id, block, object
        for(int row = this->position.y; row < this->position.y + this->size.height; row++)
        {
            for(int col = this->position.x; col < this->position.x + this->size.width; col++)
            {
                auto tile = this->map(col, row);
                if(tile == nullptr)
                    continue;

                out_stream.write_u16(tile->id)
                    .write_u16(tile->blocked)
                    .write_u16(tile->object);

                now_crc = (now_crc << 8) ^ crc16tab[now_crc >> 8] ^ tile->id;
                now_crc = (now_crc << 8) ^ crc16tab[now_crc >> 8] ^ uint16_t(tile->blocked);
                now_crc = (now_crc << 8) ^ crc16tab[now_crc >> 8] ^ tile->object;
            }
        }

        if(crc == now_crc)
            return;
    }
};

class bgm : public fb::protocol::base::header
{
public:
    const fb::game::map&            map;
    const uint8_t                   volume;

public:
    bgm(const fb::game::map& map, uint8_t volume = 100) : fb::protocol::base::header(0x19),
        map(map), volume(volume)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);

        out_stream.write_u8(0x01)
                  .write_u8(0x05)
                  .write_u16(this->map.model.id)
                  .write_u16(this->map.model.id)
                  .write_u8(volume) // volume
                  .write_u16(512)
                  .write_u16(512)
                  .write_u8(0x00)
                  .write_u8(0x00)
                  .write_u8(0x00);
    }
};

class config : public fb::protocol::base::header
{
public:
#ifndef BOT
    const fb::game::map&            map;
#else
    uint16_t                        id;
    size16_t                        size;
    bool                            building;
    std::string                     name;
#endif

public:
#ifndef BOT
    config(const fb::game::map& map) : fb::protocol::base::header(0x15),
        map(map)
    { }
#else
    config() : fb::protocol::base::header(0x15)
    { }
#endif

public:
#ifndef BOT
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);

        out_stream.write_u16(this->map.model.id) // id
                  .write_u16(this->map.width()) // width
                  .write_u16(this->map.height()) // height
                  .write_u8(enum_in(this->map.model.option, MAP_OPTION::BUILD_IN) ? 0x04 : 0x05) // this.building ? 0x04 : 0x05
                  .write(this->map.model.name, true);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->id = in_stream.read_u16();
        this->size.width = in_stream.read_u16();
        this->size.height = in_stream.read_u16();
        this->building = in_stream.read_u8();
        this->name = in_stream.readstr_u16();
    }
#endif
};


class worlds : public fb::protocol::base::header
{
public:
    const fb::game::wm::offset*     offset;

public:
    worlds(const fb::game::wm::offset& offset) : fb::protocol::base::header(0x2E),
        offset(&offset)
    { }
    worlds(const std::string& id) : fb::protocol::base::header(0x2E)
    {
        // TODO: 코드 분석 후 다시 구현
        try
        {
            //auto  windex = fb::game::old_model::worlds.find(id);
            //if(windex == -1)
            //    throw nullptr;

            //auto  world = fb::game::old_model::worlds[windex];
            //auto& offsets = world->offsets();
            //auto  found = std::find_if
            //(
            //    offsets.cbegin(), offsets.cend(),
            //    [&] (auto offset)
            //    {
            //        return offset->id == id;
            //    }
            //);

            //if(found == offsets.cend())
            //    throw nullptr;

            //this->offset = *found;
        }
        catch(...)
        {
            //this->offset = nullptr;
        }
    }

public:
    void serialize(fb::ostream& out_stream) const
    {
        // TODO: 코드 분석 후 다시 구현
        base::header::serialize(out_stream);

        //if(this->offset == nullptr)
        //    return;

        //auto windex = fb::game::old_model::worlds.find(this->offset->id);
        //if(windex == -1)
        //    return;

        //auto world = fb::game::old_model::worlds[windex];
        //if(world == nullptr)
        //    return;

        //auto& offsets = world->offsets();
        //auto current = -1;
        //for(int i = 0; i < offsets.size(); i++)
        //{
        //    if(this->offset == offsets[i])
        //    {
        //        current = i;
        //        break;
        //    }
        //}
        //if(current == -1)
        //    return;

        //out_stream.writestr_u8(world->name)
        //          .write_u8(offsets.size())
        //          .write_u8(current);

        //auto offset_id = 0;
        //for(int gropu_id = 0; gropu_id < world->size(); gropu_id++)
        //{
        //    auto& group = (*world)[gropu_id];
        //    for(auto& offset : *group)
        //    {
        //        out_stream.write_u16(offset->position.x)
        //                  .write_u16(offset->position.y)
        //                  .writestr_u8(offset->name)
        //                  .write_u16(0x0000)
        //                  .write_u16(windex)
        //                  .write_u16(current)
        //                  .write_u16(offset_id++)
        //                  .write_u16(group->size());

        //        for(int i = 0; i < offsets.size(); i++)
        //        {
        //            if(group->contains(*offsets[i]) == false)
        //                continue;

        //            out_stream.write_u16(i);
        //        }
        //    }
        //}
    }
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_GAME_MAP_H__