#include <fb/game/handler/protocol/spell_cast.h>
#include <fb/game/server.h>

namespace game_reqs = fb::protocol::game::request;

namespace fb::game::handler::protocol {

template <fb::protocol::CLIENT_VERSION V>
spell_cast<V>::spell_cast(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::spell_cast<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> spell_cast<V>::handle(fb::socket<character>& session, game_reqs::spell_cast<V>& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return true;

    if (ch->role() == ROLE::USER && ENUM_IN(map->model().option, MAP_OPTION::DISABLE_SPELL))
    {
        ch->message(_TEXT(MESSAGE_SPELL_DISABLED_AREA));
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
        ch->message(std::format(_TEXT(MESSAGE_SPELL_DELAY), delay));
        co_return true;
    }

    const_cast<fb::protocol::game::request::spell_cast<V>&>(request).parse(spell->model().type);
    switch (spell->model().type)
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

template class spell_cast<fb::protocol::CLIENT_VERSION::v550>;
template class spell_cast<fb::protocol::CLIENT_VERSION::v565>;
template class spell_cast<fb::protocol::CLIENT_VERSION::v651>;

} // namespace fb::game::handler::protocol
