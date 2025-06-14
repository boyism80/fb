#include <fb/game/context.h>
#include <fb/game/worker.h>

fb::game::map_loader::map_loader(fb::game::context& context) :
    _context(context),
    _bar("", "Load maps")
{ }

fb::generator<fb::game::map_loader::input_type> fb::game::map_loader::on_ready()
{
    for (auto& [k, v] : this->_context.model.map)
    {
        co_yield v;
    }
}

void fb::game::map_loader::on_work(const fb::game::map_loader::input_type& value)
{
    this->_context.maps.load(value.get());
}

void fb::game::map_loader::on_worked(const fb::game::map_loader::input_type& input, double percent)
{
    fb::console::progress("Loading map files", percent);
}

void fb::game::map_loader::on_error(const fb::game::map_loader::input_type& input, std::exception& e)
{
    fb::console::comment("    - {}", e.what());
}

void fb::game::map_loader::on_finish()
{
    fb::console::newline();
}