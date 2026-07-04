#ifndef __MODEL_LOADER_H__
#define __MODEL_LOADER_H__

#include <fb/parallel_worker.h>
#include <fb/model/model.h>
#include <async/task.h>

namespace fb::model {

class loader : public fb::parallel_worker<std::reference_wrapper<fb::model::container>>
{
public:
    using input_type = std::reference_wrapper<fb::model::container>;

public:
    explicit loader(fb::async_executor& executor) :
        fb::parallel_worker<input_type>(executor)
    { }

    ~loader() = default;

protected:
    fb::generator<input_type> on_ready()
    {
        auto buffer = std::vector<std::reference_wrapper<fb::model::container>>();
        table::foreach ([&, this](auto& container) {
            buffer.push_back(container);
        });

        for (auto& container : buffer)
        {
            co_yield container;
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

} // namespace fb::model

#endif
