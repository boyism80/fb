#include <fb/game/handler/click.h>
#include <fb/game/server.h>

using namespace fb::game::handler;

click::click(fb::game::server& server) :
    fb::handler<fb::game::server, fb::protocol::game::request::click>(server)
{ }

void click::handle_click_mob(character& ch, mob& mob)
{
    ch.send(fb_resp::message(mob.name(), MESSAGE_TYPE::STATE));
}

void click::handle_click_npc(character& ch, npc& npc)
{
    auto& model = npc.based<fb::model::npc>();
    if (model.script.empty())
        return;

    auto lua = fb::lua::new_context();
    if (lua == nullptr)
        return;
#if defined DEBUG | defined _DEBUG
    lua->load("scripts/npc.lua");
    lua->load(model.script);
#endif
    lua->func(model.click);
    lua->pushobject(ch);
    lua->pushobject(npc);
    std::ignore = lua->call(2);
}

async::task<bool> click::handle(fb::socket<character>& session, fb::protocol::game::request::click& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    if (request.oid == 0xFFFFFFFF) // Press F1
        co_return true;

    if (request.oid == 0xFFFFFFFE) // Preff F2
        co_return true;

    auto map = ch->map();
    auto you = map->objects[request.oid];
    if (you == nullptr)
        co_return true;

    switch (you->what())
    {
    case OBJECT_TYPE::CHARACTER:
        ch->browse_ch(static_cast<character&>(*you));
        break;

    case OBJECT_TYPE::MOB:
        this->handle_click_mob(*ch, static_cast<mob&>(*you));
        break;

    case OBJECT_TYPE::NPC:
        this->handle_click_npc(*ch, static_cast<npc&>(*you));
        break;
    }

    co_return true;
}
