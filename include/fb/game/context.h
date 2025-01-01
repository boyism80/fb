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
#include <fb/game/shard.h>

using namespace fb::protocol::internal;
namespace fb_reqs       = fb::protocol::game::request;
namespace fb_resp       = fb::protocol::game::response;
namespace internal      = fb::protocol::internal;
namespace internal_reqs = fb::protocol::internal::request;
namespace internal_resp = fb::protocol::internal::response;

namespace fb { namespace game {

/**
 * @brief      This class describes a context.
 */
class context : public fb::acceptor<fb::game::character>, public fb::game::listener
{
public:
    LUA_PROTOTYPE

public:
    /**
     * @brief      This class describes a scope.
     */
    enum class scope
    {
        SELF,
        PIVOT,
        GROUP,
        MAP,
        WORLD
    };

public:
    /**
     * @brief      { struct_description }
     */
    struct command_config
    {
        using func_type = std::function<async::task<bool>(fb::game::character&, Json::Value&)>;

        func_type fn;
        bool      admin;
    };

public:
    using object_set         = std::unordered_map<const fb::game::object*, std::unique_ptr<fb::game::object>>;
    using transfer_param     = fb_reqs::login::transfer_param;
    using protocol_generator = std::function<std::unique_ptr<fb::protocol::header>(const fb::game::object&)>;
    using command_container  = std::unordered_map<std::string, command_config>;
    using npc_interaction_func =
        std::function<bool(character&, const std::string&, const std::vector<fb::game::npc*>&)>;

private:
    command_container                 _commands;
    datetime                          _time;
    std::unique_ptr<fb::amqp::socket> _amqp;
    std::unique_ptr<std::thread>      _amqp_thread;
    fb::game::shard                   _shard;
    std::vector<npc_interaction_func> _npc_interaction_funcs;

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
     * @param[in]  datetime  The datetime
     *
     * @return     { description_of_the_return_value }
     */
    std::string elapsed_message(const std::string& datetime);

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
     * @param      ch        The ch
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
     * @param      ch        The ch
     */
    void init_option(const fb::protocol::internal::Option& response, fb::game::character& ch);

    /**
     * @brief      Initializes the items.
     *
     * @param[in]  response  The response
     * @param      ch        The ch
     */
    void init_items(const std::vector<fb::protocol::internal::Item>& response, fb::game::character& ch);
    /**
     * @brief      Initializes the spells.
     *
     * @param[in]  response  The response
     * @param      ch        The ch
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
     * @param[in]  cmd    The command
     * @param      func   The function
     * @param[in]  admin  The admin
     *
     * @tparam     Func   { description }
     */
    template <typename Func>
    void command(const std::string& cmd, Func&& func, bool admin)
    {
        this->_commands.insert({
            cmd,
            command_config{.fn = std::bind(func, this, std::placeholders::_1, std::placeholders::_2), .admin = admin}
        });
    }

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
     * @param[in]  you      You
     * @param[in]  error    The error
     * @param      message  The message
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
     * @brief      Called on enter group.
     *
     * @param[in]  resp  The response
     */
    void on_enter_group(internal_resp::EnterGroup resp);

    /**
     * @brief      Called on leave group.
     *
     * @param[in]  response  The response
     */
    void on_leave_group(const internal_resp::LeaveGroup& resp);

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
    void on_clan_broadcast(const internal_resp::BroadcastGroup& resp);

    /**
     * @brief      Called when clan title changed.
     *
     * @param[in]  response  The response
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
     * @brief      Called on clan broadcast.
     *
     * @param[in]  resp  The response
     */
    void on_clan_broadcast(const internal_resp::BroadcastClan& resp);

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
    async::task<void> destroy(T& obj, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT)
    {
        if constexpr (std::is_same_v<T, fb::game::object>)
        {
            std::ignore = co_await obj.map(nullptr, point16_t{0, 0}, destroy_type);
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
     */
    void send(fb::game::object&           object,
              const fb::protocol::header& header,
              context::scope              scope,
              bool                        exclude_self = false,
              bool                        encrypt      = true);
    /**
     * @brief      { function_description }
     *
     * @param      object        The object
     * @param[in]  fn            The function
     * @param[in]  scope         The scope
     * @param[in]  exclude_self  Indicates if the self is excluded
     * @param[in]  encrypt       The encrypt
     */
    void send(fb::game::object&         object,
              const protocol_generator& fn,
              context::scope            scope,
              bool                      exclude_self = false,
              bool                      encrypt      = true);
    /**
     * @brief      { function_description }
     *
     * @param[in]  header   The header
     * @param[in]  map      The map
     * @param[in]  encrypt  The encrypt
     */
    void send(const fb::protocol::header& header, const fb::game::map& map, bool encrypt = true);
    /**
     * @brief      { function_description }
     *
     * @param[in]  header   The header
     * @param[in]  encrypt  The encrypt
     */
    void send(const fb::protocol::header& header, bool encrypt = true);
    /**
     * @brief      { function_description }
     *
     * @param      ch    The ch
     *
     * @return     { description_of_the_return_value }
     */
    async::task<void> save(fb::game::character& ch);

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
     */
    void amqp_thread();

    /**
     * @brief      Creates a group.
     *
     * @param      me      { parameter_description }
     * @param[in]  target  The target
     *
     * @return     { description_of_the_return_value }
     */
    async::task<bool> create_group(character& me, const std::string& target);

    /**
     * @brief      { function_description }
     *
     * @param[in]  clan     The clan
     * @param[in]  message  The message
     * @param[in]  type     The type
     *
     * @return     { description_of_the_return_value }
     */
    async::task<void> broadcast(const group& group, const std::string& message, MESSAGE_TYPE type);

    /**
     * @brief      Creates a clan.
     *
     * @param      me    { parameter_description }
     * @param[in]  name  The name
     *
     * @return     { description_of_the_return_value }
     */
    async::task<void> create_clan(character& me, const std::string& name);

    /**
     * @brief      { function_description }
     *
     * @param      me    { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    async::task<void> destroy_clan(character& me);

    /**
     * @brief      Sets the clan title.
     *
     * @param      clan   The clan
     * @param[in]  title  The title
     *
     * @return     { description_of_the_return_value }
     */
    async::task<void> set_clan_title(const clan& clan, std::string title);

    /**
     * @brief      { function_description }
     *
     * @param      clan  The clan
     * @param      ch    { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    async::task<void> join_clan_member(const clan& clan, character& ch);

    /**
     * @brief      { function_description }
     *
     * @param      clan  The clan
     * @param      ch    { parameter_description }
     * @param[in]  kick  The kick
     *
     * @return     { description_of_the_return_value }
     */
    async::task<void> leave_clan_member(const clan& clan, const std::string& name, bool kick);

    /**
     * @brief      { function_description }
     *
     * @param[in]  message  The message
     * @param[in]  type     The type
     *
     * @return     { description_of_the_return_value }
     */
    async::task<void> broadcast(const clan& clan, const std::string& message, MESSAGE_TYPE type);

protected:
    /**
     * @brief      { function_description }
     *
     * @param[in]  cmd   The command
     *
     * @return     { description_of_the_return_value }
     */
    bool decrypt_policy(uint8_t cmd) const final;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    async::task<void> handle_start() override final;
    /**
     * @brief      { function_description }
     *
     * @param      ch  The ch
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_connected(fb::socket<fb::game::character>& ch) override final;
    /**
     * @brief      { function_description }
     *
     * @param      ch  The ch
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
    // async::task<void>       handle_internal_connected() override final;

    // for heart-beat

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
     * @param      ch    The ch
     * @param      mob   The mob
     */
    void handle_click_mob(fb::game::character& ch, fb::game::mob& mob);
    /**
     * @brief      { function_description }
     *
     * @param      ch    The ch
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
    [[nodiscard]] async::task<bool> handle_drop_cash(fb::socket<fb::game::character>&, const fb_reqs::item_drop_cash&);
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
    // async::task<bool>       handle_dialog_1(fb::socket<fb::game::character>&,
    // const fb_reqs::dialog1&); async::task<bool>
    // handle_dialog_2(fb::socket<fb::game::character>&, const
    // fb_reqs::dialog2&);
    //
    // @param      <unnamed>  { parameter_description }
    // @param[in]  <unnamed>  { parameter_description }
    //
    // @return     { description_of_the_return_value }
    //
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

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  now   The now
     * @param[in]  id    The identifier
     */
    [[nodiscard]] async::task<void> handle_mob_action(const datetime& now, std::thread::id id);
    /**
     * @brief      { function_description }
     *
     * @param[in]  now   The now
     * @param[in]  id    The identifier
     */
    [[nodiscard]] async::task<void> handle_mob_respawn(const datetime& now, std::thread::id id);
    /**
     * @brief      { function_description }
     *
     * @param[in]  now   The now
     * @param[in]  id    The identifier
     */
    [[nodiscard]] async::task<void> handle_buff_timer(const datetime& now, std::thread::id id);
    /**
     * @brief      { function_description }
     *
     * @param[in]  now   The now
     * @param[in]  id    The identifier
     */
    [[nodiscard]] async::task<void> handle_save_timer(const datetime& now, std::thread::id id);
    /**
     * @brief      { function_description }
     *
     * @param[in]  now   The now
     * @param[in]  id    The identifier
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
     * @param      ch  The ch
     * @param[in]  message  The message
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command(fb::game::character& ch, const std::string& message);

public:
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_map(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_sound(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_action(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_weather(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_bright(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_timer(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_effect(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_disguise(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_undisguise(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_mob(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_class(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_level(fb::game::character& ch, Json::Value& parameters);

    /**
     * @brief      { function_description }
     *
     * @param      ch          { parameter_description }
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_hp(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch          { parameter_description }
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_mp(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_spell(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_item(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_world(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_script(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_hair(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_hair_color(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_armor_color(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_exit(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_tile(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_save(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_mapobj(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_randmap(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_npc(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_durability(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_concurrency(fb::game::character& ch, Json::Value& parameters);
    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_command_sleep(fb::game::character& ch, Json::Value& parameters);

    /**
     * @brief      { function_description }
     *
     * @param      ch     The ch
     * @param      parameters  The parameters
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> handle_map_tile(character& ch, Json::Value& parameters);

    [[nodiscard]] async::task<bool> handle_mail(character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_mail_count(character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_mail_read(character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_ad(character& ch, Json::Value& parameters);
    [[nodiscard]] async::task<bool> handle_web(character& ch, Json::Value& parameters);

public:
    /**
     * @brief      { function_description }
     *
     * @param      response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_Pong(const internal_resp::Pong& response);
    /**
     * @brief      { function_description }
     *
     * @param      response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_KickOut(const internal_resp::KickOut& response);
    /**
     * @brief      { function_description }
     *
     * @param      response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_Whisper(const internal_resp::Whisper& response);
    /**
     * @brief      { function_description }
     *
     * @param      response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_EnterGroup(const internal_resp::EnterGroup& response);
    /**
     * @brief      { function_description }
     *
     * @param      response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_LeaveGroup(const internal_resp::LeaveGroup& response);
    /**
     * @brief      { function_description }
     *
     * @param      response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_SetClanTitle(const internal_resp::SetClanTitle& response);
    /**
     * @brief      { function_description }
     *
     * @param      response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_JoinClan(const internal_resp::JoinClan& response);
    /**
     * @brief      { function_description }
     *
     * @param      response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_LeaveClan(const internal_resp::LeaveClan& response);
    /**
     * @brief      { function_description }
     *
     * @param      response  The response
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<void> handle_amqp_BroadcastClan(const internal_resp::BroadcastClan& response);

public:
    // listener : object
    //
    // @param      me    { parameter_description }
    //
    void on_create(fb::game::object& me) override final;
    /**
     * @brief      Called on destroy.
     *
     * @param      me    { parameter_description }
     */
    void on_destroy(fb::game::object& me) override final;
    /**
     * @brief      Called on chat.
     *
     * @param      me         { parameter_description }
     * @param[in]  message    The message
     * @param[in]  chat_type  The chat type
     * @param[in]  shout  The shout
     */
    void on_chat(fb::game::object&  me,
                 const std::string& message,
                 CHAT_TYPE          chat_type = CHAT_TYPE::NORMAL) override final;
    /**
     * @brief      Called on direction.
     *
     * @param      me    { parameter_description }
     */
    void on_direction(fb::game::object& me) override final;
    /**
     * @brief      Called on show.
     *
     * @param      me     { parameter_description }
     * @param[in]  light  The light
     */
    void on_show(fb::game::object& me, bool light) override final;
    /**
     * @brief      Called on show.
     *
     * @param      me     { parameter_description }
     * @param      you    You
     * @param[in]  light  The light
     */
    void on_show(fb::game::object& me, fb::game::object& you, bool light) override final;
    /**
     * @brief      Called on hide.
     *
     * @param      me            { parameter_description }
     * @param[in]  destroy_type  The destroy type
     */
    void on_hide(fb::game::object& me, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) override final;
    /**
     * @brief      Called on hide.
     *
     * @param      me            { parameter_description }
     * @param      you           You
     * @param[in]  destroy_type  The destroy type
     */
    void on_hide(fb::game::object& me,
                 fb::game::object& you,
                 DESTROY_TYPE      destroy_type = DESTROY_TYPE::DEFAULT) override final;
    /**
     * @brief      Called on move.
     *
     * @param      me      { parameter_description }
     * @param[in]  before  The before
     */
    void on_move(fb::game::object& me, const point16_t& before) override final;
    /**
     * @brief      Called on unbuff.
     *
     * @param      me    { parameter_description }
     * @param      buff  The buffer
     */
    void on_unbuff(fb::game::object& me, fb::game::buff& buff) override final;
    /**
     * @brief      Called when map changed.
     *
     * @param      me      { parameter_description }
     * @param      before  The before
     * @param      after   The after
     */
    void on_map_changed(fb::game::object& me, fb::game::map* before, fb::game::map* after) override final;

    /**
     * @brief      Called on hold.
     *
     * @param      me    { parameter_description }
     */
    void on_hold(fb::game::object& me) override final;

    // listener : life
    //
    // @param      me    { parameter_description }
    // @param      you   You
    //
    void on_attack(life& me) override final;
    /**
     * @brief      Called on die.
     *
     * @param      me    { parameter_description }
     * @param      you   You
     */
    void on_dead(life& me, object* you) override final;

    /**
     * @brief      Called on hp.
     *
     * @param      me        { parameter_description }
     * @param[in]  before    The before
     * @param[in]  current   The current
     * @param[in]  critical  The critical
     * @param      from      The from
     */
    void
    on_hp_changed(life& me, uint32_t before, uint32_t current, bool critical, fb::game::object* from) override final;
    /**
     * @brief      Called on mp.
     *
     * @param      me        { parameter_description }
     * @param[in]  before    The before
     * @param[in]  current   The current
     * @param[in]  critical  The critical
     * @param      from      The from
     */
    void
    on_mp_changed(life& me, uint32_t before, uint32_t current, bool critical, fb::game::object* from) override final;

    /**
     * @brief      Called on action.
     *
     * @param      me        { parameter_description }
     * @param[in]  action    The action
     * @param[in]  duration  The duration
     * @param[in]  sound     The sound
     */
    void on_action(character& me, ACTION action, DURATION duration, uint8_t sound) override final;
    /**
     * @brief      Called when updated.
     *
     * @param      me     { parameter_description }
     * @param[in]  level  The level
     */
    void on_update(character& me, STATE_LEVEL level) override final;
    /**
     * @brief      Called on notify.
     *
     * @param      me       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  type     The type
     */
    void on_message(character& me, const std::string& message, MESSAGE_TYPE type) override final;
    /**
     * @brief      Called on option.
     *
     * @param      me       { parameter_description }
     * @param[in]  option   The option
     * @param[in]  enabled  Indicates if enabled
     */
    void on_option(character& me, SETTING option, bool enabled) override final;
    /**
     * @brief      Called on level up.
     *
     * @param      me    { parameter_description }
     */
    void on_level_up(character& me) override final;
    /**
     * @brief      Called on transfer.
     *
     * @param      me        { parameter_description }
     * @param      map       The map
     * @param[in]  position  The position
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] async::task<bool> on_transfer(character&       me,
                                                fb::game::map&   map,
                                                const point16_t& position) override final;
    //
    // @brief      Called on item remove.
    //
    // @param      me     { parameter_description }
    // @param[in]  index  The index
    // @param[in]  attr   The attribute
    //
    void on_item_remove(character& me, uint8_t index, ITEM_DELETE_TYPE attr) override final;
    /**
     * @brief      Called on item update.
     *
     * @param      me     { parameter_description }
     * @param[in]  index  The index
     */
    void on_item_update(character& me, uint8_t index) override final;
    /**
     * @brief      Called on item swap.
     *
     * @param      me    { parameter_description }
     * @param[in]  src   The source
     * @param[in]  dst   The destination
     */
    void on_item_swap(character& me, uint8_t src, uint8_t dst) override final;
    /**
     * @brief      Called on equipment on.
     *
     * @param      me     { parameter_description }
     * @param      item   The item
     * @param[in]  parts  The parts
     */
    void on_equipment_on(character& me, item& item, EQUIPMENT_PARTS parts) override final;
    /**
     * @brief      Called on equipment off.
     *
     * @param      me     { parameter_description }
     * @param[in]  parts  The parts
     * @param[in]  index  The index
     */
    void on_equipment_off(character& me, EQUIPMENT_PARTS parts, uint8_t index) override final;
    /**
     * @brief      Called on item active.
     *
     * @param      me    { parameter_description }
     * @param      item  The item
     */
    void on_item_active(character& me, item& item) override final;
    /**
     * @brief      Called on item throws.
     *
     * @param      me    { parameter_description }
     * @param      item  The item
     * @param[in]  to    { parameter_description }
     */
    void on_item_throws(character& me, item& item, const point16_t& to) override final;

    //
    // @brief      Called on spell update.
    //
    // @param      me     { parameter_description }
    // @param[in]  index  The index
    //
    void on_spell_update(life& me, uint8_t index) override final;
    /**
     * @brief      Called on spell remove.
     *
     * @param      me     { parameter_description }
     * @param[in]  index  The index
     */
    void on_spell_remove(life& me, uint8_t index) override final;

    //
    // @brief      Called on trade begin.
    //
    // @param      me    { parameter_description }
    // @param      you   You
    //
    void on_trade_begin(character& me, character& you) override final;
    /**
     * @brief      Called on trade bundle.
     *
     * @param      me    { parameter_description }
     */
    void on_trade_bundle(character& me) override final;
    /**
     * @brief      Called on trade item.
     *
     * @param      me     { parameter_description }
     * @param      from   The from
     * @param[in]  index  The index
     */
    void on_trade_item(character& me, character& from, uint8_t index, const fb::game::item& item) override final;
    /**
     * @brief      Called on trade money.
     *
     * @param      me    { parameter_description }
     * @param      from  The from
     */
    void on_trade_money(character& me, character& you, uint32_t money) override final;
    /**
     * @brief      Called on trade cancel.
     *
     * @param      me    { parameter_description }
     * @param      you   You
     */
    void on_trade_cancel(character& me, character& you) override final;
    /**
     * @brief      Called on trade lock.
     *
     * @param      me    { parameter_description }
     * @param[in]  mine  The mine
     */
    void on_trade_lock(character& me, character& you) override final;
    /**
     * @brief      Called when trade failed.
     *
     * @param      me    { parameter_description }
     */
    void on_trade_failed(character& me, character& you) override final;
    /**
     * @brief      Called on trade success.
     *
     * @param      me    { parameter_description }
     */
    void on_trade_success(character& me, character& you) override final;

    /**
     * @brief      Called on dialog.
     *
     * @param      me           { parameter_description }
     * @param[in]  object       The object
     * @param[in]  message      The message
     * @param[in]  button_prev  The button previous
     * @param[in]  button_next  The button next
     * @param[in]  interaction  The interaction
     */
    void on_dialog(character&                    me,
                   const fb::model::object&      object,
                   const std::string&            message,
                   bool                          button_prev,
                   bool                          button_next,
                   fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) override final;
    /**
     * @brief      Called on dialog.
     *
     * @param      me           { parameter_description }
     * @param[in]  npc          The npc
     * @param[in]  message      The message
     * @param[in]  menus        The menus
     * @param[in]  interaction  The interaction
     */
    void on_dialog(character&                      me,
                   const fb::model::npc&           npc,
                   const std::string&              message,
                   const std::vector<std::string>& menus,
                   fb::game::dialog::interaction   interaction = fb::game::dialog::interaction::NORMAL) override final;
    /**
     * @brief      Called on dialog.
     *
     * @param      me           { parameter_description }
     * @param[in]  npc          The npc
     * @param[in]  message      The message
     * @param[in]  item_slots   The item slots
     * @param[in]  interaction  The interaction
     */
    void on_dialog(character&                    me,
                   const fb::model::npc&         npc,
                   const std::string&            message,
                   const std::vector<uint8_t>&   item_slots,
                   fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) override final;
    /**
     * @brief      Called on dialog.
     *
     * @param      me           { parameter_description }
     * @param[in]  npc          The npc
     * @param[in]  message      The message
     * @param[in]  pairs        The pairs
     * @param[in]  pursuit      The pursuit
     * @param[in]  interaction  The interaction
     */
    void on_dialog(character&                          me,
                   const fb::model::npc&               npc,
                   const std::string&                  message,
                   const fb::game::dialog::item_pairs& pairs,
                   uint16_t                            pursuit = 0xFFFF,
                   fb::game::dialog::interaction interaction   = fb::game::dialog::interaction::NORMAL) override final;
    /**
     * @brief      Called on dialog.
     *
     * @param      me           { parameter_description }
     * @param[in]  npc          The npc
     * @param[in]  message      The message
     * @param[in]  interaction  The interaction
     */
    void on_dialog(character&                    me,
                   const fb::model::npc&         npc,
                   const std::string&            message,
                   fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) override final;
    /**
     * @brief      Called on dialog.
     *
     * @param      me           { parameter_description }
     * @param[in]  npc          The npc
     * @param[in]  message      The message
     * @param[in]  top          The top
     * @param[in]  bottom       The bottom
     * @param[in]  maxlen       The maxlen
     * @param[in]  prev         The previous
     * @param[in]  interaction  The interaction
     */
    void on_dialog(character&                    me,
                   const fb::model::npc&         npc,
                   const std::string&            message,
                   const std::string&            top,
                   const std::string&            bottom,
                   int                           maxlen      = 0xFF,
                   bool                          prev        = false,
                   fb::game::dialog::interaction interaction = fb::game::dialog::interaction::NORMAL) override final;

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
    bool npc_interaction_sell(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs);
    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool npc_interaction_buy(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs);
    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool npc_interaction_repair(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs);
    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool npc_interaction_deposit_money(character&                         ch,
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
    bool npc_interaction_withdraw_money(character&                         ch,
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
    bool npc_interaction_deposit_item(character&                         ch,
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
    bool npc_interaction_withdraw_item(character&                         ch,
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
    bool npc_interaction_sell_list(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs);
    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool npc_interaction_buy_list(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs);
    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool npc_interaction_sell_price(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs);
    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool npc_interaction_buy_price(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs);
    /**
     * @brief      { function_description }
     *
     * @param      ch       { parameter_description }
     * @param[in]  message  The message
     * @param[in]  npcs     The npcs
     *
     * @return     { description_of_the_return_value }
     */
    bool npc_interaction_show_deposited_money(character&                         ch,
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
    bool npc_interaction_rename_weapon(character&                         ch,
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
    bool npc_interaction_hold_item_list(character&                         ch,
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
    bool npc_interaction_hold_item_count(character&                         ch,
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
    bool npc_interaction(character& ch, const std::string& message, const std::vector<fb::game::npc*>& npcs);

public:
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_seed(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_sleep(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_name2mob(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_name2npc(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_name2map(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_name2item(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_pursuit_sell(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_pursuit_buy(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_sell_price(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_buy_price(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_timer(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_weather(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_name_with(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_assert_korean(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_cp949(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_broadcast(lua_State* lua);
};

}} // namespace fb::game

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
