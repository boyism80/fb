#ifndef __LIFE_H__
#define __LIFE_H__

#include <fb/game/object.h>
#include <fb/game/stat.h>
#include <fb/game/crowd_control.h>

namespace fb::game {

class life : public object
{
public:
    LUA_PROTOTYPE

public:
    struct listener_t;
    struct initial_params;

protected:
    uint32_t _damage_rate       = 1000;
    uint32_t _skill_damage_rate = 1000;
    uint32_t _damage_derate     = 1000;
    bool     _paralysis         = false;
    bool     _invincible        = false;
    bool     _cover             = false;

public:
    listener_t&      listener;
    fb::game::spells spells;
    fb::game::stat&  stat;
    crowd_control    cc = crowd_control(*this);

protected:
    life(fb::game::server& server, const fb::model::life& model, fb::game::stat& stat, const initial_params& params);
    virtual ~life();

public:
    virtual void on_init() override;

public:
    virtual async::task<void> attack(DURATION duration = DURATION::ATTACK);
    virtual uint32_t          exp() const;
    virtual void              update(UPDATE_STATE_LEVEL value = UPDATE_STATE_LEVEL::EXP_MONEY | UPDATE_STATE_LEVEL::CROWD_CONTROL);
    void                      update_hp(uint32_t diff, bool critical);
    virtual void              kill(std::shared_ptr<fb::game::object> from = nullptr, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);
    virtual bool              alive() const;
    bool                      active(fb::game::spell& spell);
    bool                      active(fb::game::spell& spell, uint32_t fd);
    bool                      active(fb::game::spell& spell, const std::string& message);
    bool                      active(fb::game::spell& spell, fb::game::object& to);
    virtual void              action(ACTION action, DURATION duration, uint8_t sound = 0x00);
    virtual uint32_t          auto_attack_damage(MOB_SIZE size) const = 0;
    virtual bool              calculate_critical(life& you) const;
    virtual uint32_t          calculate_damage(uint32_t damage, const life& you, bool critical) const;
    virtual bool              calculate_miss(life& you) const;
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

struct life::listener_t : public virtual fb::game::object::listener_t, public virtual fb::game::spells::listener_t
{
    virtual void on_action(life& me, ACTION action, DURATION duration, uint8_t sound) = 0;
    virtual void on_attack(life& me, DURATION duration = DURATION::ATTACK)            = 0;
    virtual void on_dead(life& me, std::shared_ptr<fb::game::object> you)             = 0;
    virtual void on_update_hp(life& me, uint32_t diff, bool critical)                 = 0;
};

struct life::initial_params : public fb::game::object::initial_params
{ };

} // namespace fb::game

#endif // !__LIFE_H__
