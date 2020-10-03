#ifndef __DB_H__
#define __DB_H__

#include "mysql+++/mysql+++.h"
#include <string>
using namespace daotk::mysql;

namespace fb {

class db : public connection
{};

}

#endif // !__DB_H__
