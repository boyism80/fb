#ifndef FB_MODEL_SUBSTRING_MATCHER_H
#define FB_MODEL_SUBSTRING_MATCHER_H

// aho_corasick.hpp uses std::numeric_limits without including <limits>.
// Newer libstdc++ no longer pulls it in transitively, so include it first.
#include <limits>

#include <aho_corasick/aho_corasick.hpp>

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace fb::model {

class substring_matcher
{
public:
    substring_matcher()
    {
        this->_trie.remove_overlaps();
    }

    void add_pattern(std::string_view pattern)
    {
        if (pattern.empty())
            return;

        this->_trie.insert(std::string(pattern));
    }

    void add_replacement(std::string_view src, std::string_view dst)
    {
        if (src.empty())
            return;

        this->_replacements.emplace_back(dst);
        this->_trie.insert(std::string(src));
    }

    bool contains(std::string_view text) const
    {
        if (text.empty())
            return false;

        auto emits = this->_trie.parse_text(std::string(text));
        return emits.empty() == false;
    }

    std::string filter(std::string_view message) const
    {
        if (message.empty())
            return {};

        auto text  = std::string(message);
        auto emits = this->_trie.parse_text(text);
        if (emits.empty())
            return text;

        std::sort(emits.begin(), emits.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.get_start() < rhs.get_start();
        });

        auto filtered = std::string{};
        filtered.reserve(text.size());

        auto pos = size_t{0};
        for (const auto& emit : emits)
        {
            if (emit.get_start() < pos)
                continue;

            filtered.append(text, pos, emit.get_start() - pos);

            auto index = emit.get_index();
            if (index < this->_replacements.size())
                filtered.append(this->_replacements[index]);
            else
                filtered.append(emit.get_keyword());

            pos = emit.get_end() + 1;
        }

        filtered.append(text, pos);
        return filtered;
    }

private:
    mutable aho_corasick::trie _trie;
    std::vector<std::string>   _replacements;
};

} // namespace fb::model

#endif // FB_MODEL_SUBSTRING_MATCHER_H
