#include <fb/core/query.h>

std::string fb::query::make_update(const std::map<std::string, std::string>& value)
{
    std::vector<std::string> parameters;
    for(auto& [key, value] : value)
        parameters.push_back(key + "=" + value);

    return boost::algorithm::join(parameters, ", ");
}

std::string fb::query::make_insert(const std::vector<std::string>& value)
{
    std::stringstream sstream;
    sstream << "(" << boost::algorithm::join(value, ", ") << ")";
    return sstream.str();

    return "";
}