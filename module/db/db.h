#ifndef __DB_H__
#define __DB_H__

#include "mysql+++/mysql+++.h"
#include "module/config/config.h"
#include <string>

using namespace daotk::mysql;

namespace fb {

class db
{
private:
    static const uint32_t   SIZE = 10;
    static connection**     _connection_pool;
    static uint32_t         _index;

public:
    static connection&      get();
    static void             release();
};

}

#endif // !__DB_H__
