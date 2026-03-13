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
    class batch_update_guard;

protected:
    uint32_t           _damage_rate       = 1000;
    uint32_t           _skill_damage_rate = 1000;
    uint32_t           _damage_derate     = 1000;
    bool               _paralysis         = false;
    bool               _invincible        = false;
    bool               _cover             = false;
    bool               _batch_mode        = false;
    UPDATE_STATE_LEVEL _pending_update    = UPDATE_STATE_LEVEL::MINIMUM;

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
    // clang-format off
    virtual async::task<void> attack(DURATION duration = DURATION::ATTACK);
    virtual uint32_t          exp() const;
    virtual void              update(UPDATE_STATE_LEVEL value = UPDATE_STATE_LEVEL::EXP_MONEY | UPDATE_STATE_LEVEL::CROWD_CONTROL, bool notify = true);
    void                      update_hp(uint32_t diff, bool critical, bool notify = true);
    batch_update_guard        batch_update();
    virtual void              kill(std::shared_ptr<fb::game::object> from = nullptr, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);
    virtual bool              alive() const;
    bool                      active(fb::game::spell& spell);
    bool                      active(fb::game::spell& spell, uint32_t oid);
    bool                      active(fb::game::spell& spell, std::string_view message);
    bool                      active(fb::game::spell& spell, fb::game::object& to);
    virtual void              action(ACTION action, DURATION duration, uint8_t sound = 0x00);
    virtual uint32_t          normal_attack_damage(MOB_SIZE size) const = 0;
    virtual bool              calculate_critical(life& you) const;
    virtual uint32_t          calculate_damage(uint32_t damage, const life& you, bool critical, float rate = 1.0f, bool physical = true) const;
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
    // clang-format on
};

struct life::listener_t : public virtual fb::game::object::listener_t, public virtual fb::game::spells::listener_t
{
    virtual void on_action(life& me, ACTION action, DURATION duration, uint8_t sound) = 0;
    virtual void on_attack(life& me, DURATION duration = DURATION::ATTACK)            = 0;
    virtual void on_dead(life& me, std::shared_ptr<fb::game::object> you)             = 0;
    virtual void on_update_hp(life& me, uint32_t diff, bool critical)                 = 0;
};

/**
 * RAII guard that defers and batches consecutive update() calls into a single packet.
 * All update() invocations while the guard is alive are accumulated using bitwise OR.
 * The accumulated update is flushed when the guard goes out of scope.
 *
 * Usage:
 * @code
 * {
 *     auto batch = character.batch_update();
 *     character.stat.base_str(10);  // no packet
 *     character.stat.hp(500);       // no packet
 *     character.stat.mp(300);       // no packet
 * }  // single combined packet sent here
 * @endcode
 */
class life::batch_update_guard
{
    life& _owner;

public:
    explicit batch_update_guard(life& owner);
    batch_update_guard(const batch_update_guard&) = delete;
    batch_update_guard(batch_update_guard&&)      = delete;
    ~batch_update_guard();

public:
    batch_update_guard& operator= (const batch_update_guard&) = delete;
    batch_update_guard& operator= (batch_update_guard&&)      = delete;
};

struct life::initial_params : public fb::game::object::initial_params
{ };

} // namespace fb::game

#endif // !__LIFE_H__
