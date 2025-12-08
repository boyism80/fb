#ifndef FB_GAME_CHANNEL_SYSTEM_MAIL_CHANNEL_H
#define FB_GAME_CHANNEL_SYSTEM_MAIL_CHANNEL_H

#include <async/task.h>
#include <fb/locker.h>
#include <functional>
#include <vector>

namespace fb::game {

class server;
struct system_mail;

class system_mail_channel
{
private:
    server&                              _owner;
    fb::locker<std::vector<system_mail>> _data;

public:
    explicit system_mail_channel(server& owner);

    [[nodiscard]] async::task<void> fetch();
    [[nodiscard]] async::task<void> append(const system_mail& mail);
    void                            read(std::function<void(const std::vector<system_mail>&)> fn);
    [[nodiscard]] async::task<void> read_async(std::function<async::task<void>(const std::vector<system_mail>&)> fn);
};

} // namespace fb::game

#endif // FB_GAME_CHANNEL_SYSTEM_MAIL_CHANNEL_H

