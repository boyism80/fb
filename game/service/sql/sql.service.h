#ifndef __SQL_SERVICE_H__
#define __SQL_SERVICE_H__

#include "module/service/service.h"
#include "module/time/time.h"

namespace fb { namespace game { 

class session;
class item;
    
namespace service { namespace sql {

namespace session {

std::string                 update(fb::game::session& session);

}



namespace item {

std::string                 update(fb::game::session& session);

}



namespace spell {

std::string                 update(fb::game::session& session);

}

} }

} }

#endif // !__SQL_SERVICE_H__
