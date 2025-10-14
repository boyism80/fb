#ifndef __MOB_H__
#define __MOB_H__

#include <fb/game/life.h>
#include <fb/game/item.h>
#include <async/task.h>

using namespace std::chrono_literals;

namespace fb::game {

class character;
class ai;

class rezen
{
private:
    fb::game::server&                  _server;
    uint16_t                           _count = 0;
    std::optional<fb::model::datetime> _respawn_time;

public:
    const fb::model::mob_spawn& model;

public:
    rezen(fb::game::server& server, const fb::model::mob_spawn& model);
    ~rezen() = default;

    void                            decrease();
    [[nodiscard]] async::task<void> spawn(std::thread::id thread_id);
    void                            force_spawn(std::thread::id thread_id);
};

class mob : public life
{
public:
    using model_type = fb::model::mob;

public:
    LUA_PROTOTYPE

public:
    struct listener_t;

public:
    struct initial_params : fb::game::life::initial_params
    {
    public:
        const bool             alive = false;
        fb::game::rezen* const rezen = nullptr;
        character*             owner = nullptr;
    };

private:
    fb::model::datetime                          _action_time;
    std::weak_ptr<life>                          _target;
    std::weak_ptr<life>                          _oblivion;
    rezen*                                       _rezen         = nullptr;
    lua::context*                                _attack_thread = nullptr;
    uint32_t                                     _buff_hp       = 0;
    uint32_t                                     _buff_mp       = 0;
    uint8_t                                      _buff_str      = 0;
    uint8_t                                      _buff_dex      = 0;
    uint8_t                                      _buff_int      = 0;
    int8_t                                       _buff_phydef   = 0;
    int8_t                                       _buff_magdef   = 0;
    uint8_t                                      _buff_dam      = 0;
    uint8_t                                      _buff_hit      = 0;
    std::vector<std::shared_ptr<fb::game::item>> _items;
    bool                                         _hidden = false;
    std::unique_ptr<ai>                          _ai_strategy;

public:
    const std::weak_ptr<character> owner;

public:
    mob::listener_t& listener;

public:
    mob(fb::game::server& server, const fb::model::mob& model, const initial_params& params);
    mob(const mob& right);
    ~mob();

private:
    std::weak_ptr<fb::game::life>   find_target();
    [[nodiscard]] async::task<bool> call_script();
    void                            AI(const fb::model::datetime& now);

public:
    bool     near_target(const std::shared_ptr<fb::game::life>& target, DIRECTION& out) const;
    bool     move_step(const fb::model::point16_t& position);
    uint32_t base_hp() const override final;
    uint32_t buff_hp() const override final;
    void     buff_hp(uint32_t value) override final;
    uint32_t base_mp() const override final;
    uint32_t buff_mp() const override final;
    void     buff_mp(uint32_t value) override final;
    uint8_t  base_str() const override final;
    uint8_t  buff_str() const override final;
    void     buff_str(uint8_t value) override final;
    uint8_t  base_dex() const override final;
    uint8_t  buff_dex() const override final;
    void     buff_dex(uint8_t value) override final;
    uint8_t  base_int() const override final;
    uint8_t  buff_int() const override final;
    void     buff_int(uint8_t value) override final;
    int8_t   base_phydef() const override final;
    int8_t   buff_phydef() const override final;
    void     buff_phydef(int8_t value) override final;
    int8_t   base_magdef() const override final;
    int8_t   buff_magdef() const override final;
    void     buff_magdef(int8_t value) override final;
    uint8_t  base_dam() const override final;
    uint8_t  buff_dam() const override final;
    void     buff_dam(uint8_t value) override final;
    uint8_t  base_hit() const override final;
    uint8_t  buff_hit() const override final;
    void     buff_hit(uint8_t value) override final;

public:
    [[nodiscard]] async::task<void>                     action(fb::model::datetime now);
    const fb::model::datetime&                          action_time() const;
    void                                                action_time(const fb::model::datetime& dt);
    std::shared_ptr<fb::game::life>                     target() const;
    void                                                target(std::shared_ptr<fb::game::life> value);
    std::shared_ptr<fb::game::life>                     oblivion() const;
    void                                                oblivion(std::shared_ptr<fb::game::life> value);
    std::shared_ptr<fb::game::life>                     update_target();
    virtual bool                                        available() const;
    uint32_t                                            auto_attack_damage(MOB_SIZE size) const override final;
    uint32_t                                            damage(uint32_t value, std::shared_ptr<fb::game::object> from = nullptr, bool critical = false) override final;
    void                                                kill(std::shared_ptr<fb::game::object> from = nullptr, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) override final;
    async::task<void>                                   drop_items();
    void                                                assert_thread() const override final;
    bool                                                move(DIRECTION direction) override final;
    const std::vector<std::shared_ptr<fb::game::item>>& items() const;
    bool                                                push_item(std::shared_ptr<fb::game::item> item);
    bool                                                hidden(const fb::game::object& target) const override final;
    void                                                hidden(bool enabled);
};

struct mob::listener_t : public virtual fb::game::life::listener_t
{ };

} // namespace fb::game

#endif // !__MOB_H__
