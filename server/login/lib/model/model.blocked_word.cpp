#include <model.additional.h>
#include <fb/model/model.h>
#include <string>

/**
 * @brief Checks if the given name contains any blocked word substring
 *
 * This method iterates through all blocked words in the table and checks
 * if any of them (src field) appears as a substring within the input name.
 *
 * @param name The name to check for blocked word substrings
 * @return true if the name contains any blocked word substring, false otherwise
 */
bool fb::model::__blocked_word::contains_substring(std::string_view name) const
{
    for (auto& [key, blocked] : *this)
    {
        if (name.find(blocked.src) != std::string::npos)
            return true;
    }

    return false;
}

/**
 * @brief Filters chat message by replacing blocked words
 *
 * This method iterates through all blocked words in the table and replaces
 * any occurrence of src with dst in the message.
 *
 * @param message The chat message to filter
 * @return Filtered message with blocked words replaced
 */
std::string fb::model::__blocked_word::filter(std::string_view message) const
{
    auto filtered = std::string{message};

    for (auto& [key, blocked] : *this)
    {
        if (blocked.src.empty())
            continue;

        size_t pos = 0;
        while ((pos = filtered.find(blocked.src, pos)) != std::string::npos)
        {
            filtered.replace(pos, blocked.src.length(), blocked.dst);
            pos += blocked.dst.length();
        }
    }

    return filtered;
}
