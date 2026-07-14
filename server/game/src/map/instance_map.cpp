#include <fb/game/instance_map.h>
#include <fb/game/server.h>

using namespace fb::game;

instance_map::instance_map(fb::game::server&                     server,
                           uint32_t                              id,
                           uint32_t                              slot,
                           const std::shared_ptr<fb::game::map>& source,
                           const void*                           data,
                           size_t                                size) :
    map(server, id, source->model, true, data, size),
    _source(source),
    _slot(slot)
{ }

instance_map::~instance_map()
{ }

bool instance_map::is_instance() const
{
    return true;
}

std::shared_ptr<fb::game::map> instance_map::source() const
{
    return this->_source;
}

uint32_t instance_map::slot() const
{
    return this->_slot;
}

bool instance_map::closing() const
{
    return this->_closing;
}

void instance_map::on_character_enter()
{
    if (this->_closing)
        return;

    this->_activated = true;
    this->_characters++;
}

void instance_map::on_character_leave()
{
    if (this->_characters > 0)
        this->_characters--;

    if (this->_closing)
        return;

    if (this->_activated == false)
        return;

    if (this->_characters > 0)
        return;

    this->_closing = true;
    this->schedule_destroy();
}

bool instance_map::begin_destroy()
{
    if (this->_destroying)
        return false;

    this->_closing    = true;
    this->_destroying = true;
    return true;
}

void instance_map::schedule_destroy()
{
    auto thread = this->thread();
    if (thread == nullptr)
        return;

    auto self    = this->shared_from_this_as<fb::game::map>();
    auto builder = thread->new_builder<void>();
    builder.func = [self](auto&) -> async::task<void> {
        co_await const_cast<fb::game::server&>(self->server).maps.destroy(self);
    };
    builder.enqueue();
}
