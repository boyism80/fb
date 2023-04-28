#include <fb/game/npc.h>
#include <fb/game/context.h>

using namespace fb::game;

fb::game::npc::master::master(const fb::game::npc::master::config& config) : 
    fb::game::object::master(config),
    script(config.script)
{ }

fb::game::npc::master::~master()
{ }