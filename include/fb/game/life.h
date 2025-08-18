#ifndef __LIFE_H__
#define __LIFE_H__

#include <fb/game/object.h>

namespace fb::game {

class life : public object
{
public:
    LUA_PROTOTYPE

public:
    struct listener_t;
    struct initial_params;

protected:
    uint32_t      _hp                = 0;
    uint32_t      _mp                = 0;
    uint32_t      _damage_rate       = 1000;
    uint32_t      _skill_damage_rate = 1000;
    uint32_t      _damage_derate     = 1000;
    CROWD_CONTROL _crowd_control     = CROWD_CONTROL::NONE;
    bool          _paralysis         = false;
    bool          _invincible        = false;
    bool          _cover             = false;

public:
    listener_t&      listener;
    fb::game::spells spells;

protected:
    life(fb::game::server& server, const fb::model::life& model, const initial_params& params);
    virtual ~life();

public:
    virtual void on_init() override;

public:
#pragma region stat
    virtual uint32_t base_hp() const = 0;
    virtual uint32_t buff_hp() const = 0;
    virtual uint32_t maxhp() const;
    virtual void     buff_hp(uint32_t value) = 0;
    virtual uint32_t base_mp() const         = 0;
    virtual uint32_t buff_mp() const         = 0;
    virtual uint32_t maxmp() const;
    virtual void     buff_mp(uint32_t value) = 0;
    virtual uint8_t  base_str() const        = 0;
    virtual uint8_t  buff_str() const        = 0;
    virtual uint8_t  str() const;
    virtual void     buff_str(uint8_t value) = 0;
    virtual uint8_t  base_dex() const        = 0;
    virtual uint8_t  buff_dex() const        = 0;
    virtual uint8_t  dex() const;
    virtual void     buff_dex(uint8_t value) = 0;
    virtual uint8_t  base_int() const        = 0;
    virtual uint8_t  buff_int() const        = 0;
    virtual uint8_t  intelligence() const;
    virtual void     buff_int(uint8_t value) = 0;
    virtual int8_t   base_phydef() const     = 0;
    virtual int8_t   buff_phydef() const     = 0;
    virtual int8_t   phydef() const;
    virtual void     buff_phydef(int8_t value) = 0;
    virtual int8_t   base_magdef() const       = 0;
    virtual int8_t   buff_magdef() const       = 0;
    virtual int8_t   magdef() const;
    virtual void     buff_magdef(int8_t value) = 0;
    virtual uint8_t  base_dam() const          = 0;
    virtual uint8_t  buff_dam() const          = 0;
    virtual uint8_t  dam() const;
    virtual void     buff_dam(uint8_t value) = 0;
    virtual uint8_t  base_hit() const        = 0;
    virtual uint8_t  buff_hit() const        = 0;
    virtual uint8_t  hit() const;
    virtual void     buff_hit(uint8_t value) = 0;
#pragma endregion

public:
    virtual async::task<void> attack(DURATION duration = DURATION::ATTACK);
    virtual uint32_t          exp() const;
    virtual uint32_t          heal(uint32_t value, object* from = nullptr);
    virtual uint32_t          damage(uint32_t value, std::shared_ptr<object> from = nullptr, bool critical = false);
    virtual uint32_t          mp_up(uint32_t value, object* from = nullptr);
    virtual uint32_t          mp_down(uint32_t value, object* from = nullptr);
    virtual void              update(STATE_LEVEL value = STATE_LEVEL::LEVEL_MIN);
    virtual void              kill(std::shared_ptr<object> from = nullptr, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);
    virtual bool              alive() const;
    virtual void              action(ACTION action, DURATION duration, uint8_t sound = 0x00);
    virtual uint32_t          auto_attack_damage(MOB_SIZE size) const = 0;
    virtual bool              calculate_critical(life& you) const;
    virtual uint32_t          calculate_damage(uint32_t damage, const life& you, bool critical) const;
    virtual bool              calculate_miss(life& you) const;
    uint32_t                  hp() const;
    void                      hp(uint32_t value);
    uint32_t                  mp() const;
    void                      mp(uint32_t value);
    void                      update_hp(uint32_t diff, bool critical);
    CROWD_CONTROL             crowd_control() const;
    void                      crowd_control(CROWD_CONTROL value);
    CROWD_CONTROL             add_cc(CROWD_CONTROL value);
    CROWD_CONTROL             remove_cc(CROWD_CONTROL value);
    bool                      contains_cc(CROWD_CONTROL value) const;
    bool                      active(spell& spell);
    bool                      active(spell& spell, uint32_t fd);
    bool                      active(spell& spell, const std::string& message);
    bool                      active(spell& spell, object& to);
    uint32_t                  damage_rate() const;
    void                      damage_rate(uint32_t value);
    uint32_t                  skill_damage_rate() const;
    void                      skill_damage_rate(uint32_t value);
    uint32_t                  damage_derate() const;
    void                      damage_derate(uint32_t value);
    void                      paralysis(bool value);
    bool                      paralysis() const;
    void                      invincible(bool value);
    bool                      invincible() const;
    void                      cover(bool value);
    bool                      cover() const;
};

struct life::listener_t : public virtual object::listener_t, public virtual fb::game::spells::listener_t
{
    virtual void on_action(life& me, ACTION action, DURATION duration, uint8_t sound) = 0;
    virtual void on_attack(life& me, DURATION duration = DURATION::ATTACK)            = 0;
    virtual void on_dead(life& me, std::shared_ptr<object> you)                       = 0;
    virtual void on_update_hp(life& me, uint32_t diff, bool critical)                 = 0;
};

struct life::initial_params : public object::initial_params
{
public:
    uint32_t hp  = 0;
    uint32_t mp  = 0;
    uint32_t exp = 0;
};

} // namespace fb::game

#endif // !__LIFE_H__
