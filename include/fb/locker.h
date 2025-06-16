#ifndef __LOCKER_H__
#define __LOCKER_H__

#include <shared_mutex>
#include <functional>

namespace fb {

/**
 * @brief      A thread-safe wrapper that provides synchronized access to a value.
 *
 *             This template class wraps a value of type ValueType and provides
 *             thread-safe read and write access using a shared_mutex. It supports
 *             multiple concurrent readers or a single writer, following the
 *             readers-writer lock pattern.
 *
 * @tparam     ValueType  The type of value to protect with synchronization.
 */
template <typename ValueType>
class locker
{
private:
    mutable std::shared_mutex _mutex;
    ValueType                 _value;

public:
    /**
     * @brief      Constructs a locker with the given arguments forwarded to ValueType.
     *
     *             Perfect forwarding constructor that allows initialization of the
     *             wrapped value with any arguments that ValueType's constructor accepts.
     *
     * @param[in]  args  Arguments to forward to ValueType's constructor.
     *
     * @tparam     Args  Parameter pack for constructor arguments.
     */
    template <typename... Args>
    locker(Args&&... args) :
        _value(std::forward<Args>(args)...)
    { }

    /**
     * @brief      Copy constructor is deleted to prevent copying.
     */
    locker(const locker&) = delete;

    /**
     * @brief      Move constructor is deleted to prevent moving.
     */
    locker(locker&&) = delete;

    /**
     * @brief      Destructor is defaulted.
     */
    ~locker() = default;

    /**
     * @brief      Performs a write operation with exclusive access and returns a value.
     *
     *             Acquires an exclusive lock and executes the provided function
     *             with write access to the wrapped value. Only one thread can
     *             perform write operations at a time.
     *
     * @param[in]  fn    Function to execute with write access to the value.
     *
     * @tparam     ReturnType  The return type of the function.
     *
     * @return     The value returned by the function.
     */
    template <typename ReturnType>
    ReturnType write(const std::function<ReturnType(ValueType&)>& fn)
    {
        auto _ = std::unique_lock(this->_mutex);
        return fn(this->_value);
    }

    /**
     * @brief      Performs a write operation with exclusive access (void return).
     *
     *             Acquires an exclusive lock and executes the provided function
     *             with write access to the wrapped value. Only one thread can
     *             perform write operations at a time.
     *
     * @param[in]  fn    Function to execute with write access to the value.
     */
    void write(const std::function<void(ValueType&)>& fn)
    {
        auto _ = std::unique_lock(this->_mutex);
        fn(this->_value);
    }

    /**
     * @brief      Performs a read operation with shared access and returns a value.
     *
     *             Acquires a shared lock and executes the provided function
     *             with read-only access to the wrapped value. Multiple threads
     *             can perform read operations concurrently.
     *
     * @param[in]  fn    Function to execute with read access to the value.
     *
     * @tparam     ReturnType  The return type of the function.
     *
     * @return     The value returned by the function.
     */
    template <typename ReturnType>
    ReturnType read(const std::function<ReturnType(const ValueType&)>& fn) const
    {
        auto _ = std::shared_lock(this->_mutex);
        return fn(this->_value);
    }

    /**
     * @brief      Performs a read operation with shared access (void return).
     *
     *             Acquires a shared lock and executes the provided function
     *             with read-only access to the wrapped value. Multiple threads
     *             can perform read operations concurrently.
     *
     * @param[in]  fn    Function to execute with read access to the value.
     */
    void read(const std::function<void(const ValueType&)>& fn) const
    {
        auto _ = std::shared_lock(this->_mutex);
        fn(this->_value);
    }
};

} // namespace fb

#endif
