#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "mmo.h"
#include "stream.h"

namespace fb { namespace game {

class map;
class session;


//
// object
//
class object
{
public:
    enum types { UNKNOWN = 0x00, ITEM = 0x01, NPC = 0x02, MOB = 0x04 };

public:
    class core
    {
    protected:
        uint32_t                    _id;
        std::string                 _name;
        uint16_t                    _look;
        uint8_t                     _color;

    public:
        friend class fb::game::object;

    public:
        core(uint32_t id, const std::string& name = "", uint16_t look = 0, uint8_t color = 0);
        virtual ~core();

    public:
        uint32_t                    id() const;

        const std::string&          name() const;
        void                        name(const std::string& value);

        uint16_t                    look() const;
        void                        look(uint16_t value);

        uint8_t                     color() const;
        void                        color(uint8_t value);

        virtual object*             make() const = 0;
        template <typename T>
        T*                          make() const { return static_cast<T*>(this->make()); }
    };

protected:
    const core*                 _core;
    

protected:
    uint32_t                    _id;
    point16_t                   _position;
    fb::game::direction         _direction;
    fb::game::map*              _map;

protected:
    object(const core* core, uint32_t id = 0xFFFFFFFF, const point16_t position = fb::game::point16_t(), fb::game::direction direction = fb::game::direction::BOTTOM, fb::game::map* map = NULL);
    object(const object& right);
public:
    virtual ~object();

public:
    const core*                 based() const;
    template <typename T>
    const T*                    based() const { return static_cast<const T*>(this->_core); }
    template <typename T>
    T*                          clone() const { return static_cast<T*>(this->_core->make<T>()); }
    
    uint32_t                    id() const;
    void                        id(uint32_t value);

    const std::string&          name() const;
    uint16_t                    look() const;
    uint8_t                     color() const;
    virtual object::types       type() const;


    point16_t                   position() const;
    point16_t                   position_forward() const;
    virtual bool                position(uint16_t x, uint16_t y);
    virtual bool                position(const point16_t position);

    uint16_t                    x() const;
    bool                        x(uint16_t value);

    uint16_t                    y() const;
    bool                        y(uint16_t value);

    direction                   direction() const;
    bool                        direction(fb::game::direction value);

    fb::game::map*              map() const;
    virtual uint16_t            map(fb::game::map* map);
    virtual uint16_t            map(fb::game::map* map, const point16_t& position);

    bool                        sight(const point16_t& position) const;
    bool                        sight(const fb::game::object& object) const;
    static bool                 sight(const point16_t me, const point16_t you, const fb::game::map* map);

    session*                    near_session(fb::game::direction direction) const;
    std::vector<session*>       near_sessions(fb::game::direction direction) const;
    session*                    forward_session() const;
    std::vector<session*>       forward_sessions() const;

    object*                     near_object(fb::game::direction direction, fb::game::object::types type) const;
    std::vector<object*>        near_objects(fb::game::direction direction, fb::game::object::types type) const;
    object*                     forward_object(fb::game::object::types type) const;
    std::vector<object*>        forward_objects(fb::game::object::types type) const;

    std::vector<object*>        shown_objects() const;
    std::vector<session*>       shown_sessions() const;
    std::vector<session*>       looking_sessions() const;

    virtual bool                alive() const;

    double                      distance(const object& right) const;
    uint32_t                    distance_sqrt(const object& right) const;

public:
    fb::ostream                 make_show_stream() const;
    fb::ostream                 make_hide_stream() const;
    static fb::ostream          make_show_stream(const std::vector<fb::game::object*>& objects);
    fb::ostream                 make_sound_stream(fb::game::action_sounds sound) const;

public:
    virtual void                handle_timer(uint64_t elapsed_milliseconds) {}
};


class life : public object
{
public:
    class core : public fb::game::object::core
    {
    protected:
        fb::game::defensive     _defensive;
        uint32_t                _hp, _mp;
        uint32_t                _experience;

    public:
        friend class fb::game::life;

    public:
        core(uint32_t id, const std::string& name, uint16_t look, uint8_t color, uint32_t hp, uint32_t mp);
        core(const core& core, uint32_t hp, uint32_t mp);
        core(const core& core);
        virtual ~core();

    public:
        uint32_t                hp() const;
        void                    hp(uint32_t value);

        uint32_t                mp() const;
        void                    mp(uint32_t value);

        uint32_t                experience() const;
        void                    experience(uint32_t value);

        uint32_t                defensive_physical() const;
        void                    defensive_physical(uint8_t value);

        uint32_t                defensive_magical() const;
        void                    defensive_magical(uint8_t value);

        object*                 make() const;
    };

private:
    point16_t                   _before;

protected:
    uint32_t                    _hp, _mp;
    fb::game::condition         _condition;

protected:
    life(const core* core);
    life(core* core, uint32_t id, uint32_t hp = 0, uint32_t mp = 0, uint32_t exp = 0);
    life(const fb::game::object& object, uint32_t hp, uint32_t mp, uint32_t exp);
    virtual ~life();

private:
    bool                        movable(fb::game::direction direction) const;
    bool                        movable_forward() const;

protected:
    uint32_t                    random_damage(uint32_t value, const fb::game::life& life) const;

public:
    point16_t                   position() const;
    bool                        position(uint16_t x, uint16_t y);
    bool                        position(const point16_t position);
    bool                        move(fb::game::direction direction, std::vector<object*>* show_objects = NULL, std::vector<object*>* hide_objects = NULL, std::vector<session*>* show_sessions = NULL, std::vector<session*>* hide_sessions = NULL, std::vector<object*>* shown_objects = NULL, std::vector<object*>* hidden_objects = NULL, std::vector<session*>* shown_sessions = NULL, std::vector<session*>* hidden_sessions = NULL);
    bool                        move_forward(std::vector<object*>* show_objects = NULL, std::vector<object*>* hide_objects = NULL, std::vector<session*>* show_sessions = NULL, std::vector<session*>* hide_sessions = NULL, std::vector<object*>* hown_objects = NULL, std::vector<object*>* hidden_objects = NULL, std::vector<session*>* shown_sessions = NULL, std::vector<session*>* hidden_sessions = NULL);

    virtual uint32_t            hp() const;
    virtual void                hp(uint32_t value);
    void                        heal(uint32_t value = 0xFFFFFFFF);

    virtual uint32_t            mp() const;
    virtual void                mp(uint32_t value);

    virtual uint32_t            base_hp() const;
    virtual uint32_t            base_mp() const;

    virtual uint32_t            experience() const;

    virtual uint32_t            defensive_physical() const;
    virtual uint32_t            defensive_magical() const;

    void                        hp_up(uint32_t value);
    void                        hp_down(uint32_t value);

    void                        mp_up(uint32_t value);
    void                        mp_down(uint32_t value);

    fb::game::condition         condition() const;
    fb::game::condition         condition_add(fb::game::condition value);
    fb::game::condition         condition_remove(fb::game::condition value);
    bool                        condition_contains(fb::game::condition value) const;

    bool                        alive() const;
    void                        kill();

public:
    fb::ostream                 make_move_stream(bool from_before = true) const;
    fb::ostream                 make_move_stream(fb::game::direction direction, bool from_before = true) const;
    fb::ostream                 make_direction_stream() const;

    fb::ostream                 make_action_stream(fb::game::action action, fb::game::duration duration) const;

    fb::ostream                 make_show_hp_stream(uint32_t random_damage, bool critical) const;
    fb::ostream                 make_die_stream() const;
};

} }

#endif // !__OBJECT_H__
