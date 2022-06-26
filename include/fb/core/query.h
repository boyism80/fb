#ifndef __SERVICE_H__
#define __SERVICE_H__

#include <boost/algorithm/string/join.hpp>
#include <string>
#include <vector>
#include <map>
#include <sstream>

namespace fb { namespace query {

std::string                 make_update(const std::map<std::string, std::string>& value);
std::string                 make_insert(const std::vector<std::string>& value);

} }

#endif // !__SERVICE_H__
