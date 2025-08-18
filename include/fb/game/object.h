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

using namespace fb::model::enum_value;

namespace fb::game {

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
private:
    using map_ptr = std::shared_ptr<fb::game::map>;

public:
    struct listener_t;
    struct initial_params;

public:
    LUA_PROTOTYPE

    friend fb::game::buffs;
    friend fb::game::items;

private:
    std::shared_ptr<fb::game::sector> _sector;
    mutable std::shared_mutex         _map_lock;

protected:
    uint32_t                       _oid = 0;
    const fb::model::object&       _model;
    fb::model::point16_t           _position  = fb::model::point16_t(0, 0);
    DIRECTION                      _direction = DIRECTION::BOTTOM;
    std::shared_ptr<fb::game::map> _map       = nullptr;
    fb::thread*                    _thread    = nullptr;

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
    void        leave();
    static bool sight(const fb::model::point16_t me, const fb::model::point16_t you, const std::shared_ptr<fb::game::map>& map);

public:
    template <typename T> const T& based() const
    {
        return static_cast<const T&>(this->_model);
    }

    virtual void on_init();

public:
    [[nodiscard]] virtual async::task<void> destroy(DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);
    virtual async::task<size_t>             send(const fb::stream& stream, bool encrypt = true, bool wrap = true);
    virtual async::task<size_t>             send(const fb::protocol::header& response, bool encrypt = true, bool wrap = true);
    uint32_t                                oid() const;
    void                                    oid(uint32_t value);
    const fb::model::object&                based() const;
    bool                                    is(OBJECT_TYPE type) const;
    virtual const std::string&              name() const;
    virtual uint16_t                        look() const;
    virtual uint8_t                         color() const;
    virtual OBJECT_TYPE                     what() const;
    virtual void                            update_external(bool detailed);
    virtual void                            update_external(object& you, bool detailed);
    virtual bool                            super_hide() const;
    virtual bool                            hidden(const object& target) const;
    void                                    chat(const std::string& message, CHAT_TYPE chat_type = CHAT_TYPE::NORMAL, bool decorate = true);
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
    virtual async::task<bool>               map(map_ptr map, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);
    virtual async::task<bool>               map(map_ptr map, const fb::model::point16_t& position, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT, bool notify = true);
    map_ptr                                 map() const;
    bool                                    sight(const fb::model::point16_t& position) const;
    bool                                    sight(const object& object) const;
    fb::model::point16_t                    side_position(DIRECTION direction, int step = 1) const;
    fb::model::point16_t                    front_position(int step = 1) const;
    std::shared_ptr<object>                 side(DIRECTION direction, OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    std::vector<std::shared_ptr<object>>    sides(DIRECTION direction, OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    std::shared_ptr<object>                 forward(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    std::vector<std::shared_ptr<object>>    forwards(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    double                                  distance(const object& right) const;
    uint32_t                                distance_sqrt(const object& right) const;
    virtual bool                            condition(const std::vector<fb::model::dsl>& conditions) const;
    virtual bool                            available() const;
    virtual void                            hide(DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);
    virtual void                            hide(object& to, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);
    std::vector<std::shared_ptr<object>>    sight_in(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    std::vector<std::shared_ptr<object>>    nears(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN, bool contains_super_hide = false) const;
    void                                    thread(fb::thread* value);
    virtual fb::thread*                     thread() const override;
    virtual void                            update_id();
    virtual void                            update_map(const fb::game::map& map);
    virtual void                            update_bgm(uint16_t bgm, uint8_t volume);
    virtual void                            update_position();
    void                                    sound(SOUND sound);
    void                                    effect(uint8_t value);

public:
    bool operator== (const object& right) const;
    bool operator!= (const object& right) const;
};

struct object::listener_t
{
    virtual void on_chat(object& me, const std::string& message, CHAT_TYPE chat_type = CHAT_TYPE::NORMAL) = 0;
    virtual void on_direction(object& me)                                                                 = 0;
    virtual void on_update_external(object& me, bool detailed)                                            = 0;
    virtual void on_update_external(object& me, object& you, bool detailed)                               = 0;
    virtual void on_hide(object& me, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT)                   = 0;
    virtual void on_hide(object& me, object& you, DESTROY_TYPE destroy_type)                              = 0;
    virtual void on_move(object& me, const fb::model::point16_t& before)                                  = 0;
    virtual void on_buff(object& me, fb::game::buff& buff)                                                = 0;
    virtual void on_unbuff(object& me, fb::game::buff& buff)                                              = 0;
    virtual void on_create(object& me)                                                                    = 0;
    virtual void on_destroy(object& me)                                                                   = 0;
    virtual void on_sound(object& ch, SOUND sound)                                                        = 0;
    virtual void on_effect(object& ch, uint8_t value)                                                     = 0;
    virtual void on_map_leave(object& me, const fb::game::map& map)                                       = 0;
    virtual void on_map_enter(object& me, const fb::game::map& map)                                       = 0;
};

struct object::initial_params
{
public:
    uint32_t                       id;
    const fb::model::point16_t     position  = fb::model::point16_t();
    DIRECTION                      direction = DIRECTION::BOTTOM;
    std::shared_ptr<fb::game::map> map       = nullptr;
};

} // namespace fb::game

#endif // !__OBJECT_H__
