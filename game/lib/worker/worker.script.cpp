#include <fb/game/context.h>
#include <fb/game/worker.h>

fb::game::script_loader::script_loader(fb::game::context& context) :
    _context(context)
{ }

fb::generator<std::string> fb::game::script_loader::on_ready()
{
    co_yield "scripts/spell.lua";
    co_yield "scripts/npc.lua";
    co_yield "scripts/interaction.lua";
    co_yield "scripts/command.lua";
    co_yield "scripts/script.lua";

    for (auto& [k, v] : this->_context.model.spell)
    {
        if (v.script != "")
            co_yield v.script;
    }

    for (auto& [k, v] : this->_context.model.item)
    {
        if (v.script != "")
            co_yield v.script;
    }
}

void fb::game::script_loader::on_work(const std::string& value)
{
    async::awaitable_get(fb::lua::dump(value));
}

void fb::game::script_loader::on_worked(const std::string& input, double percent)
{
    fb::console::put("* [{:0.2f}%] 스크립트 파일을 읽었습니다.", percent);
}

void fb::game::script_loader::on_error(const std::string& input, std::exception& e)
{
    fb::console::comment("    - {}", e.what());
}

void fb::game::script_loader::on_finish()
{
    fb::console::newline();
}