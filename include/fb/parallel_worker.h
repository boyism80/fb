#ifndef __PARALLEL_WORKER_H__
#define __PARALLEL_WORKER_H__

#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <future>
#include <algorithm>
#include <fb/generator.h>

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
        auto queue     = std::queue<T>();
        auto indices   = std::unordered_map<T*, int>();
        auto buffer    = std::unordered_map<uint32_t, std::unique_ptr<std::vector<R>>>();
        auto processed = std::atomic<int>(0);

        std::mutex mutex_queue, mutex_buffer;

        auto gen_ready = this->on_ready();
        while (gen_ready.next())
        {
            auto input = gen_ready.value();
            queue.push(input);

            auto& added = queue.back();
            indices.insert({&added, indices.size()});
        }

        auto count = double(queue.size());
        auto fn    = [&, this]() {
            while (true)
            {
                T* input = nullptr;
                {
                    auto _ = std::lock_guard(mutex_queue);
                    if (queue.empty())
                        break;

                    input = &queue.front();
                    queue.pop();
                }

                auto index = indices.at(input);
                mutex_buffer.lock();
                buffer.insert({index, std::make_unique<std::vector<R>>()});
                mutex_buffer.unlock();

                try
                {
                    auto gen_work = this->on_work(*input);
                    while (gen_work.next())
                    {
                        auto output = gen_work.value();
                        buffer[index]->push_back(output);
                    }

                    for (auto& output : *buffer[index])
                    {
                        this->on_worked(*input, output, (++processed * 100) / count);
                    }
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

        auto keys = std::vector<uint32_t>();
        for (auto& [k, _] : buffer)
        {
            keys.push_back(k);
        }
        std::sort(keys.begin(), keys.end());

        for (auto k : keys)
        {
            for (auto& output : *buffer[k])
            {
                result.push_back(output);
            }
        }

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
        auto queue         = std::queue<T>();
        auto mutex_queue   = std::mutex();
        auto mutex_percent = std::mutex();
        auto processed     = 0;

        auto gen = this->on_ready();
        while (gen.next())
        {
            queue.push(gen.value());
        }

        auto count = double(queue.size());
        auto fn    = [&, this]() {
            while (true)
            {
                auto input = std::optional<T>{};
                {
                    auto _ = std::lock_guard(mutex_queue);
                    if (queue.empty())
                        break;

                    input = std::move(queue.front());
                    queue.pop();
                }

                try
                {
                    this->on_work(input.value());
                    {
                        auto _ = std::lock_guard(mutex_percent);
                        this->on_worked(input.value(), (++processed * 100) / count);
                    }
                }
                catch (std::exception& e)
                {
                    {
                        auto _ = std::lock_guard(mutex_percent);
                        processed++;
                    }
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