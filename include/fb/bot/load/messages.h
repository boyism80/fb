#ifndef __BOT_LOAD_MESSAGES_H__
#define __BOT_LOAD_MESSAGES_H__

#include <json/json.h>

#include <optional>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fb::bot::load {

class messages
{
private:
    std::unordered_map<std::string, std::string>    _exact;
    std::vector<std::pair<std::regex, std::string>> _patterns;

public:
    messages() = default;
    explicit messages(const Json::Value& json);

public:
    std::optional<std::string> classify(const std::string& text) const;
};

} // namespace fb::bot::load

#endif
