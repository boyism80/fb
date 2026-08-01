#ifndef FB_GAME_HANDLER_PROTOCOL_POPUP_INPUT_SUBMIT_H
#define FB_GAME_HANDLER_PROTOCOL_POPUP_INPUT_SUBMIT_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

/**
 * Handler for C2S popup_input_submit (0x23) from popup_input dismiss.
 * Resumes ch->dialog with the edited text (same session slot as ch:input).
 */
class popup_input_submit : public fb::handler::protocol<fb::game::server, game_reqs::popup_input_submit>
{
public:
    popup_input_submit(fb::game::server& server) :
        fb::handler::protocol<fb::game::server, game_reqs::popup_input_submit>(server)
    { }
    popup_input_submit(const popup_input_submit&)             = delete;
    popup_input_submit(popup_input_submit&&)                  = delete;
    popup_input_submit& operator= (const popup_input_submit&) = delete;
    popup_input_submit& operator= (popup_input_submit&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::popup_input_submit& request) override;
};

} // namespace fb::game::handler::protocol

#endif
