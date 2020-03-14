#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "mmo.h"
#include "stream.h"
#include "lua.h"

namespace fb { namespace game {

class map;
class session;

class object : public fb::base
{
public:
    enum types 
    { 
        UNKNOWN = 0x00, 
        ITEM    = 0x01, 
        NPC     = 0x02, 
        MOB     = 0x04, 
        SESSION = 0x08, 
        OBJECT  = (ITEM | NPC | MOB) };

public:
    class core
    {
	public:
		DECLARE_LUA_PROTOTYPE

    protected:
        std::string             _name;
        uint16_t                _look;
        uint8_t                 _color;

    public:
        friend class fb::game::object;

    public:
        core(const std::string& name = "", uint16_t look = 0, uint8_t color = 0);
        virtual ~core();

    protected:
        uint8_t                 dialog_look_type() const;
		virtual void			handle_lua_field(lua_State* lua) const;

    public:
        virtual object::types   type() const;

        const std::string&      name() const;
        void                    name(const std::string& value);

        uint16_t                look() const;
        void                    look(uint16_t value);

        uint8_t                 color() const;
        void                    color(uint8_t value);

        virtual object*         make() const = 0;
        template <typename T>
        T*                      make() const { return static_cast<T*>(this->make()); }
		void					make_lua_table(lua_State* lua) const;
        
	public:
        fb::ostream             make_dialog_stream(const std::string& message, bool button_prev, bool button_next, fb::game::map* map = nullptr, dialog::interaction interaction = dialog::interaction::NORMAL) const;


	public:
		IMPLEMENT_NEW_LUA

		static int				builtin_name(lua_State* lua);
		static int				builtin_look(lua_State* lua);
		static int				builtin_color(lua_State* lua);
		static int				builtin_dialog(lua_State* lua);
    };

public:
	DECLARE_LUA_PROTOTYPE

protected:
    const core*                 _core;
	point16_t                   _before;
    

protected:
    point16_t                   _position;
    fb::game::direction         _direction;
    fb::game::map*              _map;

protected:
    object(const core* core, uint32_t id = 0xFFFFFFFF, const point16_t position = fb::game::point16_t(), fb::game::direction direction = fb::game::direction::BOTTOM, fb::game::map* map = nullptr);
    object(const object& right);
public:
    virtual ~object();

public:
	BUILTIN_CORE(fb::game::object)

private:
	static bool                 sight(const point16_t me, const point16_t you, const fb::game::map* map);

public:
    const core*                 based() const;
    template <typename T>
    const T*                    based() const { return static_cast<const T*>(this->_core); }
    template <typename T>
    T*                          clone() const { return static_cast<T*>(this->_core->make<T>()); }

    virtual const std::string&  name() const;
    virtual uint16_t            look() const;
    virtual uint8_t             color() const;
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

    bool                        sight(const point16_t& position, bool before = false) const;
    bool                        sight(const fb::game::object& object, bool before_me = false, bool before_you = false) const;

    session*                    near_session(fb::game::direction direction) const;
    std::vector<session*>       near_sessions(fb::game::direction direction) const;
    session*                    forward_session() const;
    std::vector<session*>       forward_sessions() const;

    object*                     near_object(fb::game::direction direction, fb::game::object::types type) const;
    std::vector<object*>        near_objects(fb::game::direction direction, fb::game::object::types type) const;
    object*                     forward_object(fb::game::object::types type) const;
    std::vector<object*>        forward_objects(fb::game::object::types type) const;

    std::vector<object*>        showings(object::types type = object::types::UNKNOWN) const;
    std::vector<object*>        showns(object::types type = object::types::UNKNOWN) const;

    virtual bool                alive() const;

    double                      distance(const object& right) const;
    uint32_t                    distance_sqrt(const object& right) const;

public:
    virtual fb::ostream         make_show_stream() const;
    fb::ostream                 make_hide_stream() const;
    static fb::ostream          make_show_stream(const std::vector<fb::game::object*>& objects);
    fb::ostream                 make_chat_stream(const std::string& message, chat::type type = chat::NORMAL) const;
    fb::ostream                 make_sound_stream(fb::game::sound::type sound) const;
    fb::ostream                 make_dialog_stream(const std::string& message, bool button_prev, bool button_next) const;

public:
    virtual void                handle_timer(uint64_t elapsed_milliseconds) {}

public:
	IMPLEMENT_NEW_LUA

public:
	static int					builtin_dialog(lua_State* lua);
	static int					builtin_sound(lua_State* lua);
	static int					builtin_position(lua_State* lua);
    static int                  builtin_chat(lua_State* lua);
};


class life : public object
{
public:
    class core : public fb::game::object::core
    {
	public:
		DECLARE_LUA_PROTOTYPE

    protected:
        fb::game::defensive     _defensive;
        uint32_t                _hp, _mp;
        uint32_t                _experience;

    public:
        friend class fb::game::life;

    public:
        core(const std::string& name, uint16_t look, uint8_t color, uint32_t hp, uint32_t mp);
        core(const core& core, uint32_t hp, uint32_t mp);
        core(const core& core);
        virtual ~core();

	protected:
		virtual void			handle_lua_field(lua_State* lua) const;

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

	public:
		static int				builtin_hp(lua_State* lua);
		static int				builtin_mp(lua_State* lua);
    };

protected:
    uint32_t                    _hp, _mp;
    fb::game::condition         _condition;

protected:
    life(const core* core);
    life(core* core, uint32_t id, uint32_t hp = 0, uint32_t mp = 0, uint32_t exp = 0);
    life(const fb::game::object& object, uint32_t hp, uint32_t mp, uint32_t exp);
    virtual ~life();

public:
	DECLARE_LUA_PROTOTYPE
	BUILTIN_CORE(fb::game::life)

private:
    bool                        movable(fb::game::direction direction) const;
    bool                        movable_forward() const;

protected:
    uint32_t                    random_damage(uint32_t value, const fb::game::life& life) const;

public:
    bool                        move(fb::game::direction direction, std::vector<object*>* shows = nullptr, std::vector<object*>* hides = nullptr, std::vector<object*>* showns = nullptr, std::vector<object*>* hiddens = nullptr);
    bool                        move_forward(std::vector<object*>* shows = nullptr, std::vector<object*>* hides = nullptr, std::vector<object*>* showns = nullptr, std::vector<object*>* hiddens = nullptr);

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

public:
	static int					builtin_hp(lua_State* lua);
	static int					builtin_mp(lua_State* lua);
};

} }

#endif // !__OBJECT_H__
