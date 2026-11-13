#ifndef __FB_GAME_REVISION_H__
#define __FB_GAME_REVISION_H__

#include <cstdint>

namespace fb::game {

enum class apply_result : uint8_t
{
    STALE,
    CURRENT,
    APPLIED,
    RELOAD,
};

template <typename F>
apply_result apply_revision(uint64_t& current, uint64_t revision, F&& change)
{
    if (revision < current)
    {
        return apply_result::STALE;
    }
    else if (revision == current)
    {
        return apply_result::CURRENT;
    }
    else if (revision == current + 1)
    {
        change();
        current = revision;
        return apply_result::APPLIED;
    }
    else
    {
        return apply_result::RELOAD;
    }
}

} // namespace fb::game

#endif // !__FB_GAME_REVISION_H__
