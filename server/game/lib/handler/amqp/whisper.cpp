#include <fb/game/handler/amqp/whisper.h>
#include <fb/game/server.h>

using namespace fb::game::handler::amqp;

whisper::whisper(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::Whisper>(server)
{ }

async::task<void> whisper::handle(const internal_resp::Whisper& message)
{
    if (message.host == fb::config<uint16_t>("id"))
        co_return;

    this->server.on_whisper(message);
}
