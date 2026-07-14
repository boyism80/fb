#include <fb/game/handler/protocol/spell_cast.h>
#include <fb/game/server.h>

using namespace fb::game::handler::protocol;

namespace game_reqs = fb::protocol::game::request;

spell_cast::spell_cast(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::spell_cast>(server)
{ }

async::task<bool> spell_cast::handle(fb::socket<character>& session, game_reqs::spell_cast& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return true;

    if (ch->role() == ROLE::USER && ENUM_IN(map->model.option, MAP_OPTION::DISABLE_SPELL))
    {
        co_await ch->message(_TEXT(MESSAGE_SPELL_DISABLED_AREA));
        co_return true;
    }

    if (request.slot > CONTAINER_CAPACITY - 1)
        co_return false;

    auto spell = ch->spells[request.slot];
    if (spell == nullptr)
        co_return false;

    auto delay = spell->delay();
    if (delay > 0)
    {
        co_await ch->message(std::format(_TEXT(MESSAGE_SPELL_DELAY), delay));
        co_return true;
    }

    const_cast<fb::protocol::game::request::spell_cast&>(request).parse(spell->model.type);
    switch (spell->model.type)
    {
    case SPELL_TYPE::INPUT:
        ch->active(*spell, request.message);
        break;

    case SPELL_TYPE::TARGET:
        ch->active(*spell, request.oid);
        break;

    case SPELL_TYPE::NORMAL:
        ch->active(*spell);
        break;
    }

    co_return true;
}
