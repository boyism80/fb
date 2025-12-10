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
#include <shared_mutex>

namespace fb::game {

using namespace fb::model::enum_value;

class server;
class map;
class character;
class buffs;
class objects;
class items;
class sector;
class sectors;

class object : public fb::thread_switchable
{
public:
    struct listener_t;
    struct initial_params;

    class container;

public:
    LUA_PROTOTYPE

    using map_ptr = std::shared_ptr<fb::game::map>;

    friend fb::game::buffs;
    friend fb::game::items;

private:
    std::shared_ptr<fb::game::sector> _sector;
    mutable std::shared_mutex         _map_lock;

protected:
    uint32_t                 _oid = 0;
    const fb::model::object& _model;
    fb::model::point16_t     _position  = fb::model::point16_t(0, 0);
    DIRECTION                _direction = DIRECTION::BOTTOM;
    map_ptr                  _map       = nullptr;
    fb::thread*              _thread    = nullptr;

public:
    listener_t&       listener;
    fb::game::server& server;
    fb::game::buffs   buffs;

protected:
    object(fb::game::server& server, const fb::model::object& model, const initial_params& c);
    object(const object& right);

public:
    virtual ~object();

private:
    void        update_sector();
    static bool sight(const fb::model::point16_t me, const fb::model::point16_t you, const map_ptr& map);

public:
    template <typename T> const T& based() const
    {
        return static_cast<const T&>(this->_model);
    }

    virtual void on_init();

public:
    [[nodiscard]] virtual async::task<void> destroy(DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);
    virtual async::task<size_t>             send(const fb::stream& stream, bool encrypt = true, bool wrap = true);
    virtual async::task<size_t> send(const fb::protocol::header& response, bool encrypt = true, bool wrap = true);
    uint32_t                    oid() const;
    void                        oid(uint32_t value);
    const fb::model::object&    based() const;
    bool                        is(OBJECT_TYPE type) const;
    virtual const std::string&  name() const;
    virtual uint16_t            look() const;
    virtual uint8_t             color() const;
    virtual OBJECT_TYPE         what() const;
    virtual void                update_external(bool detailed);
    virtual void                update_external(object& you, bool detailed);
    virtual bool                super_hide() const;
    virtual bool                hidden(const object& target) const;
    void chat(const std::string& message, CHAT_TYPE chat_type = CHAT_TYPE::NORMAL, bool decorate = true);
    const fb::model::point16_t&       position() const;
    virtual bool                      position(uint16_t x, uint16_t y, bool refresh = false);
    virtual bool                      position(const fb::model::point16_t position, bool refresh = false);
    bool                              move();
    virtual bool                      move(DIRECTION direction);
    uint16_t                          x() const;
    bool                              x(uint16_t value);
    uint16_t                          y() const;
    bool                              y(uint16_t value);
    DIRECTION                         direction() const;
    bool                              direction(DIRECTION value);
    virtual async::task<bool>         map(map_ptr map, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);
    virtual async::task<bool>         map(map_ptr                     map,
                                          const fb::model::point16_t& position,
                                          DESTROY_TYPE                destroy_type = DESTROY_TYPE::DEFAULT,
                                          bool                        notify       = true);
    map_ptr                           map() const;
    bool                              sight(const fb::model::point16_t& position) const;
    bool                              sight(const fb::game::object& object) const;
    fb::model::point16_t              side_position(DIRECTION direction, int step = 1) const;
    fb::model::point16_t              front_position(int step = 1) const;
    std::shared_ptr<fb::game::object> side(DIRECTION direction, OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    std::vector<std::shared_ptr<fb::game::object>> sides(DIRECTION   direction,
                                                         OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    std::shared_ptr<fb::game::object>              forward(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    std::vector<std::shared_ptr<fb::game::object>> forwards(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    double                                         distance(const object& right) const;
    uint32_t                                       distance_sqrt(const object& right) const;
    virtual bool                                   condition(const std::vector<fb::model::dsl>& conditions) const;
    virtual bool                                   available() const;
    virtual void                                   hide(DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);
    virtual void hide(fb::game::object& to, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);
    std::vector<std::shared_ptr<fb::game::object>> sight_in(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    std::vector<std::shared_ptr<fb::game::object>> nears(OBJECT_TYPE type                = OBJECT_TYPE::UNKNOWN,
                                                         bool        contains_super_hide = false) const;
    void                                           thread(fb::thread* value);
    virtual fb::thread*                            thread() const override;
    virtual void                                   update_id();
    virtual void                                   update_map(const fb::game::map& map);
    virtual void                                   update_bgm(uint16_t bgm, uint8_t volume);
    virtual void                                   update_position();
    void                                           sound(SOUND sound);
    void                                           effect(uint8_t value);

public:
    bool operator== (const object& right) const;
    bool operator!= (const object& right) const;
};

struct object::listener_t
{
    virtual void on_chat(fb::game::object& me, const std::string& message, CHAT_TYPE chat_type = CHAT_TYPE::NORMAL) = 0;
    virtual void on_direction(fb::game::object& me)                                                                 = 0;
    virtual void on_update_external(fb::game::object& me, bool detailed)                                            = 0;
    virtual void on_update_external(fb::game::object& me, fb::game::object& you, bool detailed)                     = 0;
    virtual void on_hide(fb::game::object& me, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT)                   = 0;
    virtual void on_hide(fb::game::object& me, fb::game::object& you, DESTROY_TYPE destroy_type)                    = 0;
    virtual void on_move(fb::game::object& me, const fb::model::point16_t& before)                                  = 0;
    virtual void on_buff(fb::game::object& me, fb::game::buff& buff)                                                = 0;
    virtual void on_unbuff(fb::game::object& me, fb::game::buff& buff)                                              = 0;
    virtual void on_create(fb::game::object& me)                                                                    = 0;
    virtual void on_destroy(fb::game::object& me)                                                                   = 0;
    virtual void on_sound(fb::game::object& ch, SOUND sound)                                                        = 0;
    virtual void on_effect(fb::game::object& ch, uint8_t value)                                                     = 0;
    virtual void on_map_leave(fb::game::object& me, const fb::game::map& map)                                       = 0;
    virtual void on_map_enter(fb::game::object& me, const fb::game::map& map)                                       = 0;
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

    void foreach (OBJECT_TYPE type, const handle_predicate_type& fn);
    bool contains(const std::shared_ptr<object>& obj) const;
    bool contains(uint32_t fd) const;

public:
    object* operator[] (uint32_t fd);
};

} // namespace fb::game

#endif // !__OBJECT_H__
