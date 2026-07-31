#ifndef __MODEL_LOADER_H__
#define __MODEL_LOADER_H__

#include <fb/parallel_worker.h>
#include <fb/model/model.h>
#include <fb/encoding.h>
#include <async/task.h>
#include <async/task_completion_source.h>
#include <thread>

namespace fb::model {

class loader : public fb::parallel_worker<std::reference_wrapper<fb::model::slot_base>>
{
public:
    using input_type = std::reference_wrapper<fb::model::slot_base>;

public:
    explicit loader(fb::async_executor& executor) :
        fb::parallel_worker<input_type>(executor)
    { }

    ~loader() = default;

protected:
    fb::generator<input_type> on_ready()
    {
        auto buffer = std::vector<std::reference_wrapper<fb::model::slot_base>>();
        table::foreach ([&, this](auto& slot) {
            buffer.push_back(slot);
        });

        for (auto& slot : buffer)
        {
            co_yield slot;
        }
    }

    async::task<void> on_work(const input_type& value)
    {
        value.get().load();
        co_return;
    }

    void on_worked(const input_type& input, double percent)
    {
        fb::console::progress("Loading data files", percent);
    }

    void on_error(const input_type& input, std::exception& e)
    {
        fb::console::comment("    - {}", e.what());
    }

    void on_finish()
    {
        fb::console::progress("Loading data files", 100.f);
        fb::console::newline();
    }
};

inline async::task<void> reload_async()
{
    auto promise = std::make_shared<async::task_completion_source<void>>();
    std::thread([promise]() {
        try
        {
#ifdef _WIN32
            option::decoding(fb::cp949);
            option::encoding(fb::utf8);
#endif
            table::reload();
#ifdef _WIN32
            option::decoding(nullptr);
#endif
            promise->set_value();
        }
        catch (...)
        {
#ifdef _WIN32
            option::decoding(nullptr);
#endif
            promise->set_exception(std::current_exception());
        }
    }).detach();
    co_await promise->task();
}

} // namespace fb::model

#endif
