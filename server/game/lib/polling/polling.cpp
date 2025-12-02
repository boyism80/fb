#include <fb/game/polling/polling.h>
#include <fb/game/server.h>

using namespace fb::game;

polling::polling(server& owner) :
    system_mail(owner),
    storage_pending(owner)
{ }
