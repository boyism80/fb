#ifndef __SQL_SERVICE_H__
#define __SQL_SERVICE_H__

#include <fb/core/query.h>
#include <fb/core/config.h>
#include <ctime>

namespace fb { namespace login { namespace query {

class session;
class item;
    
std::string                 make_insert(const std::string& name, const std::string& pw);
std::string                 make_update(const std::string& name, uint8_t hair, uint8_t sex, uint8_t nation, uint8_t creature);

} } }

#endif // !__SQL_SERVICE_H__
