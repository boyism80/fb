#ifndef FB_GAME_POLLING_POLLING_H
#define FB_GAME_POLLING_POLLING_H

#include <fb/game/polling/system_mail_channel.h>
#include <fb/game/polling/storage_pending_channel.h>

namespace fb::game {

class server;

class polling
{
public:
    system_mail_channel     system_mail;
    storage_pending_channel storage_pending;

public:
    explicit polling(server& owner);
};

} // namespace fb::game

#endif // FB_GAME_POLLING_POLLING_H
