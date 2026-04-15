#include <fb/game/server.h>
#include <fb/game/worker.h>

using namespace fb::game;
using table = fb::model::table;

map_loader::map_loader(server& server) :
    _server(server)
{ }

fb::generator<map_loader::input_type> map_loader::on_ready()
{
    for (auto& [k, v] : table::map)
    {
        co_yield v;
    }
}

void map_loader::on_work(const map_loader::input_type& value)
{
    this->_server.maps.load(value.get());
}

void map_loader::on_worked(const map_loader::input_type& input, double percent)
{
    fb::console::progress("Loading map files", percent);
}

void map_loader::on_error(const map_loader::input_type& input, std::exception& e)
{
    fb::console::comment("    - {}", e.what());
}

void map_loader::on_finish()
{
    fb::console::newline();
}