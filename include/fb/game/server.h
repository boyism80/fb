#ifndef __FB_GAME_H__
#define __FB_GAME_H__

/**
 * @file    server.h
 * @brief   Main game server and world management system
 * @author  FB Development Team
 *
 * @details This file implements the central game server that coordinates
 *          all game systems and manages the complete game world state. The server
 *          serves as the main orchestrator for player connections, game objects,
 *          server-wide operations, and inter-service communication in the FB 2D MMORPG.
 *
 *          Key features:
 *          - Complete game world initialization and lifecycle management
 *          - Player character session management and authentication
 *          - Comprehensive map and object container management
 *          - Group and clan system coordination with sharded containers
 *          - Database integration with Redis for caching and persistence
 *          - AMQP integration for inter-service communication and messaging
 *          - Lua script execution environment for dynamic game logic
 *          - Thread pool management and work distribution across multiple threads
 *          - Real-time event processing and broadcasting to connected clients
 *          - Protocol handler registration and automatic message routing
 *          - Internal service communication (HTTP, AMQP) for distributed architecture
 *          - Comprehensive game mechanics coordination (combat, spells, trading, etc.)
 *
 * @note    The server is the singleton entry point for all game server
 *          operations and maintains the authoritative game state for the entire
 *          game world instance.
 */

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

/**
 * @brief      Enumeration defining the scope of operations within the game world.
 *
 *             This enum is used to determine the range or scope of various game operations
 *             such as message broadcasting, spell effects, or event notifications.
 */
enum class scope
{
    PIVOT, ///< Operation affects only the pivot/center object
    GROUP, ///< Operation affects the entire group
    MAP,   ///< Operation affects all objects on the current map
    WORLD  ///< Operation affects the entire game world
};

/**
 * @brief      The main game server that manages all game world operations.
 *
 *             This class extends the acceptor to provide comprehensive game server functionality.
 *             It manages the complete game world including characters, maps, NPCs, items, spells,
 *             groups, clans, and all game mechanics. The server serves as the central coordinator
 *             for all game operations and maintains the game state.
 *
 *             Key responsibilities:
 *             - Character session management and authentication
 *             - Game world state management (maps, objects, NPCs)
 *             - Group and clan system coordination
 *             - Inter-service communication via AMQP
 *             - Lua scripting integration for game logic
 *             - Real-time game event processing
 *             - Database synchronization via Redis
 *             - Protocol handler registration and dispatch
 */
class server : public fb::acceptor<fb::game::character>
{
public:
    LUA_PROTOTYPE

public:
    struct builtin;

public:
    using object_set           = std::unordered_map<const fb::game::object*, std::unique_ptr<fb::game::object>>;
    using transfer_param       = fb_reqs::login::transfer_param;
    using protocol_generator   = std::function<std::unique_ptr<fb::protocol::header>(const fb::game::object&)>;
    using npc_interaction_func = std::function<
        async::task<bool>(character&, const std::string&, const std::vector<std::shared_ptr<fb::game::npc>>&)>;
    using clan_ptr  = std::shared_ptr<fb::game::clan>;
    using group_ptr = std::shared_ptr<fb::game::group>;

private:
    fb::model::datetime               _time;
    std::vector<npc_interaction_func> _npc_interaction_funcs;
    fb::redis                         _redis;

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
    /**
     * @brief      Constructs a new game server with network and world initialization.
     *
     *             Initializes the game server with the specified I/O server and port.
     *             Sets up the game world, loads configuration, initializes Redis connection,
     *             and prepares all game systems for operation.
     *
     * @param      io_context   The boost::asio I/O context for network operations.
     * @param[in]  port         The TCP port number to listen on for client connections.
     */
    server(boost::asio::io_context& io_context, uint16_t port);

    /**
     * @brief      Copy constructor is explicitly deleted.
     *
     *             The game server cannot be copied as it manages unique resources
     *             like network connections, database connections, and game state.
     */
    server(const server&) = delete;

    /**
     * @brief      Destroys the game server and cleans up all resources.
     *
     *             Ensures proper cleanup of all game objects, network connections,
     *             database connections, and other resources before destruction.
     */
    ~server();

private:
    /**
     * @brief      Generates a human-readable elapsed time message from a datetime string.
     *
     *             Calculates the time difference between the provided datetime and the current
     *             time, then formats it into a user-friendly message (e.g., "2 hours ago",
     *             "3 days ago"). Used for displaying timestamps in game UI elements.
     *
     * @param[in]  dt    The datetime string to calculate elapsed time from.
     *
     * @return     A formatted string describing the elapsed time.
     */
    std::string elapsed_message(const std::string& dt);

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
    async::task<void> upsert_group_then(uint32_t                               gid,
                                        const std::string&                     master,
                                        const std::vector<std::string>&        members,
                                        const std::function<void(group_ptr&)>& fn);

    /**
     * @brief      Updates clan information with data from internal protocol.
     *
     * @param      clan   The clan object to update
     * @param      resp1  The clan information response
     * @param[in]  resp2  The clan member list response
     */
    void update_clan(clan&                                                  clan,
                     fb::protocol::internal::Clan&                          resp1,
                     const std::vector<fb::protocol::internal::ClanMember>& resp2) const;

    /**
     * @brief      Updates or inserts a clan and executes a function with it.
     *
     * @param[in]  id    The clan identifier
     * @param[in]  fn    The function to execute with the clan lock
     */
    async::task<void> upsert_clan_then(uint32_t                                                           id,
                                       std::function<async::task<void>(std::shared_ptr<fb::game::clan>&)> fn);

    /**
     * @brief      Initializes a character with data from the database.
     *
     * @param[in]  response  The character data response from database
     * @param      ch        The character object to initialize
     * @param[in]  group     The group ID the character belongs to (optional)
     * @param[in]  clan      The clan ID the character belongs to (optional)
     * @param[in]  transfer  Transfer parameters for map changes (optional)
     *
     * @return     True if initialization was successful, false otherwise
     */
    [[nodiscard]] async::task<bool> init_ch(const fb::protocol::internal::Character& response,
                                            fb::game::character&                     ch,
                                            std::optional<uint32_t>                  group,
                                            std::optional<uint32_t>                  clan,
                                            const std::optional<transfer_param>&     transfer);

    /**
     * @brief      Initializes character options from database response.
     *
     * @param[in]  response  The option data response from database
     * @param      ch        The character to set options for
     */
    void init_option(const fb::protocol::internal::Option& response, fb::game::character& ch);

    /**
     * @brief      Initializes character items from database response.
     *
     * @param[in]  response  The item data response from database
     * @param      ch        The character to set items for
     */
    void init_items(const std::vector<fb::protocol::internal::Item>& response, fb::game::character& ch);

    /**
     * @brief      Initializes character spells from database response.
     *
     * @param[in]  response  The spell data response from database
     * @param      ch        The character to set spells for
     */
    void init_spells(const std::vector<fb::protocol::internal::Spell>& response, fb::game::character& ch);

    /**
     * @brief      Initializes character quests from database response.
     *
     *             This method initializes the character's quest progress based on the
     *             provided quest data from the database. It updates the character's
     *             quest state with the new progress information.
     *
     * @param[in]  response  The quest data response from database
     * @param      ch        The character to set quests for
     */
    void init_quests(const std::vector<fb::protocol::internal::Quest>& response, fb::game::character& ch);

    /**
     * @brief      Initializes character achievements from database response.
     *
     * @param[in]  response  The achievement data response from database
     * @param      ch        The character to set achievements for
     */
    void init_achievements(const std::vector<fb::protocol::internal::Achievement>& response, fb::game::character& ch);

private:
    /**
     * @brief      Binds an NPC interaction handler function.
     *
     * @param      func  The NPC interaction handler function to bind
     *
     * @tparam     Func  The function type (auto-deduced)
     */
    template <typename Func>
    void bind_npc_interaction(Func&& func)
    {
        auto c_fn = std::bind(func, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        this->_npc_interaction_funcs.push_back(c_fn);
    }

private:
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
    async::task<void> send(fb::game::object&           object,
                           const fb::protocol::header& header,
                           fb::game::scope             scope,
                           bool                        exclude_self = false,
                           bool                        encrypt      = true);

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
     * @brief      Broadcasts a message to all players based on the broadcast type.
     *
     * @param[in]  message         The message content to broadcast.
     * @param[in]  type            The message type for display formatting.
     * @param[in]  broadcast_type  The scope of the broadcast (server-wide, etc.).
     *
     * @return     An async task that completes when the broadcast is sent.
     */
    [[nodiscard]] async::task<void> broadcast(const std::string& message,
                                              MESSAGE_TYPE       type,
                                              BROADCAST_TYPE     broadcast_type);

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
    [[nodiscard]] async::task<void> kick_group_member(const group&       group,
                                                      const std::string& kicker,
                                                      const std::string& target);

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
    [[nodiscard]] async::task<void> kick_clan_member(const clan&        clan,
                                                     const std::string& kicker,
                                                     const std::string& target);

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
    [[nodiscard]] async::task<void>
    change_clan_member_role(const clan& clan, uint32_t changer_uid, const std::string& target, CLAN_ROLE role);

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
    [[nodiscard]] async::task<internal_resp::WriteMail>
    send_mail(const character& ch, const std::string& to, const std::string& title, const std::string& contents);

    /**
     * @brief      Gets a list of mail messages for a character.
     *
     * @param[in]  ch      The character to get mail list for.
     * @param[in]  offset  The starting offset for pagination.
     * @param[in]  count   The maximum number of mails to retrieve.
     *
     * @return     An async task returning the mail list response.
     */
    [[nodiscard]] async::task<internal_resp::GetMailList> mail_list(const character& ch,
                                                                    uint16_t         offset,
                                                                    uint16_t         count);

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
    [[nodiscard]] async::task<void>
    write_bulletin(character& ch, uint16_t section, const std::string& title, const std::string& contents);

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
     * @brief      Handles character interaction with a mob (monster).
     *
     * @param      ch    The character performing the interaction.
     * @param      mob   The mob being interacted with.
     */
    void handle_click_mob(fb::game::character& ch, fb::game::mob& mob);

    /**
     * @brief      Handles character interaction with an NPC.
     *
     * @param      ch    The character performing the interaction.
     * @param      npc   The NPC being interacted with.
     */
    void handle_click_npc(fb::game::character& ch, fb::game::npc& npc);

public:
    /**
     * @brief      Handles player login request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The login request packet containing credentials.
     *
     * @return     True if login was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_login(fb::socket<fb::game::character>&, const fb_reqs::login&);

    /**
     * @brief      Handles player direction change packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The direction change packet.
     *
     * @return     True if direction change was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_direction(fb::socket<fb::game::character>&, const fb_reqs::direction&);

    /**
     * @brief      Handles player logout/exit request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The exit request packet.
     *
     * @return     True if logout was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_logout(fb::socket<fb::game::character>&, const fb_reqs::exit&);

    /**
     * @brief      Handles player movement request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The movement request packet containing destination.
     *
     * @return     True if movement was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_move(fb::socket<fb::game::character>&, const fb_reqs::move&);

    /**
     * @brief      Handles player movement update packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The movement update packet.
     *
     * @return     True if movement update was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_update_move(fb::socket<fb::game::character>&, const fb_reqs::update_move&);

    /**
     * @brief      Handles player attack action packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The attack request packet containing target information.
     *
     * @return     True if attack was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_attack(fb::socket<fb::game::character>&, const fb_reqs::attack&);

    /**
     * @brief      Handles player item loot request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The loot request packet containing item information.
     *
     * @return     True if loot was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_loot(fb::socket<fb::game::character>&, const fb_reqs::loot&);

    /**
     * @brief      Handles player emotion/gesture packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The emotion packet containing gesture type.
     *
     * @return     True if emotion was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_emotion(fb::socket<fb::game::character>&, const fb_reqs::emotion&);

    /**
     * @brief      Handles map update request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The map update request packet.
     *
     * @return     True if map update was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_update_map(fb::socket<fb::game::character>&, const fb_reqs::map_update&);

    /**
     * @brief      Handles screen update request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The screen update request packet.
     *
     * @return     True if screen update was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_update_screen(fb::socket<fb::game::character>&,
                                                         const fb_reqs::update_screen&);

    /**
     * @brief      Handles item activation request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The item activation packet containing item information.
     *
     * @return     True if item activation was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_active_item(fb::socket<fb::game::character>&, const fb_reqs::item_active&);

    /**
     * @brief      Handles item deactivation request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The item deactivation packet containing item information.
     *
     * @return     True if item deactivation was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_inactive_item(fb::socket<fb::game::character>&,
                                                         const fb_reqs::item_inactive&);

    /**
     * @brief      Handles item drop request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The item drop packet containing item and location information.
     *
     * @return     True if item drop was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_drop_item(fb::socket<fb::game::character>&, const fb_reqs::item_drop&);

    /**
     * @brief      Handles money drop request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The money drop packet containing amount and location.
     *
     * @return     True if money drop was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_drop_money(fb::socket<fb::game::character>&,
                                                      const fb_reqs::item_drop_money&);

    /**
     * @brief      Handles front object information request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The front info request packet.
     *
     * @return     True if front info was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_front_info(fb::socket<fb::game::character>&, const fb_reqs::front_info&);

    /**
     * @brief      Handles self character information request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The self info request packet.
     *
     * @return     True if self info was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_self_info(fb::socket<fb::game::character>&, const fb_reqs::self_info&);

    /**
     * @brief      Handles player option/settings change packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The option update packet containing new settings.
     *
     * @return     True if option change was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_option_changed(fb::socket<fb::game::character>&,
                                                          const fb_reqs::update_option&);

    /**
     * @brief      Handles object click request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The click packet containing target object information.
     *
     * @return     True if click was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_click_object(fb::socket<fb::game::character>&, const fb_reqs::click&);

    /**
     * @brief      Handles item information request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The item info request packet containing item identifier.
     *
     * @return     True if item info was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_item_info(fb::socket<fb::game::character>&, const fb_reqs::item_info&);

    /**
     * @brief      Handles item mixing/crafting request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The item mix packet containing recipe and materials.
     *
     * @return     True if item mixing was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_item_combine(fb::socket<fb::game::character>&, const fb_reqs::item_combine&);

    /**
     * @brief      Handles player trade request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The trade packet containing trade information.
     *
     * @return     True if trade was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_trade(fb::socket<fb::game::character>&, const fb_reqs::trade&);

    /**
     * @brief      Handles group/party management packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The group packet containing group operation details.
     *
     * @return     True if group operation was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_group(fb::socket<fb::game::character>&, const fb_reqs::group&);

    /**
     * @brief      Handles user list request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The user list request packet.
     *
     * @return     True if user list was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_user_list(fb::socket<fb::game::character>&, const fb_reqs::user_list&);

    /**
     * @brief      Handles chat message packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The chat packet containing message and type.
     *
     * @return     True if chat was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_chat(fb::socket<fb::game::character>&, const fb_reqs::chat&);

    /**
     * @brief      Handles bulletin system request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The bulletin packet containing bulletin operation details.
     *
     * @return     True if bulletin operation was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_bulletin(fb::socket<fb::game::character>&, const fb_reqs::bulletin&);

    /**
     * @brief      Handles item/equipment swap request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The swap packet containing item positions to exchange.
     *
     * @return     True if swap was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_swap(fb::socket<fb::game::character>&, const fb_reqs::swap&);

    /**
     * @brief      Handles NPC dialog interaction packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The dialog packet containing NPC interaction details.
     *
     * @return     True if dialog was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_dialog(fb::socket<fb::game::character>&, const fb_reqs::dialog&);

    /**
     * @brief      Handles item throwing request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The item throw packet containing target and item information.
     *
     * @return     True if item throw was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_throw_item(fb::socket<fb::game::character>&, const fb_reqs::item_throws&);

    /**
     * @brief      Handles spell casting request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The spell cast packet containing spell and target information.
     *
     * @return     True if spell cast was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_spell(fb::socket<fb::game::character>&, const fb_reqs::spell_cast&);

    /**
     * @brief      Handles door interaction request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The door packet containing door operation details.
     *
     * @return     True if door interaction was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_door(fb::socket<fb::game::character>&, const fb_reqs::door&);

    /**
     * @brief      Handles whisper message packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The whisper packet containing recipient and message.
     *
     * @return     True if whisper was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_whisper(fb::socket<fb::game::character>&, const fb_reqs::whisper&);

    /**
     * @brief      Handles world map request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The world map request packet.
     *
     * @return     True if world map was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_world(fb::socket<fb::game::character>&, const fb_reqs::map_world&);

    /**
     * @brief      Handles object miss/failure notification packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The miss packet containing failure information.
     *
     * @return     True if miss was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_object_miss(fb::socket<fb::game::character>&, const fb_reqs::miss&);

    /**
     * @brief      Handles item giving request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The give item packet containing recipient and item details.
     *
     * @return     True if item giving was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_give_item(fb::socket<fb::game::character>&, const fb_reqs::give_item&);

    /**
     * @brief      Handles money giving request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The give money packet containing recipient and amount.
     *
     * @return     True if money giving was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_give_money(fb::socket<fb::game::character>&, const fb_reqs::give_money&);

    /**
     * @brief      Handles mail/post system request packet.
     *
     * @param      socket  The client socket connection.
     * @param[in]  packet  The post packet containing mail operation details.
     *
     * @return     True if post operation was processed successfully, false otherwise.
     */
    [[nodiscard]] async::task<bool> handle_post(fb::socket<fb::game::character>&, const fb_reqs::post&);

public:
    /**
     * @brief      Handles mob AI actions and behavior updates.
     *
     * @param[in]  now   The current datetime for timing calculations.
     * @param[in]  id    The thread identifier for this processing server.
     *
     * @return     An async task that completes when mob actions are processed.
     */
    [[nodiscard]] async::task<void> handle_mob_action(const fb::model::datetime& now, std::thread::id id);

    /**
     * @brief      Handles mob respawning logic and timing.
     *
     * @param[in]  now   The current datetime for respawn timing.
     * @param[in]  id    The thread identifier for this processing server.
     *
     * @return     An async task that completes when mob respawning is processed.
     */
    [[nodiscard]] async::task<void> handle_mob_respawn(const fb::model::datetime& now, std::thread::id id);

    /**
     * @brief      Handles buff/debuff timer updates and expiration.
     *
     * @param[in]  now   The current datetime for buff timing calculations.
     * @param[in]  id    The thread identifier for this processing server.
     *
     * @return     An async task that completes when buff timers are processed.
     */
    [[nodiscard]] async::task<void> handle_buff_timer(const fb::model::datetime& now, std::thread::id id);

    /**
     * @brief      Handles equipment gear timer effects and durability.
     *
     * @param[in]  now   The current datetime for gear timing calculations.
     * @param[in]  id    The thread identifier for this processing server.
     *
     * @return     An async task that completes when gear timers are processed.
     */
    [[nodiscard]] async::task<void> handle_gear_timer(const fb::model::datetime& now, std::thread::id id);

    /**
     * @brief      Handles NPC soliloquy/speech timer events.
     *
     * @param[in]  now   The current datetime for speech timing.
     * @param[in]  id    The thread identifier for this processing server.
     *
     * @return     An async task that completes when soliloquy timers are processed.
     */
    [[nodiscard]] async::task<void> handle_soliloquy_timer(const fb::model::datetime& now, std::thread::id id);

    /**
     * @brief      Handles periodic character data saving operations.
     *
     * @param[in]  now   The current datetime for save timing.
     * @param[in]  id    The thread identifier for this processing server.
     *
     * @return     An async task that completes when save operations are finished.
     */
    [[nodiscard]] async::task<void> handle_save_timer(const fb::model::datetime& now, std::thread::id id);

    /**
     * @brief      Handles game time updates and time-based events.
     *
     * @return     An async task that completes when time processing is finished.
     */
    [[nodiscard]] async::task<void> handle_time();

    /**
     * @brief      Handles periodic heartbeat operations for server health monitoring.
     *
     * @return     An async task that completes when heartbeat processing is finished.
     */
    [[nodiscard]] async::task<void> handle_heart_beat();

    /**
     * @brief      Handles server announcement broadcasting to all connected clients.
     *
     * @return     An async task that completes when announcement processing is finished.
     */
    [[nodiscard]] async::task<void> handle_announce();

public:
    /**
     * @brief      Handles server shutdown notification from AMQP.
     *
     * @param[in]  response  The shutdown response containing shutdown details.
     *
     * @return     An async task that completes when shutdown handling is finished.
     */
    [[nodiscard]] async::task<void> handle_amqp_shutdown(const internal_resp::Shutdown& response);
    /**
     * @brief      Handles server-wide broadcast message from AMQP.
     *
     * @param[in]  response  The broadcast response containing the message and metadata.
     *
     * @return     An async task that completes when broadcast handling is finished.
     */
    [[nodiscard]] async::task<void> handle_amqp_Broadcast(const internal_resp::Broadcast& response);

    /**
     * @brief      Handles player kick-out notification from AMQP.
     *
     * @param[in]  response  The kick-out response containing player and reason information.
     *
     * @return     An async task that completes when kick-out handling is finished.
     */
    [[nodiscard]] async::task<void> handle_amqp_KickOut(const internal_resp::KickOut& response);

    /**
     * @brief      Handles whisper message from AMQP.
     *
     * @param[in]  response  The whisper response containing the message and sender info.
     *
     * @return     An async task that completes when whisper handling is finished.
     */
    [[nodiscard]] async::task<void> handle_amqp_Whisper(const internal_resp::Whisper& response);

    /**
     * @brief      Handles group entry notification from AMQP.
     *
     * @param[in]  response  The group entry response containing group information.
     *
     * @return     An async task that completes when group entry handling is finished.
     */
    [[nodiscard]] async::task<void> handle_amqp_EnterGroup(const internal_resp::EnterGroup& response);

    /**
     * @brief      Handles group leave notification from AMQP.
     *
     * @param[in]  response  The group leave response containing departure details.
     *
     * @return     An async task that completes when group leave handling is finished.
     */
    [[nodiscard]] async::task<void> handle_amqp_LeaveGroup(const internal_resp::LeaveGroup& response);

    /**
     * @brief      Handles group kick notification from AMQP.
     *
     * @param[in]  response  The group kick response containing the kicked character.
     *
     * @return     An async task that completes when group kick handling is finished.
     */
    [[nodiscard]] async::task<void> handle_amqp_KickGroup(const internal_resp::KickGroup& response);

    /**
     * @brief      Handles clan title change notification from AMQP.
     *
     * @param[in]  response  The clan title change response containing the new title.
     *
     * @return     An async task that completes when clan title change handling is finished.
     */
    [[nodiscard]] async::task<void> handle_amqp_SetClanTitle(const internal_resp::SetClanTitle& response);

    /**
     * @brief      Handles clan member join notification from AMQP.
     *
     * @param[in]  response  The clan join response containing member information.
     *
     * @return     An async task that completes when clan join handling is finished.
     */
    [[nodiscard]] async::task<void> handle_amqp_JoinClan(const internal_resp::JoinClan& response);

    /**
     * @brief      Handles clan member leave notification from AMQP.
     *
     * @param[in]  response  The clan leave response containing departure details.
     *
     * @return     An async task that completes when clan leave handling is finished.
     */
    [[nodiscard]] async::task<void> handle_amqp_LeaveClan(const internal_resp::LeaveClan& response);

    /**
     * @brief      Handles clan member kick notification from AMQP.
     *
     * @param[in]  response  The clan kick response containing the kicked character.
     *
     * @return     An async task that completes when clan kick handling is finished.
     */
    [[nodiscard]] async::task<void> handle_amqp_KickClan(const internal_resp::KickClan& response);

    /**
     * @brief      Handles clan position change notification from AMQP.
     *
     * @param[in]  response  The clan position change response containing the updated member information.
     *
     * @return     An async task that completes when clan position change handling is finished.
     */
    [[nodiscard]] async::task<void> handle_amqp_ChangeClanRole(const internal_resp::ChangeClanRole& response);

    /**
     * @brief      Handles clan broadcast message from AMQP.
     *
     * @param[in]  response  The clan broadcast response containing the message and clan info.
     *
     * @return     An async task that completes when clan broadcast handling is finished.
     */
    [[nodiscard]] async::task<void> handle_amqp_BroadcastClan(const internal_resp::BroadcastClan& response);

    /**
     * @brief      Handles mail write notification from AMQP.
     *
     * @param[in]  response  The mail write response containing delivery status.
     *
     * @return     An async task that completes when mail write handling is finished.
     */
    [[nodiscard]] async::task<void> handle_amqp_WriteMail(const internal_resp::WriteMail& response);

public:
    /**
     * @brief      Handles NPC interaction for selling items to NPCs.
     *
     * @param      ch       The character performing the sell action.
     * @param[in]  message  The player's input message containing sell command.
     * @param[in]  npcs     The list of NPCs that can handle sell transactions.
     *
     * @return     True if the sell interaction was handled successfully, false otherwise.
     */
    async::task<bool> npc_interaction_sell(character&                                         ch,
                                           const std::string&                                 message,
                                           const std::vector<std::shared_ptr<fb::game::npc>>& npcs);

    /**
     * @brief      Handles NPC interaction for buying items from NPCs.
     *
     * @param      ch       The character performing the buy action.
     * @param[in]  message  The player's input message containing buy command.
     * @param[in]  npcs     The list of NPCs that can handle buy transactions.
     *
     * @return     True if the buy interaction was handled successfully, false otherwise.
     */
    async::task<bool> npc_interaction_buy(character&                                         ch,
                                          const std::string&                                 message,
                                          const std::vector<std::shared_ptr<fb::game::npc>>& npcs);

    /**
     * @brief      Handles NPC interaction for repairing items.
     *
     * @param      ch       The character requesting item repair.
     * @param[in]  message  The player's input message containing repair command.
     * @param[in]  npcs     The list of NPCs that can handle repair services.
     *
     * @return     True if the repair interaction was handled successfully, false otherwise.
     */
    async::task<bool> npc_interaction_repair(character&                                         ch,
                                             const std::string&                                 message,
                                             const std::vector<std::shared_ptr<fb::game::npc>>& npcs);

    /**
     * @brief      Handles NPC interaction for depositing money.
     *
     * @param      ch       The character depositing money.
     * @param[in]  message  The player's input message containing deposit command and amount.
     * @param[in]  npcs     The list of NPCs that can handle banking services.
     *
     * @return     True if the deposit interaction was handled successfully, false otherwise.
     */
    async::task<bool> npc_interaction_deposit_money(character&                                         ch,
                                                    const std::string&                                 message,
                                                    const std::vector<std::shared_ptr<fb::game::npc>>& npcs);

    /**
     * @brief      Handles NPC interaction for withdrawing money.
     *
     * @param      ch       The character withdrawing money.
     * @param[in]  message  The player's input message containing withdraw command and amount.
     * @param[in]  npcs     The list of NPCs that can handle banking services.
     *
     * @return     True if the withdraw interaction was handled successfully, false otherwise.
     */
    async::task<bool> npc_interaction_withdraw_money(character&                                         ch,
                                                     const std::string&                                 message,
                                                     const std::vector<std::shared_ptr<fb::game::npc>>& npcs);

    /**
     * @brief      Handles NPC interaction for storing items in storage.
     *
     * @param      ch       The character storing items.
     * @param[in]  message  The player's input message containing store command.
     * @param[in]  npcs     The list of NPCs that can handle storage services.
     *
     * @return     True if the store interaction was handled successfully, false otherwise.
     */
    async::task<bool> npc_interaction_store_item(character&                                         ch,
                                                 const std::string&                                 message,
                                                 const std::vector<std::shared_ptr<fb::game::npc>>& npcs);

    /**
     * @brief      Handles NPC interaction for retrieving items from storage.
     *
     * @param      ch       The character retrieving items.
     * @param[in]  message  The player's input message containing retrieve command.
     * @param[in]  npcs     The list of NPCs that can handle storage services.
     *
     * @return     True if the retrieve interaction was handled successfully, false otherwise.
     */
    async::task<bool> npc_interaction_retrieve_item(character&                                         ch,
                                                    const std::string&                                 message,
                                                    const std::vector<std::shared_ptr<fb::game::npc>>& npcs);

    /**
     * @brief      Handles NPC interaction for displaying items available for sale.
     *
     * @param      ch       The character requesting the sell list.
     * @param[in]  message  The player's input message containing sell list command.
     * @param[in]  npcs     The list of NPCs that can provide sell lists.
     *
     * @return     True if the sell list interaction was handled successfully, false otherwise.
     */
    async::task<bool> npc_interaction_sell_list(character&                                         ch,
                                                const std::string&                                 message,
                                                const std::vector<std::shared_ptr<fb::game::npc>>& npcs);

    /**
     * @brief      Handles NPC interaction for displaying items available for purchase.
     *
     * @param      ch       The character requesting the buy list.
     * @param[in]  message  The player's input message containing buy list command.
     * @param[in]  npcs     The list of NPCs that can provide buy lists.
     *
     * @return     True if the buy list interaction was handled successfully, false otherwise.
     */
    async::task<bool> npc_interaction_buy_list(character&                                         ch,
                                               const std::string&                                 message,
                                               const std::vector<std::shared_ptr<fb::game::npc>>& npcs);

    /**
     * @brief      Handles NPC interaction for checking item sell prices.
     *
     * @param      ch       The character checking sell prices.
     * @param[in]  message  The player's input message containing price check command.
     * @param[in]  npcs     The list of NPCs that can provide price information.
     *
     * @return     True if the sell price interaction was handled successfully, false otherwise.
     */
    async::task<bool> npc_interaction_sell_price(character&                                         ch,
                                                 const std::string&                                 message,
                                                 const std::vector<std::shared_ptr<fb::game::npc>>& npcs);

    /**
     * @brief      Handles NPC interaction for checking item buy prices.
     *
     * @param      ch       The character checking buy prices.
     * @param[in]  message  The player's input message containing price check command.
     * @param[in]  npcs     The list of NPCs that can provide price information.
     *
     * @return     True if the buy price interaction was handled successfully, false otherwise.
     */
    async::task<bool> npc_interaction_buy_price(character&                                         ch,
                                                const std::string&                                 message,
                                                const std::vector<std::shared_ptr<fb::game::npc>>& npcs);

    /**
     * @brief      Handles NPC interaction for displaying deposited money balance.
     *
     * @param      ch       The character checking their balance.
     * @param[in]  message  The player's input message containing balance check command.
     * @param[in]  npcs     The list of NPCs that can handle banking services.
     *
     * @return     True if the balance check interaction was handled successfully, false otherwise.
     */
    async::task<bool> npc_interaction_show_deposited_money(character&                                         ch,
                                                           const std::string&                                 message,
                                                           const std::vector<std::shared_ptr<fb::game::npc>>& npcs);

    /**
     * @brief      Handles NPC interaction for renaming weapons.
     *
     * @param      ch       The character requesting weapon rename.
     * @param[in]  message  The player's input message containing rename command and new name.
     * @param[in]  npcs     The list of NPCs that can handle weapon renaming services.
     *
     * @return     True if the weapon rename interaction was handled successfully, false otherwise.
     */
    async::task<bool> npc_interaction_rename_weapon(character&                                         ch,
                                                    const std::string&                                 message,
                                                    const std::vector<std::shared_ptr<fb::game::npc>>& npcs);

    /**
     * @brief      Handles NPC interaction for displaying stored items list.
     *
     * @param      ch       The character requesting their stored items list.
     * @param[in]  message  The player's input message containing storage list command.
     * @param[in]  npcs     The list of NPCs that can handle storage services.
     *
     * @return     True if the storage list interaction was handled successfully, false otherwise.
     */
    async::task<bool> npc_interaction_store_item_list(character&                                         ch,
                                                      const std::string&                                 message,
                                                      const std::vector<std::shared_ptr<fb::game::npc>>& npcs);

    /**
     * @brief      Handles NPC interaction for checking stored item count.
     *
     * @param      ch       The character checking their storage count.
     * @param[in]  message  The player's input message containing storage count command.
     * @param[in]  npcs     The list of NPCs that can handle storage services.
     *
     * @return     True if the storage count interaction was handled successfully, false otherwise.
     */
    async::task<bool> npc_interaction_store_item_count(character&                                         ch,
                                                       const std::string&                                 message,
                                                       const std::vector<std::shared_ptr<fb::game::npc>>& npcs);

    /**
     * @brief      Handles NPC interaction for character revival.
     *
     * @param      ch       The character requesting revival.
     * @param[in]  message  The player's input message containing revive command.
     * @param[in]  npcs     The list of NPCs that can handle revival services.
     *
     * @return     True if the revive interaction was handled successfully, false otherwise.
     */
    async::task<bool> npc_interaction_revive(character&                                         ch,
                                             const std::string&                                 message,
                                             const std::vector<std::shared_ptr<fb::game::npc>>& npcs);

    /**
     * @brief      Handles NPC interaction for appreciation/gratitude expressions.
     *
     * @param      ch       The character expressing appreciation.
     * @param[in]  message  The player's input message containing appreciation command.
     * @param[in]  npcs     The list of NPCs that can respond to appreciation.
     *
     * @return     True if the appreciation interaction was handled successfully, false otherwise.
     */
    async::task<bool> npc_interaction_appreciate(character&                                         ch,
                                                 const std::string&                                 message,
                                                 const std::vector<std::shared_ptr<fb::game::npc>>& npcs);

    /**
     * @brief      General NPC interaction handler that processes player messages.
     *
     * @param      ch       The character interacting with NPCs.
     * @param[in]  message  The player's input message to process.
     * @param[in]  npcs     The list of NPCs available for interaction.
     *
     * @return     True if any NPC interaction was handled successfully, false otherwise.
     */
    async::task<bool> npc_interaction(character&                                         ch,
                                      const std::string&                                 message,
                                      const std::vector<std::shared_ptr<fb::game::npc>>& npcs);

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

struct server::builtin
{
    /**
     * @brief      Lua builtin function to log a message.
     *
     * @param      L   The Lua state containing log message arguments.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_log(lua_State* L);

    /**
     * @brief      Lua builtin function to seed the random number generator.
     *
     * @param      L   The Lua state containing function arguments.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_seed(lua_State* L);

    /**
     * @brief      Lua builtin function to pause execution for a specified time.
     *
     * @param      L   The Lua state containing sleep duration argument.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_sleep(lua_State* L);

    /**
     * @brief      Lua builtin function to get current Baram calendar time.
     *             Baram calendar starts from 1996-03-31 00:00:00,
     *             where 1 Baram day equals 3 real hours.
     *
     * @param      L   The Lua state containing no arguments.
     *
     * @return     Number of return values pushed to Lua stack (Baram calendar time).
     */
    static int builtin_baram_time(lua_State* L);

    /**
     * @brief      Lua builtin function to find a mob by name.
     *
     * @param      L   The Lua state containing mob name argument.
     *
     * @return     Number of return values pushed to Lua stack (mob object or nil).
     */
    static int builtin_name2mob(lua_State* L);

    /**
     * @brief      Lua builtin function to find a spell by name.
     *
     * @param      L   The Lua state containing spell name argument.
     *
     * @return     Number of return values pushed to Lua stack (spell object or nil).
     */
    static int builtin_name2spell(lua_State* L);

    /**
     * @brief      Lua builtin function to find an NPC by name.
     *
     * @param      L   The Lua state containing NPC name argument.
     *
     * @return     Number of return values pushed to Lua stack (NPC object or nil).
     */
    static int builtin_name2npc(lua_State* L);

    /**
     * @brief      Lua builtin function to find a map by name.
     *
     * @param      L   The Lua state containing map name argument.
     *
     * @return     Number of return values pushed to Lua stack (map object or nil).
     */
    static int builtin_name2map(lua_State* L);

    /**
     * @brief      Lua builtin function to find a character by name.
     *
     * @param      L   The Lua state containing character name argument.
     *
     * @return     Number of return values pushed to Lua stack (character object or nil).
     */
    static int builtin_name2ch(lua_State* L);

    /**
     * @brief      Lua builtin function to find an item by name.
     *
     * @param      L   The Lua state containing item name argument.
     *
     * @return     Number of return values pushed to Lua stack (item object or nil).
     */
    static int builtin_name2item(lua_State* L);

    /**
     * @brief      Lua builtin function to handle pursuit sell operations.
     *
     * @param      L   The Lua state containing pursuit sell arguments.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_pursuit_sell(lua_State* L);

    /**
     * @brief      Lua builtin function to get pursuit sell price information.
     *
     * @param      L   The Lua state containing price query arguments.
     *
     * @return     Number of return values pushed to Lua stack (price information).
     */
    static int builtin_pursuit_sell_price(lua_State* L);

    /**
     * @brief      Lua builtin function to get pursuit sell item name.
     *
     * @param      L   The Lua state containing item query arguments.
     *
     * @return     Number of return values pushed to Lua stack (item name).
     */
    static int builtin_pursuit_sell_name(lua_State* L);

    /**
     * @brief      Lua builtin function to handle pursuit buy operations.
     *
     * @param      L   The Lua state containing pursuit buy arguments.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_pursuit_buy(lua_State* L);

    /**
     * @brief      Lua builtin function to create or manage timers.
     *
     * @param      L   The Lua state containing timer configuration arguments.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_timer(lua_State* L);

    /**
     * @brief      Lua builtin function to get or set weather information.
     *
     * @param      L   The Lua state containing weather query/set arguments.
     *
     * @return     Number of return values pushed to Lua stack (weather data).
     */
    static int builtin_weather(lua_State* L);

    /**
     * @brief      Lua builtin function to get or set brightness/lighting.
     *
     * @param      L   The Lua state containing brightness arguments.
     *
     * @return     Number of return values pushed to Lua stack (brightness data).
     */
    static int builtin_bright(lua_State* L);

    /**
     * @brief      Lua builtin function to find objects with specific name patterns.
     *
     * @param      L   The Lua state containing name pattern arguments.
     *
     * @return     Number of return values pushed to Lua stack (matching objects).
     */
    static int builtin_name_with(lua_State* L);

    /**
     * @brief      Lua builtin function to validate Korean text input.
     *
     * @param      L   The Lua state containing text to validate.
     *
     * @return     Number of return values pushed to Lua stack (validation result).
     */
    static int builtin_assert_korean(lua_State* L);

    /**
     * @brief      Lua builtin function to handle CP949 encoding operations.
     *
     * @param      L   The Lua state containing encoding arguments.
     *
     * @return     Number of return values pushed to Lua stack (encoded text).
     */
    static int builtin_cp949(lua_State* L);

    /**
     * @brief      Lua builtin function to send broadcast messages.
     *
     * @param      L   The Lua state containing broadcast message arguments.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_broadcast(lua_State* L);

    /**
     * @brief      Lua builtin function to assert that a character is alive.
     *
     * @param      L   The Lua state containing character to check.
     *
     * @return     Number of return values pushed to Lua stack (assertion result).
     */
    static int builtin_assert_alive(lua_State* L);

    /**
     * @brief      Lua builtin function to output debug information.
     *
     * @param      L   The Lua state containing debug message arguments.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_debug(lua_State* L);

    /**
     * @brief      Lua builtin function to convert character name to class type.
     *
     * @param      L   The Lua state containing character name argument.
     *
     * @return     Number of return values pushed to Lua stack (class type).
     */
    static int builtin_name2class(lua_State* L);

    /**
     * @brief      Lua builtin function to convert class type to class name.
     *
     * @param      L   The Lua state containing class type argument.
     *
     * @return     Number of return values pushed to Lua stack (class name).
     */
    static int builtin_class2name(lua_State* L);

    /**
     * @brief      Lua builtin function to save character data.
     *
     * @param      L   The Lua state containing character to save.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_save(lua_State* L);

    /**
     * @brief      Lua builtin function to create an NPC instance.
     *
     * @param      L   The Lua state containing NPC creation arguments.
     *
     * @return     Number of return values pushed to Lua stack (created NPC).
     */
    static int builtin_mknpc(lua_State* L);

    /**
     * @brief      Lua builtin function to get available maps list.
     *
     * @param      L   The Lua state containing map query arguments.
     *
     * @return     Number of return values pushed to Lua stack (maps list).
     */
    static int builtin_maps(lua_State* L);

    /**
     * @brief      Lua builtin function to initiate server shutdown.
     *
     * @param      L   The Lua state containing shutdown parameters.
     *
     * @return     Number of return values pushed to Lua stack.
     */
    static int builtin_shutdown(lua_State* L);
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
