#ifndef FB_GAME_ESCROW_H
#define FB_GAME_ESCROW_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace fb::game {

// A held value keeps its container index, so the index stays taken while it is held.
template <typename T, size_t N>
class escrow
{
public:
    using value_ptr = std::shared_ptr<T>;

    struct part
    {
        uint8_t   index = 0;
        value_ptr value;
    };

    struct held
    {
        uint64_t          money = 0;
        std::vector<part> parts;
    };

private:
    struct slot
    {
        uint64_t  id = 0;
        value_ptr value;
    };

private:
    std::array<std::optional<slot>, N> _slots   = {};
    std::map<uint64_t, uint64_t>       _entries = {};
    uint64_t                           _money   = 0;

public:
    bool contains(uint64_t id) const
    {
        return this->_entries.contains(id);
    }

    bool reserved(uint8_t index) const
    {
        return index < N && this->_slots[index].has_value();
    }

    value_ptr at(uint8_t index) const
    {
        if (this->reserved(index) == false)
            return nullptr;

        return this->_slots[index]->value;
    }

    std::optional<uint64_t> id(uint8_t index) const
    {
        if (this->reserved(index) == false)
            return std::nullopt;

        return this->_slots[index]->id;
    }

    std::vector<uint8_t> indices() const
    {
        auto result = std::vector<uint8_t>{};
        for (size_t i = 0; i < N; i++)
        {
            if (this->_slots[i].has_value())
                result.push_back(static_cast<uint8_t>(i));
        }
        return result;
    }

    std::vector<part> parts(uint64_t id) const
    {
        auto result = std::vector<part>{};
        for (size_t i = 0; i < N; i++)
        {
            if (this->_slots[i].has_value() && this->_slots[i]->id == id)
                result.push_back(part{static_cast<uint8_t>(i), this->_slots[i]->value});
        }
        return result;
    }

    const std::map<uint64_t, uint64_t>& entries() const
    {
        return this->_entries;
    }

    uint64_t money() const
    {
        return this->_money;
    }

    // Holds the money and every part, or nothing when the id is taken or any index is invalid, taken or repeated.
    bool hold(uint64_t id, uint64_t money, const std::vector<part>& parts)
    {
        if (this->contains(id))
            return false;

        auto claimed = std::array<bool, N>{};
        for (const auto& part : parts)
        {
            if (part.index >= N || part.value == nullptr || this->_slots[part.index].has_value() || claimed[part.index])
                return false;

            claimed[part.index] = true;
        }

        this->_entries[id]  = money;
        this->_money       += money;
        for (const auto& part : parts)
        {
            this->_slots[part.index] = slot{id, part.value};
        }
        return true;
    }

    // Loading rebuilds an entry from its money row and its value rows, which arrive in any order.
    void restore(uint64_t id, uint64_t money)
    {
        this->_entries[id] += money;
        this->_money       += money;
    }

    bool restore(uint64_t id, uint8_t index, value_ptr value)
    {
        if (index >= N || value == nullptr || this->_slots[index].has_value())
            return false;

        this->_entries.try_emplace(id, 0);
        this->_slots[index] = slot{id, std::move(value)};
        return true;
    }

    std::optional<held> remove(uint64_t id)
    {
        auto found = this->_entries.find(id);
        if (found == this->_entries.end())
            return std::nullopt;

        auto result = held{found->second, this->parts(id)};
        for (const auto& part : result.parts)
        {
            this->_slots[part.index].reset();
        }

        this->_money -= found->second;
        this->_entries.erase(found);
        return result;
    }

    void swap(uint8_t src, uint8_t dst)
    {
        if (src >= N || dst >= N)
            return;

        std::swap(this->_slots[src], this->_slots[dst]);
    }
};

} // namespace fb::game

#endif // FB_GAME_ESCROW_H
