#ifndef __GENERATOR_H__
#define __GENERATOR_H__

#include <coroutine>
#include <optional>
#include <stdexcept>

namespace fb {

/**
 * @brief      A C++20 coroutine-based generator that can yield values and optionally receive input.
 *
 *             This template class implements a generator pattern using C++20 coroutines.
 *             It supports both output-only generators and bidirectional generators that
 *             can receive input values. The generator maintains state between yields
 *             and provides an iterator-like interface for consuming generated values.
 *
 * @tparam     OUTPUT  The type of values yielded by the generator.
 * @tparam     INPUT   The type of input values the generator can receive (default: void for output-only).
 */
template <typename OUTPUT, typename INPUT = void>
struct generator
{
    /**
     * @brief      Input marker type used in coroutines to request input values.
     */
    struct input
    { };

    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;

    /**
     * @brief      Awaitable type that provides access to input values within the coroutine.
     */
    struct passthru_value
    {
        INPUT& ret_;

        /**
         * @brief      Indicates that the awaitable is always ready.
         *
         * @return     Always returns true.
         */
        bool await_ready()
        {
            return true;
        }

        /**
         * @brief      Called when the coroutine is suspended (no-op for this awaitable).
         *
         * @param[in]  handle  The coroutine handle.
         */
        void await_suspend(coro_handle)
        { }

        /**
         * @brief      Returns the input value when the coroutine resumes.
         *
         * @return     Reference to the input value.
         */
        INPUT& await_resume()
        {
            return ret_;
        }
    };

    /**
     * @brief      Promise type that manages the coroutine's state and behavior.
     *
     *             Handles value yielding, input receiving, and coroutine lifecycle.
     */
    struct promise_type
    {
        std::optional<OUTPUT> current_value;
        INPUT                 input_value;

        /**
         * @brief      Creates the generator object from this promise.
         *
         * @return     A generator instance associated with this promise.
         */
        auto get_return_object()
        {
            return generator{coro_handle::from_promise(*this)};
        }

        /**
         * @brief      Specifies that the coroutine should suspend initially.
         *
         * @return     suspend_always to pause execution at the start.
         */
        auto initial_suspend()
        {
            return std::suspend_always{};
        }

        /**
         * @brief      Specifies that the coroutine should suspend when finishing.
         *
         * @return     suspend_always to pause execution at the end.
         */
        auto final_suspend() noexcept
        {
            return std::suspend_always{};
        }

        /**
         * @brief      Handles unhandled exceptions by terminating the program.
         */
        void unhandled_exception()
        {
            std::terminate();
        }

        /**
         * @brief      Handles yield expressions in the coroutine.
         *
         * @param[in]  value  The value to yield.
         *
         * @return     suspend_always to pause execution after yielding.
         */
        auto yield_value(OUTPUT value)
        {
            current_value = value;
            return std::suspend_always{};
        }

        /**
         * @brief      Handles return statements without values.
         */
        void return_void()
        { }

        /**
         * @brief      Transforms input requests into awaitable objects.
         *
         * @param[in]  inp  The input marker.
         *
         * @return     An awaitable that provides access to the input value.
         */
        auto await_transform(input inp)
        {
            return passthru_value{input_value};
        }
    };

    /**
     * @brief      Advances the generator to the next value.
     *
     * @return     True if a new value is available, false if the generator is exhausted.
     */
    bool next()
    {
        return coro ? (coro.resume(), !coro.done()) : false;
    }

    /**
     * @brief      Gets the current yielded value.
     *
     * @return     The current value from the generator.
     *
     * @throws     std::runtime_error if no value is available.
     */
    OUTPUT value()
    {
        auto& promise = coro.promise();
        if (promise.current_value.has_value() == false)
            throw std::runtime_error("has no value");

        return promise.current_value.value();
    }

    /**
     * @brief      Sends an input value to the generator (copy version).
     *
     * @param[in]  input  The input value to send to the generator.
     */
    void send(const INPUT& input)
    {
        coro.promise().input_value = input;
    }

    /**
     * @brief      Sends an input value to the generator (move version).
     *
     * @param[in]  input  The input value to move to the generator.
     */
    void send(INPUT&& input)
    {
        coro.promise().input_value = std::move(input);
    }

    /**
     * @brief      Copy constructor is deleted to prevent copying.
     */
    generator(generator const& rhs) = delete;

    /**
     * @brief      Move constructor transfers ownership of the coroutine.
     *
     * @param[in]  rhs  The generator to move from.
     */
    generator(generator&& rhs) :
        coro(rhs.coro)
    {
        rhs.coro = nullptr;
    }

    /**
     * @brief      Destructor that cleans up the coroutine handle.
     */
    ~generator()
    {
        if (coro)
            coro.destroy();
    }

private:
    /**
     * @brief      Private constructor used by the promise type.
     *
     * @param[in]  h     The coroutine handle.
     */
    generator(coro_handle h) :
        coro(h)
    { }
    coro_handle coro;
};

/**
 * @brief      Specialization of generator for output-only generators (no input).
 *
 *             This specialization provides a simpler interface for generators
 *             that only yield values without receiving input. It removes
 *             input-related functionality while maintaining the same basic
 *             generator interface.
 *
 * @tparam     OUTPUT  The type of values yielded by the generator.
 */
template <typename OUTPUT>
struct generator<OUTPUT, void>
{
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;

    /**
     * @brief      Promise type for output-only generators.
     *
     *             Simplified promise that only handles value yielding without
     *             input processing capabilities.
     */
    struct promise_type
    {
        std::optional<OUTPUT> current_value;

        /**
         * @brief      Creates the generator object from this promise.
         *
         * @return     A generator instance associated with this promise.
         */
        auto get_return_object()
        {
            return generator{coro_handle::from_promise(*this)};
        }

        /**
         * @brief      Specifies that the coroutine should suspend initially.
         *
         * @return     suspend_always to pause execution at the start.
         */
        auto initial_suspend()
        {
            return std::suspend_always{};
        }

        /**
         * @brief      Specifies that the coroutine should suspend when finishing.
         *
         * @return     suspend_always to pause execution at the end.
         */
        auto final_suspend() noexcept
        {
            return std::suspend_always{};
        }

        /**
         * @brief      Handles unhandled exceptions by terminating the program.
         */
        void unhandled_exception()
        {
            std::terminate();
        }

        /**
         * @brief      Handles yield expressions in the coroutine.
         *
         * @param[in]  value  The value to yield.
         *
         * @return     suspend_always to pause execution after yielding.
         */
        auto yield_value(OUTPUT value)
        {
            current_value = value;
            return std::suspend_always{};
        }

        /**
         * @brief      Handles return statements without values.
         */
        void return_void()
        { }
    };

    /**
     * @brief      Advances the generator to the next value.
     *
     * @return     True if a new value is available, false if the generator is exhausted.
     */
    bool next()
    {
        return coro ? (coro.resume(), !coro.done()) : false;
    }

    /**
     * @brief      Gets the current yielded value.
     *
     * @return     The current value from the generator.
     *
     * @throws     std::runtime_error if no value is available.
     */
    OUTPUT value()
    {
        auto& promise = coro.promise();
        if (promise.current_value.has_value() == false)
            throw std::runtime_error("has no value");

        return promise.current_value.value();
    }

    /**
     * @brief      Copy constructor is deleted to prevent copying.
     */
    generator(generator const& rhs) = delete;

    /**
     * @brief      Move constructor transfers ownership of the coroutine.
     *
     * @param[in]  rhs  The generator to move from.
     */
    generator(generator&& rhs) :
        coro(rhs.coro)
    {
        rhs.coro = nullptr;
    }

    /**
     * @brief      Destructor that cleans up the coroutine handle.
     */
    ~generator()
    {
        if (coro)
            coro.destroy();
    }

private:
    /**
     * @brief      Private constructor used by the promise type.
     *
     * @param[in]  h     The coroutine handle.
     */
    generator(coro_handle h) :
        coro(h)
    { }
    coro_handle coro;
};

} // namespace fb

#endif // !__GENERATOR_H__
