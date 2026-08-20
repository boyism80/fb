#ifndef __FB_META_DAT_FILE_H__
#define __FB_META_DAT_FILE_H__

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace fb {

struct meta_dat_entry
{
    std::string name;
    uint32_t    crc = 0;
    std::string compressed;
};

struct meta_dat_collection_item
{
    std::string name;
    uint8_t     group_id = 0;
    uint8_t     slot     = 0;
    uint32_t    look     = 0;
    std::string description;
};

struct meta_dat_collection_group
{
    uint8_t                               id = 0;
    std::string                           name;
    std::vector<meta_dat_collection_item> items;
};

class meta_dat_file
{
private:
    std::vector<meta_dat_entry>            _entries;
    std::vector<meta_dat_collection_group> _groups;

public:
    void load(std::string_view path, bool parse_collections);

    const std::vector<meta_dat_entry>&            entries() const;
    const meta_dat_entry*                         find(std::string_view name) const;
    std::vector<std::pair<std::string, uint32_t>> crc_list() const;
    const std::vector<meta_dat_collection_group>& groups() const;
    const meta_dat_collection_group*              group(uint8_t id) const;
    const meta_dat_collection_item*               find_item(std::string_view name) const;
    const meta_dat_collection_item*               item(uint8_t group_id, uint8_t slot) const;
};

} // namespace fb

#endif
