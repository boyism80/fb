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
#include <fb/game/shard.h>
#include <fb/redis.h>

using namespace fb::protocol::internal;
namespace fb_reqs       = fb::protocol::game::request;
namespace fb_resp       = fb::protocol::game::response;
namespace internal      = fb::protocol::internal;
namespace internal_reqs = fb::protocol::internal::request;
namespace internal_resp = fb::protocol::internal::response;

namespace fb::game {

/**
 * @brief      This class describes a scope.
 */
enum class scope
{
    PIVOT,
    GROUP,
    MAP,
    WORLD
};

/**
 * @brief      This class describes a context.
 */
class context : public fb::acceptor<fb::game::character>
{
public:
    LUA_PROTOTYPE

public:
    using object_set         = std::unordered_map<const fb::game::object*, std::unique_ptr<fb::game::object>>;
    using transfer_param     = fb_reqs::login::transfer_param;
    using protocol_generator = std::function<std::unique_ptr<fb::protocol::header>(const fb::game::object&)>;
    using npc_interaction_func =
        std::function<async::task<bool>(character&, const std::string&, const std::vector<fb::game::npc*>&)>;

private:
    fb::model::datetime               _time;
    fb::hash<shard_params>            _shard;
    std::vector<npc_interaction_func> _npc_interaction_funcs;
    fb::redis                         _redis;

public:
    fb::game::listener_impl listener;

public:
    fb::model::model        model;
    fb::game::map_container maps;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  port     The port
     */
    context(boost::asio::io_context& context, uint16_t port);

    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    context(const context&) = delete;

    /**
     * @brief      Destroys the object.
     */
    ~context();

private:
    /**
     * @brief      { function_description }
     *
     * @param[in]  dt    { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    std::string elapsed_message(const std::string& dt);

    /**
     * @brief      { function_description }
     *
     * @param[in]  gid   The gid
     * @param[in]  fn    The function
     */
    void upsert_group_then(uint32_t gid, const std::function<void(shared_group_lock&)>& fn);

    /**
     * @brief      { function_description }
     *
     * @param[in]  gid      The gid
     * @param[in]  master   The master
     * @param[in]  members  The members
     * @param[in]  fn       The function
     */
    void upsert_group_then(uint32_t                                       gid,
                           const std::string&                             master,
                           const std::vector<std::string>&                members,
                           const std::function<void(shared_group_lock&)>& fn);

    /**
     * @brief      { function_description }
     *
     * @param      clan   The clan
     * @param      resp1  The response 1
     * @param[in]  resp2  The response 2
     */
    void update_clan(clan&                                                  clan,
                     fb::protocol::internal::Clan&                          resp1,
                     const std::vector<fb::protocol::internal::ClanMember>& resp2) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  id    The identifier
     * @param[in]  fn    The function
     */
    void upsert_clan_then(uint32_t id, const std::function<void(shared_clan_lock&)>& fn);

    /**
     * @brief      Initializes the ch.
     *
     * @param[in]  response  The response
     * @param      ch        { parameter_description }
     * @param[in]  group     The group
     * @param[in]  clan      The clan
     * @param[in]  transfer  The transfer
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> init_ch(const fb::protocol::internal::Character& response,
                                            fb::game::character&                     ch,
                                            std::optional<uint32_t>                  group,
                                            std::optional<uint32_t>                  clan,
                                            const std::optional<transfer_param>&     transfer);

    /**
     * @brief      Initializes the option.
     *
     * @param[in]  response  The response
     * @param      ch        { parameter_description }
     */
    void init_option(const fb::protocol::internal::Option& response, fb::game::character& ch);

    /**
     * @brief      Initializes the items.
     *
     * @param[in]  response  The response
     * @param      ch        { parameter_description }
     */
    void init_items(const std::vector<fb::protocol::internal::Item>& response, fb::game::character& ch);

    /**
     * @brief      Initializes the spells.
     *
     * @param[in]  response  The response
     * @param      ch        { parameter_description }
     */
    void init_spells(const std::vector<fb::protocol::internal::Spell>& response, fb::game::character& ch);

    /**
     * @brief      Initializes the traces.
     *
     * @param[in]  response  The response
     * @param      ch        { parameter_description }
     */
    void init_traces(const std::vector<fb::protocol::internal::Trace>& response, fb::game::character& ch);

private:
    /**
     * @brief      { function_description }
     *
     * @param      func  The function
     *
     * @tparam     Func  { description }
     */
    template <typename Func>
    void bind_npc_interaction(Func&& func)
    {
        auto c_fn = std::bind(func, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        this->_npc_interaction_funcs.push_back(c_fn);
    }

private:
    /**
     * @brief      { function_description }
     *
     * @param[in]  response  The response
     */
    void assert_whisper(const internal::response::Whisper& response) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  error  The error
     * @param[in]  actor  The actor
     */
    void assert_group(uint32_t error, const std::string& actor) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    void assert_clan(uint32_t) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    void assert_mail(uint32_t) const;

    /**
     * @brief      Called on enter group.
     *
     * @param[in]  resp  The response
     */
    void on_enter_group(internal_resp::EnterGroup resp);

    /**
     * @brief      Called on leave group.
     *
     * @param[in]  resp  The response
     */
    void on_leave_group(const internal_resp::LeaveGroup& resp);

    /**
     * @brief      Called on broadcast.
     *
     * @param[in]  resp  The response
     */
    void on_broadcast(const internal_resp::Broadcast& resp);

    /**
     * @brief      Called on group broadcast.
     *
     * @param[in]  resp  The response
     */
    void on_group_broadcast(const internal_resp::BroadcastGroup& resp);

    /**
     * @brief      Called on clan broadcast.
     *
     * @param[in]  resp  The response
     */
    void on_clan_broadcast(const internal_resp::BroadcastClan& resp);

    /**
     * @brief      Called when clan title changed.
     *
     * @param[in]  resp  The response
     */
    void on_clan_title_changed(const internal_resp::SetClanTitle& resp);

    /**
     * @brief      Called on clan join member.
     *
     * @param[in]  resp  The response
     */
    void on_clan_join_member(const internal_resp::JoinClan& resp);

    /**
     * @brief      Called on clan leave member.
     *
     * @param[in]  resp  The response
     */
    void on_clan_leave_member(const internal_resp::LeaveClan& resp);

    /**
     * @brief      Called on write mail.
     *
     * @param[in]  resp  The response
     */
    void on_write_mail(const internal_resp::WriteMail& resp);

    /**
     * @brief      Called on whisper.
     *
     * @param[in]  resp  The response
     */
    void on_whisper(const internal_resp::Whisper& resp);

public:
    /**
     * @brief      { function_description }
     *
     * @param      args  The arguments
     *
     * @tparam     T     { description }
     * @tparam     Args  { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <typename T, typename... Args>
    T* make(Args&&... args)
    {
        return new T(*this, std::forward<Args>(args)...);
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param      obj           The object
     * @param[in]  destroy_type  The destroy type
     *
     * @tparam     T             { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <typename T>
    [[nodiscard]] async::task<void> destroy(T& obj, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT)
    {
        if constexpr (std::is_same_v<T, fb::game::object>)
        {
            std::ignore = co_await obj.map(nullptr, fb::model::point16_t{0, 0}, destroy_type);
        }
        delete &obj;
        co_return;
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param      object        The object
     * @param[in]  header        The header
     * @param[in]  scope         The scope
     * @param[in]  exclude_self  Indicates if the self is excluded
     * @param[in]  encrypt       The encrypt
     *
     * @return     { description_of_the_return_value }
     */
    async::task<void> send(fb::game::object&           object,
                           const fb::protocol::header& header,
                           fb::game::context::scope    scope,
                           bool                        exclude_self = false,
                           bool                        encrypt      = true);

    /**
     * @brief      { function_description }
     *
     * @param      ch    { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> save(fb::game::character& ch);

    /**
     * @brief      { function_description }
     *
     * @param[in]  name  The name
     * @param[in]  fn    The function
     * @param[in]  miss  The miss
     */
    void foreach_ch(const std::string&                                  name,
                    const std::function<void(fb::game::character&)>&    fn,
                    const std::function<void(const std::string& name)>& miss);

    /**
     * @brief      { function_description }
     *
     * @param[in]  name  The name
     * @param[in]  fn    The function
     */
    void foreach_ch(const std::string& name, const std::function<void(fb::game::character&)>& fn);

    /**
     * @brief      { function_description }
     *
     * @param[in]  names  The names
     * @param[in]  fn     The function
     * @param[in]  miss   The miss
     */
    void foreach_ch(const std::vector<std::string>&                     names,
                    const std::function<void(fb::game::character&)>&    fn,
                    const std::function<void(const std::string& name)>& miss);

    /**
     * @brief      { function_description }
     *
     * @param[in]  names  The names
     * @param[in]  fn     The function
     */
    void foreach_ch(const std::vector<std::string>& names, const std::function<void(fb::game::character&)>& fn);

    /**
     * @brief      { function_description }
     *
     * @param[in]  clan  The clan
     * @param[in]  fn    The function
     */
    void foreach_ch(const clan& clan, const std::function<void(fb::game::character&)>& fn);

    /**
     * @brief      { function_description }
     *
     * @param[in]  group  The group
     * @param[in]  fn     The function
     */
    void foreach_ch(const group& group, const std::function<void(fb::game::character&)>& fn);

    /**
     * @brief      { function_description }
     *
     * @param[in]  fn    The function
     */
    void foreach_ch(const std::function<void(fb::game::character&)>& fn);

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  socket  The socket
     *
     * @return     { description_of_the_return_value }
     */
    virtual uint32_t thread_id(const fb::socket<fb::game::character>& socket) const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  map   The map
     *
     * @return     { description_of_the_return_value }
     */
    fb::thread* thread(const fb::game::map& map);

    /**
     * @brief      { function_description }
     *
     * @param[in]  message         The message
     * @param[in]  type            The type
     * @param[in]  broadcast_type  The broadcast type
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> broadcast(const std::string& message,
                                              MESSAGE_TYPE       type,
                                              BROADCAST_TYPE     broadcast_type);

    /**
     * @brief      Creates a group.
     *
     * @param      me      { parameter_description }
     * @param[in]  target  The target
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> create_group(character& me, const std::string& target);

    /**
     * @brief      { function_description }
     *
     * @param[in]  group    The group
     * @param[in]  message  The message
     * @param[in]  type     The type
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> broadcast(const group& group, const std::string& message, MESSAGE_TYPE type);

    /**
     * @brief      Creates a clan.
     *
     * @param      me    { parameter_description }
     * @param[in]  name  The name
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> create_clan(character& me, const std::string& name);

    /**
     * @brief      { function_description }
     *
     * @param      me    { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> destroy_clan(character& me);

    /**
     * @brief      Sets the clan title.
     *
     * @param[in]  clan   The clan
     * @param[in]  title  The title
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> set_clan_title(const clan& clan, std::string title);

    /**
     * @brief      { function_description }
     *
     * @param[in]  clan  The clan
     * @param      ch    { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> join_clan_member(const clan& clan, character& ch);

    /**
     * @brief      { function_description }
     *
     * @param[in]  clan  The clan
     * @param[in]  name  The name
     * @param[in]  kick  The kick
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> leave_clan_member(const clan& clan, const std::string& name, bool kick);

    /**
     * @brief      { function_description }
     *
     * @param[in]  clan     The clan
     * @param[in]  message  The message
     * @param[in]  type     The type
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> broadcast(const clan& clan, const std::string& message, MESSAGE_TYPE type);

    /**
     * @brief      Writes a mail.
     *
     * @param[in]  ch        { parameter_description }
     * @param[in]  to        { parameter_description }
     * @param[in]  title     The title
     * @param[in]  contents  The contents
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<internal_resp::WriteMail>
    send_mail(const character& ch, const std::string& to, const std::string& title, const std::string& contents);

    /**
     * @brief      { function_description }
     *
     * @param[in]  ch      { parameter_description }
     * @param[in]  offset  The offset
     * @param[in]  count   The count
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<internal_resp::GetMailList> mail_list(const character& ch,
                                                                    uint16_t         offset,
                                                                    uint16_t         count);

    /**
     * @brief      Reads a mail.
     *
     * @param      ch    { parameter_description }
     * @param[in]  id    The identifier
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<internal_resp::GetMail> read_mail(character& ch, uint16_t id);

    /**
     * @brief      { function_description }
     *
     * @param      ch    { parameter_description }
     * @param[in]  id    The identifier
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<internal_resp::DeleteMail> delete_mail(character& ch, uint16_t id);

    /**
     * @brief      { function_description }
     *
     * @param[in]  section  The section identifier
     * @param[in]  offset   The offset
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<std::list<board::article>> board_list(uint16_t section, uint16_t offset);

    /**
     * @brief      Reads a board.
     *
     * @param[in]  section  The section
     * @param[in]  id       The identifier
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<board::article> read_board(uint16_t section, uint16_t id);

    /**
     * @brief      Writes a board.
     *
     * @param      ch        { parameter_description }
     * @param[in]  section   The section
     * @param[in]  title     The title
     * @param[in]  contents  The contents
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void>
    write_board(character& ch, uint16_t section, const std::string& title, const std::string& contents);

    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  section  The section
     * @param[in]  id       The identifier
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> delete_board(character& ch, uint16_t section, uint16_t id);

    /**
     * @brief      { function_description }
     *
     * @param      from     The from
     * @param[in]  to       { parameter_description }
     * @param[in]  message  The message
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> whisper(character& from, std::string to, std::string message);

protected:
    /**
     * @brief      { function_description }
     *
     * @param[in]  cmd   The command
     *
     * @return     { description_of_the_return_value }
     */
    bool decrypt_policy(uint8_t cmd) const override final;

    /**
     * @brief      { function_description }
     *
     * @param      ch    { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool assert_tps(const fb::socket<fb::game::character>& socket) const override final;

    /**
     * @brief      { function_description }
     *
     * @param      amqp  The amqp
     */
    void handle_declare_amqp_queue(fb::amqp::socket& amqp) override final;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_start() override final;

    /**
     * @brief      { function_description }
     *
     * @param      ch    { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_connected(fb::socket<fb::game::character>& ch) override final;

    /**
     * @brief      { function_description }
     *
     * @param      ch    { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_disconnected(fb::socket<fb::game::character>& ch) override final;

    /**
     * @brief      { function_description }
     *
     * @param      socket  The socket
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::character* handle_accepted(fb::socket<fb::game::character>& socket) override final;

protected:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t id() const
    {
        return fb::config<uint8_t>("id");
    }
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    Service service() const
    {
        return Service::Game;
    }

public:
    /**
     * @brief      { function_description }
     *
     * @param      ch    { parameter_description }
     * @param      mob   The mob
     */
    void handle_click_mob(fb::game::character& ch, fb::game::mob& mob);

    /**
     * @brief      { function_description }
     *
     * @param      ch    { parameter_description }
     * @param      npc   The npc
     */
    void handle_click_npc(fb::game::character& ch, fb::game::npc& npc);

public:
    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_login(fb::socket<fb::game::character>&, const fb_reqs::login&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_direction(fb::socket<fb::game::character>&, const fb_reqs::direction&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_logout(fb::socket<fb::game::character>&, const fb_reqs::exit&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_move(fb::socket<fb::game::character>&, const fb_reqs::move&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_update_move(fb::socket<fb::game::character>&, const fb_reqs::update_move&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_attack(fb::socket<fb::game::character>&, const fb_reqs::attack&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_pickup(fb::socket<fb::game::character>&, const fb_reqs::pick_up&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_emotion(fb::socket<fb::game::character>&, const fb_reqs::emotion&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_update_map(fb::socket<fb::game::character>&, const fb_reqs::map_update&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_update_screen(fb::socket<fb::game::character>&,
                                                         const fb_reqs::update_screen&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_active_item(fb::socket<fb::game::character>&, const fb_reqs::item_active&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_inactive_item(fb::socket<fb::game::character>&,
                                                         const fb_reqs::item_inactive&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_drop_item(fb::socket<fb::game::character>&, const fb_reqs::item_drop&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_drop_money(fb::socket<fb::game::character>&,
                                                      const fb_reqs::item_drop_money&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_front_info(fb::socket<fb::game::character>&, const fb_reqs::front_info&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_self_info(fb::socket<fb::game::character>&, const fb_reqs::self_info&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_option_changed(fb::socket<fb::game::character>&,
                                                          const fb_reqs::update_option&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_click_object(fb::socket<fb::game::character>&, const fb_reqs::click&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_item_info(fb::socket<fb::game::character>&, const fb_reqs::item_info&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_itemmix(fb::socket<fb::game::character>&, const fb_reqs::item_mix&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_trade(fb::socket<fb::game::character>&, const fb_reqs::trade&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_group(fb::socket<fb::game::character>&, const fb_reqs::group&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_user_list(fb::socket<fb::game::character>&, const fb_reqs::user_list&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_chat(fb::socket<fb::game::character>&, const fb_reqs::chat&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_board(fb::socket<fb::game::character>&, const fb_reqs::board&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_swap(fb::socket<fb::game::character>&, const fb_reqs::swap&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_dialog(fb::socket<fb::game::character>&, const fb_reqs::dialog&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_throw_item(fb::socket<fb::game::character>&, const fb_reqs::item_throws&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_spell(fb::socket<fb::game::character>&, const fb_reqs::spell_cast&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_door(fb::socket<fb::game::character>&, const fb_reqs::door&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_whisper(fb::socket<fb::game::character>&, const fb_reqs::whisper&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_world(fb::socket<fb::game::character>&, const fb_reqs::map_world&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_object_miss(fb::socket<fb::game::character>&, const fb_reqs::miss&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_give_item(fb::socket<fb::game::character>&, const fb_reqs::give_item&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_give_money(fb::socket<fb::game::character>&, const fb_reqs::give_money&);

    /**
     * @brief      { function_description }
     *
     * @param      <unnamed>  { parameter_description }
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_post(fb::socket<fb::game::character>&, const fb_reqs::post&);

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  now   The now
     * @param[in]  id    The identifier
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_mob_action(const fb::model::datetime& now, std::thread::id id);

    /**
     * @brief      { function_description }
     *
     * @param[in]  now   The now
     * @param[in]  id    The identifier
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_mob_respawn(const fb::model::datetime& now, std::thread::id id);

    /**
     * @brief      { function_description }
     *
     * @param[in]  now   The now
     * @param[in]  id    The identifier
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_buff_timer(const fb::model::datetime& now, std::thread::id id);

    /**
     * @brief      { function_description }
     *
     * @param[in]  now   The now
     * @param[in]  id    The identifier
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_gear_timer(const fb::model::datetime& now, std::thread::id id);

    /**
     * @brief      { function_description }
     *
     * @param[in]  now   The now
     * @param[in]  id    The identifier
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_soliloquy_timer(const fb::model::datetime& now, std::thread::id id);

    /**
     * @brief      { function_description }
     *
     * @param[in]  now   The now
     * @param[in]  id    The identifier
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_save_timer(const fb::model::datetime& now, std::thread::id id);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_time();

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_heart_beat();

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_announce();

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_shutdown(const internal_resp::Shutdown& response);
    /**
     * @brief      { function_description }
     *
     * @param[in]  response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_Broadcast(const internal_resp::Broadcast& response);

    /**
     * @brief      { function_description }
     *
     * @param[in]  response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_KickOut(const internal_resp::KickOut& response);

    /**
     * @brief      { function_description }
     *
     * @param[in]  response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_Whisper(const internal_resp::Whisper& response);

    /**
     * @brief      { function_description }
     *
     * @param[in]  response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_EnterGroup(const internal_resp::EnterGroup& response);

    /**
     * @brief      { function_description }
     *
     * @param[in]  response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_LeaveGroup(const internal_resp::LeaveGroup& response);

    /**
     * @brief      { function_description }
     *
     * @param[in]  response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_SetClanTitle(const internal_resp::SetClanTitle& response);

    /**
     * @brief      { function_description }
     *
     * @param[in]  response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_JoinClan(const internal_resp::JoinClan& response);

    /**
     * @brief      { function_description }
     *
     * @param[in]  response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_LeaveClan(const internal_resp::LeaveClan& response);

    /**
     * @brief      { function_description }
     *
     * @param[in]  response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_BroadcastClan(const internal_resp::BroadcastClan& response);

    /**
     * @brief      { function_description }
     *
     * @param[in]  response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_WriteMail(const internal_resp::WriteMail& response);

public:
    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> npc_interaction_sell(character&                         ch,
                                           const std::string&                 message,
                                           const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> npc_interaction_buy(character&                         ch,
                                          const std::string&                 message,
                                          const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> npc_interaction_repair(character&                         ch,
                                             const std::string&                 message,
                                             const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> npc_interaction_deposit_money(character&                         ch,
                                                    const std::string&                 message,
                                                    const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> npc_interaction_withdraw_money(character&                         ch,
                                                     const std::string&                 message,
                                                     const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> npc_interaction_store_item(character&                         ch,
                                                 const std::string&                 message,
                                                 const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> npc_interaction_retrieve_item(character&                         ch,
                                                    const std::string&                 message,
                                                    const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> npc_interaction_sell_list(character&                         ch,
                                                const std::string&                 message,
                                                const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> npc_interaction_buy_list(character&                         ch,
                                               const std::string&                 message,
                                               const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> npc_interaction_sell_price(character&                         ch,
                                                 const std::string&                 message,
                                                 const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> npc_interaction_buy_price(character&                         ch,
                                                const std::string&                 message,
                                                const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> npc_interaction_show_deposited_money(character&                         ch,
                                                           const std::string&                 message,
                                                           const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> npc_interaction_rename_weapon(character&                         ch,
                                                    const std::string&                 message,
                                                    const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> npc_interaction_store_item_list(character&                         ch,
                                                      const std::string&                 message,
                                                      const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> npc_interaction_store_item_count(character&                         ch,
                                                       const std::string&                 message,
                                                       const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> npc_interaction_revive(character&                         ch,
                                             const std::string&                 message,
                                             const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> npc_interaction_appreciate(character&                         ch,
                                                 const std::string&                 message,
                                                 const std::vector<fb::game::npc*>& npcs);

    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> npc_interaction(character&                         ch,
                                      const std::string&                 message,
                                      const std::vector<fb::game::npc*>& npcs);

public:
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_seed(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_sleep(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_name2mob(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_name2spell(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_name2npc(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_name2map(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_name2ch(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_name2item(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_pursuit_sell(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_pursuit_sell_price(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_pursuit_sell_name(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_pursuit_buy(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_timer(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_weather(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_bright(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_name_with(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_assert_korean(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_cp949(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_broadcast(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_assert_alive(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_debug(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_name2class(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_class2name(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_save(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_mknpc(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_maps(lua_State* L);

    /**
     * @brief      { function_description }
     *
     * @param      L     { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_shutdown(lua_State* L);
};

} // namespace fb::game

/**
 * @brief      { function_description }
 *
 * @param      context  The context
 * @param      args     The arguments
 *
 * @tparam     T        { description }
 * @tparam     Args     { description }
 *
 * @return     { description_of_the_return_value }
 */
template <typename T, typename... Args>
T* fb::model::object::make(fb::game::context& context, Args&&... args) const
{
    auto& model = static_cast<const typename T::model_type&>(*this);
    return context.template make<T>(model, std::forward<Args>(args)...);
}

#endif // !__FB_GAME_H__
