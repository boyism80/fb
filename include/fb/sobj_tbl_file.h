#ifndef __FB_SOBJ_TBL_FILE_H__
#define __FB_SOBJ_TBL_FILE_H__

#include <cstdint>
#include <string_view>
#include <vector>

namespace fb {

class sobj_tbl_file
{
public:
    static constexpr uint8_t SOUTH = 0x01;
    static constexpr uint8_t NORTH = 0x02;
    static constexpr uint8_t WEST  = 0x04;
    static constexpr uint8_t EAST  = 0x08;
    static constexpr uint8_t ALL   = 0x0F;

private:
    std::vector<uint8_t> _collision;

public:
    void    load(std::string_view path);
    uint8_t collision(uint16_t object_id) const;
    bool    fully_blocked(uint16_t object_id) const;
};

} // namespace fb

#endif
