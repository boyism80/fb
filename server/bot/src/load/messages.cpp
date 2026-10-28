#include <fb/bot/load/messages.h>

#include <optional>
#include <regex>
#include <string>

using namespace fb::bot::load;

messages::messages(const Json::Value& json)
{
    for (auto& entry : json["messages"])
    {
        auto kind = entry.get("kind", "").asString();
        if (entry.isMember("pattern"))
            this->_patterns.push_back({std::regex(entry["pattern"].asString()), kind});
        else
            this->_exact.insert({entry["text"].asString(), kind});
    }
}

std::optional<std::string> messages::classify(const std::string& text) const
{
    auto found = this->_exact.find(text);
    if (found != this->_exact.end())
        return found->second;

    for (auto& [pattern, kind] : this->_patterns)
    {
        if (std::regex_match(text, pattern))
            return kind;
    }
    return std::nullopt;
}
