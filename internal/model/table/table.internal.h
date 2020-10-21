#ifndef __FB_INTERNAL_TABLE_H__
#define __FB_INTERNAL_TABLE_H__

#include <map>
#include "module/table/table.h"

namespace fb { namespace internal {

class table;

namespace container {

class host : private std::map<uint32_t, std::string>
{
private:
    friend class fb::internal::table;

public:
    using std::map<uint32_t, std::string>::begin;
    using std::map<uint32_t, std::string>::end;
    using std::map<uint32_t, std::string>::cbegin;
    using std::map<uint32_t, std::string>::cend;

public:
    std::string*            operator [] (uint32_t id);

public:
    bool                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);
};

}

class table
{
public:
    static fb::internal::container::host hosts;

private:
    table();
    ~table();
};

} }
#endif // !__FB_INTERNAL_TABLE_H__
