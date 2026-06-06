#include <model.additional.h>
#include <fb/model/model.h>
#include <string>

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
