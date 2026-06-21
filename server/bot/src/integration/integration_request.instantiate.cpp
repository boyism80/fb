#include <fb/bot/controller.h>
#include <fb/bot/game_bot.h>
#include <fb/bot/integration/bulletin_bot.h>
#include <fb/bot/integration/dialog_bot.h>
#include <fb/bot/integration/dialog_ext_bot.h>
#include <fb/bot/integration/trade_bot.h>

namespace fb::bot {

using integration::bulletin_bot;
using integration::dialog_bot;
using integration::dialog_ext_bot;
using integration::trade_bot;

template async::task<dialog_bot> bot<game_bot>::request(const fb::protocol::header&                        protocol,
                                                        const std::function<bool(const dialog_bot& resp)>& condition,
                                                        const fb::model::timespan&                         timeout,
                                                        bool                                               encrypt,
                                                        bool                                               wrap);

template async::task<dialog_ext_bot>
bot<game_bot>::request(const fb::protocol::header&                            protocol,
                       const std::function<bool(const dialog_ext_bot& resp)>& condition,
                       const fb::model::timespan&                             timeout,
                       bool                                                   encrypt,
                       bool                                                   wrap);

template async::task<bulletin_bot> bot<game_bot>::request(const fb::protocol::header& protocol,
                                                          const fb::model::timespan&  timeout,
                                                          bool                        encrypt,
                                                          bool                        wrap);

template async::task<bulletin_bot>
bot<game_bot>::request(const fb::protocol::header&                          protocol,
                       const std::function<bool(const bulletin_bot& resp)>& condition,
                       const fb::model::timespan&                           timeout,
                       bool                                                 encrypt,
                       bool                                                 wrap);

template async::task<trade_bot> bot<game_bot>::request(const fb::protocol::header&                       protocol,
                                                       const std::function<bool(const trade_bot& resp)>& condition,
                                                       const fb::model::timespan&                        timeout,
                                                       bool                                              encrypt,
                                                       bool                                              wrap);

} // namespace fb::bot
