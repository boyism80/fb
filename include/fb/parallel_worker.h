#ifndef __PARALLEL_WORKER_H__
#define __PARALLEL_WORKER_H__

/**
 * @file    parallel_worker.h
 * @brief   Template framework for parallel processing with automatic work distribution
 * @author  FB Development Team
 *
 * @details This file implements a comprehensive template framework for parallel processing
 *          that automatically distributes work across multiple threads and collects results.
 *          The system uses a producer-consumer pattern with thread-safe work distribution
 *          and result collection, ideal for CPU-intensive tasks that can be parallelized.
 *
 *          Key features:
 *          - Template-based parallel processing framework for type-safe operations
 *          - Automatic work distribution across available CPU cores
 *          - Thread-safe work queue management with mutex protection
 *          - Result collection in original order for deterministic output
 *          - Progress tracking with percentage completion callbacks
 *          - Exception handling and error reporting for individual work items
 *          - Specialization for void return types (side-effect only processing)
 *          - Generator-based work item production for memory-efficient processing
 *          - Configurable thread pool size based on hardware capabilities
 *          - Comprehensive callback system for monitoring and error handling
 *
 * @note    This parallel processing framework is used throughout the FB 2D MMORPG
 *          server for data processing tasks such as map generation, asset processing,
 *          and batch operations that can benefit from multi-threaded execution.
 */

#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <future>
#include <algorithm>
#include <fb/generator.h>
#include <fb/locker.h>

namespace fb {

/**
 * @brief      Template class for parallel processing of work items with output collection.
 *
 *             This class provides a framework for distributing work across multiple threads
 *             and collecting the results. It uses a producer-consumer pattern where work
 *             items are generated, processed in parallel, and results are collected in
 *             the original order. Ideal for CPU-intensive tasks that can be parallelized.
 *
 * @tparam     T     The type of input work items to be processed.
 * @tparam     R     The type of output results produced by processing.
 */
template <typename T, typename R = void>
class parallel_worker
{
protected:
    /**
     * @brief      Gets the ready generator.
     *
     * @return     The ready generator.
     */
    virtual fb::generator<T> on_ready() = 0;
    /**
     * @brief      Gets the work generator.
     *
     * @param[in]  value  The value.
     *
     * @return     The work generator.
     */
    virtual fb::generator<R> on_work(const T& value) = 0;

    /**
     * @brief      Gets the worked generator.
     *
     * @param[in]  input  The input.
     * @param[in]  output  The output.
     * @param[in]  percent  The percent.
     */
    virtual void on_worked(const T& input, const R& output, double percent)
    { }
    /**
     * @brief      Gets the error generator.
     *
     * @param[in]  input  The input.
     * @param[in]  e  The exception.
     */
    virtual void on_error(const T& input, std::exception& e)
    { }
    /**
     * @brief      Gets the finish generator.
     *
     * @param[in]  result  The result.
     */
    virtual void on_finish(const std::vector<R>& result)
    { }

public:
    /**
     * @brief      Runs the parallel worker.
     *
     * @param[in]  result  The result.
     */
    void run(std::vector<R>& result)
    {
        auto indices   = std::unordered_map<T*, int>();
        auto processed = std::atomic<int>(0);

        auto queue  = fb::locker<std::queue<T>>();
        auto buffer = fb::locker<std::unordered_map<uint32_t, std::unique_ptr<std::vector<R>>>>();

        auto gen_ready = this->on_ready();
        while (gen_ready.next())
        {
            auto input = gen_ready.value();
            queue.write([&input, &indices](auto& q) {
                q.push(input);
                auto& added = q.back();
                indices.insert({&added, indices.size()});
            });
        }

        auto count = queue.read([](const auto& q) {
            return double(q.size());
        });
        auto fn    = [&, this]() {
            while (true)
            {
                T*   input       = nullptr;
                bool queue_empty = queue.write([&input](auto& q) -> bool {
                    if (q.empty())
                        return true;

                    input = &q.front();
                    q.pop();
                    return false;
                });

                if (queue_empty)
                    break;

                auto index = indices.at(input);
                buffer.write([index](auto& buf) {
                    buf.insert({index, std::make_unique<std::vector<R>>()});
                });

                try
                {
                    auto gen_work = this->on_work(*input);
                    while (gen_work.next())
                    {
                        auto output = gen_work.value();
                        buffer.write([index, &output](auto& buf) {
                            buf[index]->push_back(output);
                        });
                    }

                    buffer.read([this, input, index, &processed, count](const auto& buf) {
                        for (auto& output : *buf.at(index))
                        {
                            this->on_worked(*input, output, (++processed * 100) / count);
                        }
                    });
                }
                catch (std::exception& e)
                {
                    this->on_error(*input, e);
                    processed++;
                }
            }
        };

        auto tasks = std::vector<std::future<void>>();
        for (int i = 0; i < std::thread::hardware_concurrency(); i++)
        {
            tasks.push_back(std::async(std::launch::async, fn));
        }

        for (auto& task : tasks)
        {
            task.wait();
        }

        auto keys = buffer.read([](const auto& buf) {
            auto result_keys = std::vector<uint32_t>();
            for (auto& [k, _] : buf)
            {
                result_keys.push_back(k);
            }
            std::sort(result_keys.begin(), result_keys.end());
            return result_keys;
        });

        buffer.read([&result, &keys](const auto& buf) {
            for (auto k : keys)
            {
                for (auto& output : *buf.at(k))
                {
                    result.push_back(output);
                }
            }
        });

        this->on_finish(result);
    }
};

/**
 * @brief      Template specialization for parallel processing without output collection.
 *
 *             This specialization provides a framework for distributing work across
 *             multiple threads when no output collection is needed. Work items are
 *             processed in parallel for their side effects only. Ideal for tasks
 *             like file processing, database updates, or other operations where
 *             the processing itself is the goal rather than collecting results.
 *
 * @tparam     T     The type of input work items to be processed.
 */
template <typename T>
class parallel_worker<T, void>
{
protected:
    /**
     * @brief      Gets the ready generator.
     *
     * @return     The ready generator.
     */
    virtual fb::generator<T> on_ready() = 0;
    /**
     * @brief      Gets the work generator.
     *
     * @param[in]  value  The value.
     */
    virtual void on_work(const T& value) = 0;
    virtual void on_worked(const T& input, double percent)
    { }
    /**
     * @brief      Gets the error generator.
     *
     * @param[in]  input  The input.
     * @param[in]  e  The exception.
     */
    virtual void on_error(const T& input, std::exception& e)
    { }
    /**
     * @brief      Gets the finish generator.
     */
    virtual void on_finish()
    { }

public:
    /**
     * @brief      Runs the parallel worker.
     */
    void run()
    {
        fb::locker<std::queue<T>> queue;
        fb::locker<int>           processed(0);

        auto gen = this->on_ready();
        while (gen.next())
        {
            queue.write([&gen](auto& q) {
                q.push(gen.value());
            });
        }

        auto count = queue.read([](const auto& q) {
            return double(q.size());
        });
        auto fn    = [&, this]() {
            while (true)
            {
                auto input       = std::optional<T>{};
                bool queue_empty = queue.write([&input](auto& q) -> bool {
                    if (q.empty())
                        return true;

                    input = std::move(q.front());
                    q.pop();
                    return false;
                });

                if (queue_empty)
                    break;

                try
                {
                    this->on_work(input.value());
                    auto current_progress = processed.write([count](auto& p) {
                        return (++p * 100) / count;
                    });
                    this->on_worked(input.value(), current_progress);
                }
                catch (std::exception& e)
                {
                    processed.write([](auto& p) {
                        p++;
                    });
                    this->on_error(input.value(), e);
                }
            }
        };

        auto tasks = std::vector<std::future<void>>();
        for (int i = 0; i < std::thread::hardware_concurrency(); i++)
        {
            tasks.push_back(std::async(std::launch::async, fn));
        }

        for (auto& task : tasks)
        {
            task.wait();
        }

        this->on_finish();
    }
};

} // namespace fb

#endif