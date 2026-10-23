#ifndef __BOT_CLIENT_PROFILE_H__
#define __BOT_CLIENT_PROFILE_H__

#include <fb/protocol/client_version.h>

#include <optional>
#include <string>
#include <string_view>

namespace fb::bot {

struct client_profile
{
    fb::protocol::CLIENT_VERSION version = fb::protocol::CLIENT_VERSION::v550;
    fb::protocol::CLIENT_UI_MODE ui_mode = fb::protocol::CLIENT_UI_MODE::OLD;

    std::string to_string() const;

    static std::optional<client_profile> parse(std::string_view text);
    static std::optional<client_profile> parse(uint16_t version, std::string_view ui_mode);
    static void                          configure(const client_profile& profile);
    static client_profile                configured();
    static client_profile                pick();
};

} // namespace fb::bot

#endif
