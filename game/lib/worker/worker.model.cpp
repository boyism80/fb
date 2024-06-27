#include <fb/game/worker.h>
#include <fb/game/context.h>

fb::game::model_loader::model_loader(fb::game::context& context) : _context(context)
{ }

fb::generator<fb::game::model_loader::input_type> fb::game::model_loader::on_ready()
{
    auto buffer = std::vector<std::reference_wrapper<fb::model::container>>();
    this->_context.model.foreach([&, this] (auto& container)
    {
        buffer.push_back(container);
    });

    for (auto& container : buffer)
    {
        co_yield container;
    }
}

void fb::game::model_loader::on_work(const fb::game::model_loader::input_type& value)
{
    value.get().load();
}

void fb::game::model_loader::on_worked(const fb::game::model_loader::input_type& input, double percent)
{
    auto& c = fb::console::get();
    c.put("* [%0.2lf%%] 데이터를 읽었습니다.", percent);
}

void fb::game::model_loader::on_error(const fb::game::model_loader::input_type& input, std::exception& e)
{
    auto& c = fb::console::get();
    c.comment("    - %s", e.what());
}

void fb::game::model_loader::on_finish()
{
    auto& c = fb::console::get();
    c.next();
}