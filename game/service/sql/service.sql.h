#ifndef __SERVICE_OBJECT_H__
#define __SERVICE_OBJECT_H__

#include <boost/algorithm/string/join.hpp>
#include <string>
#include <map>

namespace fb { namespace game { 

class session;
class item;
    
namespace service { namespace sql {

std::string                 _dict_to_update(const std::map<std::string, std::string>& value);
std::string                 _vec_to_insert(const std::vector<std::string>& value);


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

#endif // !__SERVICE_OBJECT_H__
