#ifndef __SQL_SERVICE_H__
#define __SQL_SERVICE_H__

#include "module/service/service.h"
#include "module/config/config.h"
#include <ctime>

namespace fb { namespace login { 

class session;
class item;
    
namespace service { namespace sql {

namespace auth {

std::string                 insert(const std::string& name, const std::string& pw);
std::string                 update(const std::string& name, uint8_t hair, uint8_t sex, uint8_t nation, uint8_t creature);

}

} }

} }

#endif // !__SQL_SERVICE_H__
