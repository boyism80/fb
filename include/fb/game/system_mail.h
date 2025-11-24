#ifndef FB_GAME_SYSTEM_MAIL_H
#define FB_GAME_SYSTEM_MAIL_H

#include <cstdint>
#include <string>
#include <optional>
#include <fb/model/datetime.h>

namespace fb::protocol::internal {
class SystemMail;
}

namespace fb::game {

/**
 * @brief Data Access Object (DAO) for system mail in the game server.
 * Represents a system mail message that can be sent to all users.
 * This is the in-memory representation used by the game server.
 */
struct system_mail
{
    /**
     * @brief Unique identifier for the system mail.
     */
    uint32_t id;

    /**
     * @brief Sender's user ID for the system mail.
     */
    uint32_t sender;

    /**
     * @brief Title/subject of the system mail message.
     */
    std::string title;

    /**
     * @brief Body content of the system mail message.
     */
    std::string contents;

    /**
     * @brief Optional expiration date for the system mail.
     * If null, the mail never expires.
     */
    std::optional<fb::model::datetime> expire_date;

    /**
     * @brief Creation date of the system mail.
     */
    fb::model::datetime created_date;

    /**
     * @brief Default constructor.
     */
    system_mail() = default;

    /**
     * @brief Constructs a system mail from protocol DTO.
     * @param dto The FlatBuffer protocol DTO containing system mail data.
     */
    explicit system_mail(const fb::protocol::internal::SystemMail& dto);
};

} // namespace fb::game

#endif // FB_GAME_SYSTEM_MAIL_H
