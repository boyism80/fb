#ifndef __SQL_SERVICE_H__
#define __SQL_SERVICE_H__

#include <fb/core/query.h>
#include <fb/core/time.h>

namespace fb { namespace game { namespace query {

class session;
class item;
    
std::string                 make_update_session(fb::game::session& session);
std::string                 make_update_item(fb::game::session& session);
std::string                 make_delete_item(fb::game::session& session);
std::string                 make_update_spell(fb::game::session& session);
std::string                 make_delete_spell(fb::game::session& session);

} } }

#endif // !__SQL_SERVICE_H__
