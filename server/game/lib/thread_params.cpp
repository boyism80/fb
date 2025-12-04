#include <fb/game/thread_params.h>
#include <fb/game/server.h>

using namespace fb::game;

thread_params::thread_params(server& server) :
    characters(server)
{ }

