#ifndef __EXECUTION_CONTEXT_H__
#define __EXECUTION_CONTEXT_H__

#include <async/propagation.h>
#include <cstddef>
#include <memory>
#include <unordered_map>

namespace fb {

class async_executor;

class execution_context
{
public:
    using slot_key     = std::size_t;
    using slot_storage = std::unordered_map<slot_key, std::shared_ptr<void>>;

    static std::shared_ptr<execution_context> create();

    [[nodiscard]] static std::shared_ptr<execution_context> current();
    [[nodiscard]] static std::shared_ptr<execution_context> active();
    static void                                             active(std::shared_ptr<execution_context> value);

    [[nodiscard]] static async::propagation::token token();
    [[nodiscard]] static async::propagation::token token(async::propagation::token value);
    [[nodiscard]] static async::propagation::token token(std::shared_ptr<execution_context> value);

    [[nodiscard]] static std::shared_ptr<execution_context> frame(async::propagation::token value);

    static void pending(async::propagation::token value);

    static void install_propagation_hooks(async_executor& executor);

    template <typename T>
    void slot(slot_key id, T value)
    {
        (*this->_slots)[id] = std::make_shared<T>(std::move(value));
    }

    template <typename T>
    [[nodiscard]] T* slot(slot_key id) const
    {
        if (this->_slots == nullptr)
            return nullptr;

        auto found = this->_slots->find(id);
        if (found == this->_slots->end())
            return nullptr;

        return static_cast<T*>(found->second.get());
    }

private:
    execution_context();

private:
    std::shared_ptr<slot_storage> _slots;
};

} // namespace fb

#endif // !__EXECUTION_CONTEXT_H__
