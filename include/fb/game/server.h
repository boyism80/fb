#ifndef __FB_GAME_H__
#define __FB_GAME_H__

#include <boost/algorithm/string/join.hpp>
#include <json/json.h>
#include <fstream>
#include <fb/acceptor.h>
#include <fb/game/protocol.h>
#include <fb/amqp.h>
#include <fb/game/listener.h>
#include <fb/game/thread_params.h>
#include <fb/game/map/container.h>
#include <fb/hash.h>
#include <fb/redis.h>
#include <fb/shard_container.h>
#include <fb/game/clan.h>
#include <fb/game/npc_interaction_handler.h>

using namespace fb::protocol::internal;
using namespace fb::protocol::internal::request;
namespace fb_reqs       = fb::protocol::game::request;
namespace fb_resp       = fb::protocol::game::response;
namespace internal      = fb::protocol::internal;
namespace internal_resp = fb::protocol::internal::response;

REGISTER_RESPONSE(fb::protocol::internal::request::Shutdown, fb::protocol::internal::response::Shutdown)
REGISTER_RESPONSE(fb::protocol::internal::request::WriteArticle, fb::protocol::internal::response::WriteArticle)
REGISTER_RESPONSE(fb::protocol::internal::request::DeleteArticle, fb::protocol::internal::response::DeleteArticle)
REGISTER_RESPONSE(fb::protocol::internal::request::CreateClan, fb::protocol::internal::response::CreateClan)
REGISTER_RESPONSE(fb::protocol::internal::request::DestroyClan, fb::protocol::internal::response::DestroyClan)
REGISTER_RESPONSE(fb::protocol::internal::request::SetClanTitle, fb::protocol::internal::response::SetClanTitle)
REGISTER_RESPONSE(fb::protocol::internal::request::JoinClan, fb::protocol::internal::response::JoinClan)
REGISTER_RESPONSE(fb::protocol::internal::request::LeaveClan, fb::protocol::internal::response::LeaveClan)
REGISTER_RESPONSE(fb::protocol::internal::request::KickClan, fb::protocol::internal::response::KickClan)
REGISTER_RESPONSE(fb::protocol::internal::request::BroadcastClan, fb::protocol::internal::response::BroadcastClan)
REGISTER_RESPONSE(fb::protocol::internal::request::Logout, fb::protocol::internal::response::Logout)
REGISTER_RESPONSE(fb::protocol::internal::request::Save, fb::protocol::internal::response::Save)
REGISTER_RESPONSE(fb::protocol::internal::request::Broadcast, fb::protocol::internal::response::Broadcast)
REGISTER_RESPONSE(fb::protocol::internal::request::EnterGroup, fb::protocol::internal::response::EnterGroup)
REGISTER_RESPONSE(fb::protocol::internal::request::BroadcastGroup, fb::protocol::internal::response::BroadcastGroup)
REGISTER_RESPONSE(fb::protocol::internal::request::Login, fb::protocol::internal::response::Login)
REGISTER_RESPONSE(fb::protocol::internal::request::LeaveGroup, fb::protocol::internal::response::LeaveGroup)
REGISTER_RESPONSE(fb::protocol::internal::request::KickGroup, fb::protocol::internal::response::KickGroup)
REGISTER_RESPONSE(fb::protocol::internal::request::SetOption, fb::protocol::internal::response::SetOption)
REGISTER_RESPONSE(fb::protocol::internal::request::WriteMail, fb::protocol::internal::response::WriteMail)
REGISTER_RESPONSE(fb::protocol::internal::request::DeleteMail, fb::protocol::internal::response::DeleteMail)
REGISTER_RESPONSE(fb::protocol::internal::request::Whisper, fb::protocol::internal::response::Whisper)
REGISTER_RESPONSE(fb::protocol::internal::request::Transfer, fb::protocol::internal::response::Transfer)
REGISTER_RESPONSE(fb::protocol::internal::request::ChangeClanRole, fb::protocol::internal::response::ChangeClanRole)

namespace fb::game {

enum class scope
{
    PIVOT, ///< Operation affects only the pivot/center object
    GROUP, ///< Operation affects the entire group
    MAP,   ///< Operation affects all objects on the current map
    WORLD  ///< Operation affects the entire game world
};

class server : public fb::acceptor<fb::game::character>
{
public:
    LUA_PROTOTYPE

public:
    using object_set         = std::unordered_map<const fb::game::object*, std::unique_ptr<fb::game::object>>;
    using protocol_generator = std::function<std::unique_ptr<fb::protocol::header>(const fb::game::object&)>;
    using clan_ptr           = std::shared_ptr<fb::game::clan>;
    using group_ptr          = std::shared_ptr<fb::game::group>;

private:
    fb::model::datetime                                             _time;
    std::vector<std::unique_ptr<fb::game::npc_interaction_handler>> _npc_interaction_handlers;
    fb::redis                                                       _redis;

public:
    fb::game::listener_impl listener;

public:
    fb::model::model                                        model;
    fb::game::map_container                                 maps;
    fb::game::character::container                          characters;
    fb::sharded_container<clan_ptr, 16>                     clans;
    fb::sharded_container<group_ptr, 16>                    groups;
    fb::sharded_container<map::cache_bytes, 1024, uint64_t> map_update_cache;

public:
    server(boost::asio::io_context& io_context, uint16_t port);
    server(const server&) = delete;
    ~server();

public:
    /**
     * @brief      Creates or updates a group and executes a callback with thread-safe access.
     *
     *             This method ensures thread-safe access to group data by creating a new group
     *             if it doesn't exist or accessing an existing one. The provided callback
     *             function is executed with exclusive access to the group data.
     *
     * @param[in]  gid   The group ID to create or access.
     * @param[in]  fn    The callback function to execute with the group lock.
     */
    async::task<void> upsert_group_then(uint32_t gid, const std::function<void(group_ptr&)>& fn);

    /**
     * @brief      Creates or updates a group with specific members and executes a callback.
     *
     *             This overload allows creating a group with predefined master and member list.
     *             If the group already exists, it updates the member information. The callback
     *             is executed with thread-safe access to the group data.
     *
     * @param[in]  gid      The group ID to create or access.
     * @param[in]  master   The name of the group master/leader.
     * @param[in]  members  The list of member names in the group.
     * @param[in]  fn       The callback function to execute with the group lock.
     */
    async::task<void> upsert_group_then(uint32_t gid, const std::string& master, const std::vector<std::string>& members, const std::function<void(group_ptr&)>& fn);

    /**
     * @brief      Updates clan information with data from internal protocol.
     *
     * @param      clan   The clan object to update
     * @param      resp1  The clan information response
     * @param[in]  resp2  The clan member list response
     */
    void update_clan(clan& clan, fb::protocol::internal::Clan& resp1, const std::vector<fb::protocol::internal::ClanMember>& resp2) const;

    /**
     * @brief      Updates or inserts a clan and executes a function with it.
     *
     * @param[in]  id    The clan identifier
     * @param[in]  fn    The function to execute with the clan lock
     */
    async::task<void> upsert_clan_then(uint32_t id, std::function<async::task<void>(std::shared_ptr<fb::game::clan>&)> fn);

private:
    /**
     * @brief      Registers an NPC interaction handler.
     *
     * @tparam     HandlerType  The handler class type
     */
    template <typename HandlerType>
    void bind_npc_interaction()
    {
        auto handler = std::make_unique<HandlerType>(*this);
        this->_npc_interaction_handlers.push_back(std::move(handler));
    }

public:
    /**
     * @brief      Validates whisper response and throws exception if invalid.
     *
     * @param[in]  response  The whisper response to validate.
     */
    void assert_whisper(const internal::response::Whisper& response) const;

    /**
     * @brief      Validates group operation and throws exception if error occurs.
     *
     * @param[in]  error  The error code from group operation.
     * @param[in]  actor  The actor name involved in the group operation.
     */
    void assert_group(uint32_t error, const std::string& actor) const;

    /**
     * @brief      Validates clan operation and throws exception if error occurs.
     *
     * @param[in]  error  The error code from clan operation.
     */
    void assert_clan(uint32_t error) const;

    /**
     * @brief      Validates mail operation and throws exception if error occurs.
     *
     * @param[in]  error  The error code from mail operation.
     */
    void assert_mail(uint32_t error) const;

public:
    /**
     * @brief      Called when a character enters a group.
     *
     * @param[in]  resp  The group entry response containing group information.
     */
    async::task<void> on_enter_group(const internal_resp::EnterGroup& resp);

    /**
     * @brief      Called when a character leaves a group.
     *
     * @param[in]  resp  The group leave response containing departure details.
     */
    async::task<void> on_leave_group(const internal_resp::LeaveGroup& resp);

    /**
     * @brief      Called when a character is kicked from a group.
     *
     * @param[in]  resp  The group kick response containing the kicked character.
     */
    async::task<void> on_kick_group(const internal_resp::KickGroup& resp);

public:
    /**
     * @brief      Called when a server-wide broadcast message is received.
     *
     * @param[in]  resp  The broadcast response containing the message and metadata.
     */
    void on_broadcast(const internal_resp::Broadcast& resp);

    /**
     * @brief      Called when a group broadcast message is received.
     *
     * @param[in]  resp  The group broadcast response containing the message and group info.
     */
    void on_group_broadcast(const internal_resp::BroadcastGroup& resp);

    /**
     * @brief      Called when a clan broadcast message is received.
     *
     * @param[in]  resp  The clan broadcast response containing the message and clan info.
     */
    async::task<void> on_clan_broadcast(const internal_resp::BroadcastClan& resp);

    /**
     * @brief      Called when a clan's title/motto is changed.
     *
     * @param[in]  resp  The clan title change response containing the new title.
     */
    async::task<void> on_clan_title_changed(const internal_resp::SetClanTitle& resp);

    /**
     * @brief      Called when a new member joins a clan.
     *
     * @param[in]  resp  The clan join response containing member information.
     */
    async::task<void> on_clan_join_member(const internal_resp::JoinClan& resp);

    /**
     * @brief      Called when a member leaves a clan.
     *
     * @param[in]  resp  The clan leave response containing departure details.
     */
    async::task<void> on_clan_leave_member(const internal_resp::LeaveClan& resp);

    /**
     * @brief      Called when a member is kicked from a clan.
     *
     * @param[in]  resp  The clan kick response containing the kicked character.
     */
    async::task<void> on_clan_kick_member(const internal_resp::KickClan& resp);

    /**
     * @brief      Called when a mail message is written/sent.
     *
     * @param[in]  resp  The mail write response containing delivery status.
     */
    void on_write_mail(const internal_resp::WriteMail& resp);

    /**
     * @brief      Called when a whisper message is received.
     *
     * @param[in]  resp  The whisper response containing the message and sender info.
     */
    void on_whisper(const internal_resp::Whisper& resp);

    /**
     * @brief      Called when a clan member position is changed.
     *
     * @param[in]  resp  The clan position change response containing the updated member information.
     */
    async::task<void> on_clan_change_role(const internal_resp::ChangeClanRole& resp);

public:
    /**
     * @brief      Creates a new game object managed by shared_ptr with the server as first parameter.
     *
     *             This is the new preferred method for creating objects that supports
     *             automatic lifetime management and safe async operations.
     *
     * @param      args  The constructor arguments for the object.
     *
     * @tparam     T     The type of object to create.
     * @tparam     Args  Variadic template arguments for object construction.
     *
     * @return     Shared pointer to the newly created object.
     */
    template <typename T, typename... Args>
    std::shared_ptr<T> make(Args&&... args)
    {
        auto ptr = std::make_shared<T>(*this, std::forward<Args>(args)...);
        if constexpr (std::is_base_of_v<fb::game::object, T>)
        {
            ptr->on_init();
        }
        return ptr;
    }

    /**
     * @brief      Forces a rezen spawn regardless of normal spawn conditions.
     *
     *             This method bypasses the normal spawn restrictions and immediately
     *             spawns mobs at this spawn point. Useful for special events,
     *             GM commands, or scripted scenarios that need to override normal
     *             spawn logic.
     */
    void rezen_force();

    /**
     * @brief      Forces a rezen spawn regardless of normal spawn conditions.
     *
     *             This method bypasses the normal spawn restrictions and immediately
     *             spawns mobs at this spawn point. Useful for special events,
     *             GM commands, or scripted scenarios that need to override normal
     *             spawn logic.
     *
     * @param[in]  map  The map to spawn mobs on.
     */
    void rezen_force(const fb::game::map& map);

public:
    /**
     * @brief      Destroys a game object with proper cleanup and map removal.
     *
     * @param      obj           The object to destroy.
     * @param[in]  destroy_type  The type of destruction to perform.
     *
     * @tparam     T             The type of object to destroy.
     *
     * @return     An async task that completes when destruction is finished.
     */
    template <typename T>
    [[nodiscard]] async::task<void> destroy(T& obj, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT)
    {
        if constexpr (std::is_same_v<T, fb::game::object>)
        {
            auto shared_ptr = obj.template shared_from_this_as<fb::game::object>();
            std::ignore     = co_await shared_ptr->map(nullptr, fb::model::point16_t{0, 0}, destroy_type);
        }
        co_return;
    }

public:
    /**
     * @brief      Sends a protocol message to objects within the specified scope.
     *
     * @param      object        The source object sending the message.
     * @param[in]  header        The protocol header containing the message data.
     * @param[in]  scope         The scope determining which objects receive the message.
     * @param[in]  exclude_self  Whether to exclude the source object from receiving the message.
     * @param[in]  encrypt       Whether to encrypt the message before sending.
     *
     * @return     An async task that completes when the message is sent.
     */
    async::task<void> send(fb::game::object& object, const fb::protocol::header& header, fb::game::scope scope, bool exclude_self = false, bool encrypt = true);

    /**
     * @brief      Saves character data to persistent storage.
     *
     * @param      ch    The character to save.
     *
     * @return     An async task that completes when the character is saved.
     */
    [[nodiscard]] async::task<void> save(fb::game::character& ch);

public:
    /**
     * @brief      Gets the thread ID for the specified character socket.
     *
     * @param[in]  socket  The character socket to get the thread ID for.
     *
     * @return     The thread ID assigned to handle this socket.
     */
    virtual uint32_t thread_id(const fb::socket<fb::game::character>& socket) const;

    /**
     * @brief      Gets the thread responsible for handling the specified map.
     *
     * @param[in]  map   The map to get the handling thread for.
     *
     * @return     Pointer to the thread that handles this map.
     */
    fb::thread* thread(const fb::game::map& map);

    /**
     * @brief      Gets the current time.
     *
     * @return     The current time.
     */
    const fb::model::datetime& time() const;

    /**
     * @brief      Broadcasts a message to all players based on the broadcast type.
     *
     * @param[in]  message         The message content to broadcast.
     * @param[in]  type            The message type for display formatting.
     * @param[in]  broadcast_type  The scope of the broadcast (server-wide, etc.).
     *
     * @return     An async task that completes when the broadcast is sent.
     */
    [[nodiscard]] async::task<void> broadcast(const std::string& message, MESSAGE_TYPE type, BROADCAST_TYPE broadcast_type);

    /**
     * @brief      Creates a group with the specified character as leader.
     *
     * @param      me      The character who will become the group leader.
     * @param[in]  target  The name of the character to invite to the group.
     *
     * @return     An async task returning true if the group was created successfully.
     */
    [[nodiscard]] async::task<bool> create_group(character& me, const std::string& target);

    /**
     * @brief      Leaves the character from their current group.
     *
     *             This method sends a request to the server to leave the character from their current group.
     *             It returns a task that completes when the leave operation is complete.
     *
     * @param      me    The character who will leave the group.
     *
     * @return     A task that completes when the leave operation is complete
     */
    [[nodiscard]] async::task<void> leave_group(character& me);

    /**
     * @brief      Kicks a character from a group by the group master.
     *
     * @param[in]  group   The group to kick the character from.
     * @param[in]  kicker  The name of the character performing the kick.
     * @param[in]  target  The name of the character to kick.
     *
     * @return     An async task that completes when the character is kicked from the group.
     */
    [[nodiscard]] async::task<void> kick_group_member(const group& group, const std::string& kicker, const std::string& target);

    /**
     * @brief      Broadcasts a message to all members of a group.
     *
     * @param[in]  group    The group to broadcast the message to.
     * @param[in]  message  The message content to send.
     * @param[in]  type     The message type for display formatting.
     *
     * @return     An async task that completes when the broadcast is sent.
     */
    [[nodiscard]] async::task<void> broadcast(const group& group, const std::string& message, MESSAGE_TYPE type);

    /**
     * @brief      Creates a new clan with the specified character as leader.
     *
     * @param      me    The character who will become the clan leader.
     * @param[in]  name  The name of the clan to create. (rvalue)
     *
     * @return     An async task that completes when the clan is created.
     */
    [[nodiscard]] async::task<void> create_clan(character& me, std::string name);

    /**
     * @brief      Destroys the clan that the character leads.
     *
     * @param      me    The character who is the clan leader requesting destruction.
     *
     * @return     An async task that completes when the clan is destroyed.
     */
    [[nodiscard]] async::task<void> destroy_clan(character& me);

    /**
     * @brief      Sets the title/motto of a clan.
     *
     * @param[in]  changer_uid  The character updating the title.
     * @param[in]  title        The new title/motto to set.
     *
     * @return     An async task that completes when the title is updated.
     */
    [[nodiscard]] async::task<void> set_clan_title(uint32_t changer_uid, std::string title);

    /**
     * @brief      Adds a character as a member to a clan through invitation.
     *
     * @param      inviter  The character performing the invitation (must have Mate or higher position).
     * @param      invitee  The character to be invited to the clan.
     *
     * @return     An async task that completes when the character joins the clan.
     */
    [[nodiscard]] async::task<void> join_clan_member(character& inviter, character& invitee);

    /**
     * @brief      Kicks a character from a clan by an authorized member.
     *
     * @param[in]  clan    The clan to kick the character from.
     * @param[in]  kicker  The name of the character performing the kick.
     * @param[in]  target  The name of the character to kick.
     *
     * @return     An async task that completes when the character is kicked from the clan.
     */
    [[nodiscard]] async::task<void> kick_clan_member(const clan& clan, const std::string& kicker, const std::string& target);

    /**
     * @brief      Changes the position of a clan member by an authorized member.
     *
     * @param[in]  clan         The clan to change the member position in.
     * @param[in]  changer_uid  The character performing the position change.
     * @param[in]  target       The name of the character whose position will be changed.
     * @param[in]  role         The new position to assign to the target member.
     *
     * @return     An async task that completes when the position change is processed.
     */
    [[nodiscard]] async::task<void> change_clan_member_role(const clan& clan, uint32_t changer_uid, const std::string& target, CLAN_ROLE role);

    /**
     * @brief      Broadcasts a message to all members of a clan.
     *
     * @param[in]  clan     The clan to broadcast the message to.
     * @param[in]  message  The message content to send.
     * @param[in]  type     The message type for display formatting.
     *
     * @return     An async task that completes when the broadcast is sent.
     */
    [[nodiscard]] async::task<void> broadcast(const clan& clan, const std::string& message, MESSAGE_TYPE type);

    /**
     * @brief      Sends a mail message to another character.
     *
     * @param[in]  ch        The character sending the mail.
     * @param[in]  to        The name of the character to send mail to.
     * @param[in]  title     The subject/title of the mail.
     * @param[in]  contents  The body content of the mail.
     *
     * @return     An async task returning the mail sending response.
     */
    [[nodiscard]] async::task<internal_resp::WriteMail> send_mail(const character& ch, const std::string& to, const std::string& title, const std::string& contents);

    /**
     * @brief      Gets a list of mail messages for a character.
     *
     * @param[in]  ch      The character to get mail list for.
     * @param[in]  offset  The starting offset for pagination.
     * @param[in]  count   The maximum number of mails to retrieve.
     *
     * @return     An async task returning the mail list response.
     */
    [[nodiscard]] async::task<internal_resp::GetMailList> mail_list(const character& ch, uint16_t offset, uint16_t count);

    /**
     * @brief      Reads the content of a specific mail message.
     *
     * @param      ch    The character reading the mail.
     * @param[in]  id    The unique identifier of the mail to read.
     *
     * @return     An async task returning the mail content response.
     */
    [[nodiscard]] async::task<internal_resp::GetMail> read_mail(character& ch, uint16_t id);

    /**
     * @brief      Deletes a mail message from a character's mailbox.
     *
     * @param      ch    The character deleting the mail.
     * @param[in]  id    The unique identifier of the mail to delete.
     *
     * @return     An async task returning the mail deletion response.
     */
    [[nodiscard]] async::task<internal_resp::DeleteMail> delete_mail(character& ch, uint16_t id);

    /**
     * @brief      Gets a list of bulletin articles from a specific section.
     *
     * @param[in]  section  The bulletin section identifier to retrieve articles from.
     * @param[in]  offset   The starting offset for pagination.
     *
     * @return     An async task returning a list of bulletin articles.
     */
    [[nodiscard]] async::task<std::list<bulletin::article>> bulletin_list(uint16_t section, uint16_t offset);

    /**
     * @brief      Reads the content of a specific bulletin article.
     *
     * @param[in]  section  The bulletin section identifier.
     * @param[in]  id       The unique identifier of the article to read.
     *
     * @return     An async task returning the bulletin article content.
     */
    [[nodiscard]] async::task<bulletin::article> read_bulletin(uint16_t section, uint16_t id);

    /**
     * @brief      Writes a new article to a bulletin section.
     *
     * @param      ch        The character writing the bulletin article.
     * @param[in]  section   The bulletin section to write to.
     * @param[in]  title     The title of the article.
     * @param[in]  contents  The content body of the article.
     *
     * @return     An async task that completes when the article is written.
     */
    [[nodiscard]] async::task<void> write_bulletin(character& ch, uint16_t section, const std::string& title, const std::string& contents);

    /**
     * @brief      Deletes a bulletin article from a section.
     *
     * @param      ch       The character deleting the article.
     * @param[in]  section  The bulletin section containing the article.
     * @param[in]  id       The unique identifier of the article to delete.
     *
     * @return     An async task that completes when the article is deleted.
     */
    [[nodiscard]] async::task<void> delete_bulletin(character& ch, uint16_t section, uint16_t id);

    /**
     * @brief      Sends a private whisper message between characters.
     *
     * @param[in]  sender        The character sending the whisper.
     * @param[in]  receiver_name The name of the character to send the whisper to.
     * @param[in]  message       The whisper message content.
     *
     * @return     An async task that completes when the whisper is sent.
     */
    [[nodiscard]] async::task<void> whisper(character& sender, std::string receiver_name, std::string message);

protected:
    /**
     * @brief      Determines the decryption policy for incoming packets.
     *
     * @param[in]  cmd   The command byte of the incoming packet.
     *
     * @return     True if the packet should be decrypted, false otherwise.
     */
    bool decrypt_policy(uint8_t cmd) const override final;

    /**
     * @brief      Validates TPS (transactions per second) limits for a socket.
     *
     * @param      socket  The character socket to validate TPS for.
     *
     * @return     True if TPS limits are within acceptable range, false otherwise.
     */
    bool assert_tps(const fb::socket<fb::game::character>& socket) const override final;

    /**
     * @brief      Declares AMQP queues required for game server communication.
     *
     * @param      amqp  The AMQP socket to use for queue declaration.
     */
    void handle_init_amqp(fb::amqp::socket& amqp) override final;

    /**
     * @brief      Handles game server startup initialization.
     *
     * @return     An async task that completes when startup is finished.
     */
    [[nodiscard]] async::task<void> handle_start() override final;

    /**
     * @brief      Handles successful character connection establishment.
     *
     * @param      ch    The character socket that has connected.
     *
     * @return     An async task returning true if connection handling succeeded.
     */
    [[nodiscard]] async::task<bool> handle_connected(fb::socket<fb::game::character>& ch) override final;

    /**
     * @brief      Handles character disconnection and cleanup.
     *
     * @param      ch    The character socket that has disconnected.
     *
     * @return     An async task returning true if disconnection handling succeeded.
     */
    [[nodiscard]] async::task<bool> handle_disconnected(fb::socket<fb::game::character>& ch) override final;

    /**
     * @brief      Handles new character socket acceptance.
     *
     *             Creates a new character instance with the provided socket.
     *             The socket reference is used to obtain a weak_ptr internally
     *             within the character for lifecycle management.
     *
     * @param      socket  The newly accepted character socket.
     *
     * @return     Pointer to the created character object, or nullptr if failed.
     */
    std::shared_ptr<fb::game::character> handle_accepted(fb::socket<fb::game::character>& socket) override final;

protected:
    /**
     * @brief      Gets the unique identifier of this game server.
     *
     * @return     The server ID as configured in the settings.
     */
    uint8_t id() const
    {
        return fb::config<uint8_t>("id");
    }
    /**
     * @brief      Gets the service type for this game server.
     *
     * @return     The service type identifier for game servers.
     */
    Service service() const
    {
        return Service::Game;
    }

public:
    /**
     * @brief      Updates server status and heartbeat information in Redis.
     */
    void update_status();

    /**
     * @brief      Updates the server time.
     */
    void update_time();

    /**
     * @brief      General NPC interaction handler that processes player messages.
     *
     * @param      ch       The character interacting with NPCs.
     * @param[in]  message  The player's input message to process.
     * @param[in]  npcs     The list of NPCs available for interaction.
     *
     * @return     True if any NPC interaction was handled successfully, false otherwise.
     */
    async::task<bool> npc_interaction(character& ch, const std::string& message, const std::vector<std::shared_ptr<fb::game::npc>>& npcs);

    /**
     * @brief      Removes a character from a clan.
     *
     * @param[in]  clan  The clan to remove the character from.
     * @param[in]  name  The name of the character to remove.
     *
     * @return     An async task that completes when the character leaves the clan.
     */
    [[nodiscard]] async::task<void> leave_clan_member(const clan& clan, const std::string& name);

    /**
     * @brief      Adds a character as a member to a clan.
     *
     * @param[in]  clan  The clan to join the character to.
     * @param      ch    The character to add as a clan member.
     *
     * @return     An async task that completes when the character joins the clan.
     */
    [[nodiscard]] async::task<void> join_clan_member(const clan& clan, character& inviter, character& invitee);
};

} // namespace fb::game

/**
 * @brief      Creates a new game object managed by shared_ptr with the server as first parameter.
 *
 *             This is the new preferred method for creating objects that supports
 *             automatic lifetime management and safe async operations.
 *
 * @param      args  The constructor arguments for the object.
 *
 * @tparam     T     The type of object to create.
 * @tparam     Args  Variadic template arguments for object construction.
 *
 * @return     Shared pointer to the newly created object.
 */
template <typename T, typename... Args>
std::shared_ptr<T> fb::model::object::make(fb::game::server& server, Args&&... args) const
{
    return server.template make<T>(*this, std::forward<Args>(args)...);
}

#endif // !__FB_GAME_H__
