#include <fb/game/handler/amqp/set_datetime.h>

#include <fb/game/inventory.h>
#include <fb/game/map.h>
#include <fb/game/mob.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

set_datetime::set_datetime(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::SetDateTime>(server)
{ }

async::task<void> set_datetime::handle(const internal_resp::SetDateTime& message)
{
    if (message.error != 0)
        co_return;

    auto before = this->server.now();
    if (message.reset)
        this->server.reset_now_offset();
    else
        this->server.now(fb::model::datetime(message.datetime));
    auto shift = this->server.now() - before;

    this->server.characters.foreach_enqueue([shift](auto& ch) -> async::task<void> {
        for (auto i = 0; i < CONTAINER_CAPACITY; ++i)
        {
            auto spell = ch->spells[i];
            if (spell != nullptr)
                spell->delay(0);
        }
        ch->stat.shift_recovery(shift);
        ch->ping_state().last_ping_time += shift;
        co_return;
    });

    for (auto& [id, thread] : this->server.threads)
    {
        auto builder = thread->new_builder<void>();
        builder.func = [shift](auto& thread) -> async::task<void> {
            auto params = thread.template data<thread_params>();
            for (auto& rezen : params->rezens)
            {
                rezen->shift(shift);
            }

            for (auto& [map_id, map] : params->maps)
            {
                for (auto& [oid, obj] : map->objects)
                {
                    if (obj->is(OBJECT_TYPE::MOB) == false)
                        continue;

                    auto mob = std::static_pointer_cast<fb::game::mob>(obj);
                    mob->action_time(mob->action_time() + shift);
                }
            }
            co_return;
        };
        builder.enqueue();
    }
    co_return;
}
