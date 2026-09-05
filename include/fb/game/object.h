#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <fb/lua.h>
#include <fb/stream.h>
#include <fb/protocol/header.h>
#include <fb/thread_switchable.h>
#include <fb/model/model.h>
#include <fb/game/spell.h>
#include <random.h>
#include <async/task.h>
#include <functional>
#include <optional>
#include <shared_mutex>
#include <string_view>

namespace fb::game {

using namespace fb::model::enum_value;

enum class OBJECT_CURSOR : uint8_t
{
    ATTACK       = 0,
    INSPECT      = 11,
    INTERACT     = 12,
    INTERACT_ALT = 14,
    ITEM         = 15
};

using map_callback = std::function<async::task<bool>()>;

struct map_options
{
    DESTROY_TYPE destroy_type       = DESTROY_TYPE::DEFAULT;
    bool         notify             = true;
    bool         skip_instance_rule = false;
    map_callback callback           = {};
};

class server;
class map;
class character;
class buffs;
class objects;
class items;
class sector;
class appearance;

class object : public fb::thread_switchable
{
public:
    struct listener_t;
    struct initial_params;

    class container;

public:
    LUA_PROTOTYPE

    using map_ptr        = std::shared_ptr<fb::game::map>;
    using object_ptr     = std::shared_ptr<object>;
    using appearance_ptr = std::shared_ptr<fb::game::appearance>;

    friend fb::game::buffs;
    friend fb::game::items;

private:
    fb::game::sector*         _sector = nullptr;
    mutable std::shared_mutex _map_lock;

protected:
    uint32_t             _oid       = 0;
    uint32_t             _model_id  = 0;
    fb::model::point16_t _position  = fb::model::point16_t(0, 0);
    DIRECTION            _direction = DIRECTION::BOTTOM;
    map_ptr              _map       = nullptr;
    fb::thread*          _thread    = nullptr;

public:
    listener_t&       listener;
    fb::game::server& server;
    fb::game::buffs   buffs;

protected:
    // clang-format off
    object(fb::game::server& server, const fb::model::object& model, const initial_params& c);
    object(const object& right);
    // clang-format on

public:
    virtual ~object();

private:
    // clang-format off
    void                             update_sector();
    async::task<void>                invoke_map_character_hook(const fb::model::map& map_model, std::string_view hook);
    static bool                      sight(const fb::model::point16_t me, const fb::model::point16_t you, const map_ptr& map);
    static fb::model::area<uint16_t> sight_area(const fb::model::point16_t& position, const map_ptr& map);
    // clang-format on

public:
    virtual void on_init();

public:
    // clang-format off
    [[nodiscard]] virtual async::task<void> destroy(DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);
    virtual size_t                          send(const fb::stream& stream, bool encrypt = true, bool wrap = true);
    virtual size_t                          send(const fb::protocol::header& response, bool encrypt = true, bool wrap = true);
    uint32_t                                oid() const;
    void                                    oid(uint32_t value);
    virtual const fb::model::object&        model() const = 0;
    bool                                    is(OBJECT_TYPE type) const;
    virtual const std::string&              name() const;
    virtual uint16_t                        look() const;
    virtual uint8_t                         color() const;
    virtual OBJECT_TYPE                     what() const;
    virtual void                            show();
    virtual void                            show(object& you);
    virtual void                            update_external();
    virtual void                            update_external(object& you);
    virtual bool                            super_hide() const;
    virtual bool                            hidden(const object& target) const;
    virtual void                            chat(std::string_view message, CHAT_TYPE chat_type = CHAT_TYPE::NORMAL, bool decorate = true);
    const fb::model::point16_t&             position() const;
    virtual bool                            position(uint16_t x, uint16_t y, bool refresh = false);
    virtual bool                            position(const fb::model::point16_t position, bool refresh = false);
    bool                                    move();
    virtual bool                            move(DIRECTION direction);
    uint16_t                                x() const;
    bool                                    x(uint16_t value);
    uint16_t                                y() const;
    bool                                    y(uint16_t value);
    DIRECTION                               direction() const;
    bool                                    direction(DIRECTION value);
    virtual async::task<bool>               map(map_ptr map, std::optional<fb::model::point16_t> position = std::nullopt, map_options options = {});
    map_ptr                                 map() const;
    bool                                    active_sector() const;
    bool                                    sight(const fb::model::point16_t& position) const;
    bool                                    sight(const fb::game::object& object) const;
    fb::model::area<uint16_t>               sight_area() const;
    fb::model::point16_t                    side_position(DIRECTION direction, int step = 1) const;
    fb::model::point16_t                    front_position(int step = 1) const;
    object_ptr                              side(DIRECTION direction, OBJECT_TYPE type = OBJECT_TYPE::OBJECT) const;
    std::vector<object_ptr>                 sides(DIRECTION direction, OBJECT_TYPE type = OBJECT_TYPE::OBJECT) const;
    object_ptr                              forward(OBJECT_TYPE type = OBJECT_TYPE::OBJECT) const;
    std::vector<object_ptr>                 forwards(OBJECT_TYPE type = OBJECT_TYPE::OBJECT) const;
    double                                  distance(const object& right) const;
    uint32_t                                distance_sqrt(const object& right) const;
    virtual bool                            condition(const std::vector<fb::model::dsl>& conditions) const;
    virtual bool                            available() const;
    virtual void                            hide(DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);
    virtual void                            hide(fb::game::object& to, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);
    std::vector<object_ptr>                 sight_in(OBJECT_TYPE type = OBJECT_TYPE::OBJECT) const;
    std::vector<object_ptr>                 nears(OBJECT_TYPE type = OBJECT_TYPE::OBJECT, bool contains_super_hide = false) const;
    void                                    thread(fb::thread* value);
    virtual fb::thread*                     thread() const override;
    virtual void                            update_id();
    virtual void                            update_map(const fb::game::map& map);
    virtual void                            update_bgm(uint16_t bgm, uint8_t volume);
    virtual void                            update_position();
    void                                    sound(SOUND sound, uint8_t volume = 100);
    void                                    effect(uint8_t value);
    virtual appearance_ptr                  appearance() const = 0;
    // clang-format on

public:
    bool operator== (const object& right) const;
    bool operator!= (const object& right) const;
};

struct object::listener_t
{
    // clang-format off
    virtual void on_chat(fb::game::object& me, std::string_view message, CHAT_TYPE chat_type = CHAT_TYPE::NORMAL) = 0;
    virtual void on_direction(fb::game::object& me)                                                               = 0;
    virtual void on_show(fb::game::object& me)                                                                    = 0;
    virtual void on_show(fb::game::object& me, fb::game::object& you)                                             = 0;
    virtual void on_update_external(fb::game::object& me)                                                         = 0;
    virtual void on_update_external(fb::game::object& me, fb::game::object& you)                                  = 0;
    virtual void on_hide(fb::game::object& me, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT)                 = 0;
    virtual void on_hide(fb::game::object& me, fb::game::object& you, DESTROY_TYPE destroy_type)                  = 0;
    virtual void on_move(fb::game::object& me, const fb::model::point16_t& before)                                = 0;
    virtual void on_buff(fb::game::object& me, fb::game::buff& buff)                                              = 0;
    virtual void on_unbuff(fb::game::object& me, fb::game::buff& buff)                                            = 0;
    virtual void on_create(fb::game::object& me)                                                                  = 0;
    virtual void on_destroy(fb::game::object& me)                                                                 = 0;
    virtual void on_sound(fb::game::object& ch, SOUND sound, uint8_t volume = 100)                                = 0;
    virtual void on_effect(fb::game::object& ch, uint8_t value)                                                   = 0;
    virtual void on_map_leave(fb::game::object& me, const fb::game::map& map)                                     = 0;
    virtual void on_map_enter(fb::game::object& me, const fb::game::map& map)                                     = 0;
    // clang-format on
};

struct object::initial_params
{
public:
    uint32_t                   id;
    const fb::model::point16_t position  = fb::model::point16_t();
    DIRECTION                  direction = DIRECTION::BOTTOM;
    map_ptr                    map       = nullptr;
};

class object::container
{
public:
    using ptrs                  = std::unordered_map<uint32_t, std::shared_ptr<object>>;
    using iterator              = ptrs::iterator;
    using const_iterator        = ptrs::const_iterator;
    using handle_predicate_type = std::function<bool(object&)>;

private:
    ptrs                 _ptrs;
    uint32_t             _oid = 1;
    std::queue<uint32_t> _available_seq;

public:
    fb::game::map& owner;

public:
    container(fb::game::map& map);
    ~container() = default;

private:
    uint32_t allocate_seq();

public:
    // clang-format off
    iterator                begin();
    iterator                end();
    const_iterator          begin() const;
    const_iterator          end() const;
    uint32_t                size() const;
    std::shared_ptr<object> at(uint32_t i);
    void                    push(object& obj);
    object&                 pop(uint32_t fd);
    object&                 pop(object& obj);
    object*                 try_pop(uint32_t fd);
    object*                 try_pop(object& obj);
    // clang-format on

    // clang-format off
    void foreach (OBJECT_TYPE type, const handle_predicate_type& fn);
    bool contains(const std::shared_ptr<object>& obj) const;
    bool contains(uint32_t fd) const;
    // clang-format on

public:
    object* operator[] (uint32_t fd);
};

} // namespace fb::game

#endif // !__OBJECT_H__
