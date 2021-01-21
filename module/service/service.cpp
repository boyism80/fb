#include "service.h"

std::string fb::service::sql::dict_to_update(const std::map<std::string, std::string>& value)
{
    std::vector<std::string> parameters;
    for(auto pair : value)
        parameters.push_back(pair.first + "=" + pair.second);

    return boost::algorithm::join(parameters, ", ");
}

std::string fb::service::sql::vec_to_insert(const std::vector<std::string>& value)
{
    std::stringstream sstream;
    sstream << "(" << boost::algorithm::join(value, ", ") << ")";
    return sstream.str();

    return "";
}