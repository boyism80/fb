#include <model.additional.h>
#include <fb/model/model.h>
#include <string>

/**
 * @brief Checks if the given name contains any blocked substring
 *
 * This method iterates through all blocked names in the table and checks
 * if any of them appears as a substring within the input name.
 *
 * @param name The name to check for blocked substrings
 * @return true if the name contains any blocked substring, false otherwise
 */
bool fb::model::__blocked_name::contains_substring(std::string_view name) const
{
    for (auto& [key, blocked] : *this)
    {
        if (name.find(blocked.id) != std::string::npos)
            return true;
    }

    return false;
}
