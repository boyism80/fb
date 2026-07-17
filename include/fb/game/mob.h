#ifndef __MOB_H__
#define __MOB_H__

#include <fb/game/life.h>
#include <fb/game/item.h>
#include <fb/game/appearance.h>
#include <async/task.h>
#include <vector>
#include <memory>

namespace fb::game {

using namespace std::chrono_literals;

class character;
class ai;

class map;

enum class MOB_PARTS_MODE : uint8_t
{
    PARTS = 0, // Part has its own HP; body HP tracks the sum of parts
    BODY  = 1, // Part is hitbox only; only body HP decreases
};

class rezen
{
private:
    fb::game::server&                  _server;
    std::weak_ptr<fb::game::map>       _map;
    uint16_t                           _count = 0;
    std::optional<fb::model::datetime> _respawn_time;

public:
    const fb::model::mob_spawn& model;

public:
    rezen(fb::game::server& server, const fb::model::mob_spawn& model, const std::shared_ptr<fb::game::map>& map);
    ~rezen() = default;

    // clang-format off
    uint32_t                        map_id() const;
    void                            decrease();
    [[nodiscard]] async::task<void> spawn(std::thread::id thread_id);
    void                            force_spawn(std::thread::id thread_id);
    // clang-format on
};

class mob : public life
{
public:
    using model_type     = fb::model::mob;
    using item_vector_t  = std::vector<std::shared_ptr<fb::game::item>>;
    using appearance_ptr = std::shared_ptr<fb::game::appearance>;
    using parts_vector_t = std::vector<std::shared_ptr<mob>>;

    friend class mob_stat;

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
    fb::model::datetime             _action_time;
    std::weak_ptr<life>             _target;
    std::weak_ptr<life>             _oblivion;
    rezen*                          _rezen         = nullptr;
    lua::context*                   _attack_thread = nullptr;
    item_vector_t                   _items;
    bool                            _hidden = false;
    std::unique_ptr<ai>             _ai_strategy;
    std::weak_ptr<mob>              _body;
    std::vector<std::weak_ptr<mob>> _parts;
    MOB_PARTS_MODE                  _parts_mode        = MOB_PARTS_MODE::PARTS;
    bool                            _forwarding_damage = false;
    bool                            _destroying        = false;

public:
    const std::weak_ptr<character> owner;
    mob::listener_t&               listener;
    fb::game::mob_stat             stat;

public:
    // clang-format off
    mob(fb::game::server& server, const fb::model::mob& model, const initial_params& params);
    ~mob();
    // clang-format on

private:
    // clang-format off
    std::weak_ptr<fb::game::life>   find_target();
    [[nodiscard]] async::task<bool> call_script();
    void                            AI(const fb::model::datetime& now);
    static bool                     is_cardinally_adjacent(const fb::model::point16_t& a, const fb::model::point16_t& b);
    static bool                     is_cover_barrier_cell(const fb::model::point16_t& cell, const fb::model::point16_t& cover_center);
    bool                            cover_blocks_move(const fb::game::map& map, const fb::model::point16_t& from, const fb::model::point16_t& to) const;
    uint64_t                        damage_as_part(uint64_t value, std::shared_ptr<object> from, bool critical, float rate, bool physical, bool fixed, bool notify);
    void                            sync_body_hp_from_parts();
    void                            unlink_part(mob& part);
    void                            on_part_hp_increased(uint64_t delta);
    [[nodiscard]] async::task<void> drop_model_items(const fb::model::mob& model, const fb::model::point16_t& position, std::vector<uint32_t>& oids);
    // clang-format on

public:
    // clang-format off
    bool near_target(const std::shared_ptr<fb::game::life>& target, DIRECTION& out) const;
    bool move_step(const fb::model::point16_t& position);
    // clang-format on

public:
    // clang-format off
    [[nodiscard]] async::task<void> action(fb::model::datetime now);
    const fb::model::datetime&      action_time() const;
    void                            action_time(const fb::model::datetime& dt);
    std::shared_ptr<fb::game::life> target() const;
    void                            target(std::shared_ptr<fb::game::life> value);
    std::shared_ptr<fb::game::life> oblivion() const;
    void                            oblivion(std::shared_ptr<fb::game::life> value);
    std::shared_ptr<fb::game::life> update_target();
    virtual bool                    available() const;
    uint64_t                        normal_attack_damage(MOB_SIZE size) const override final;
    void                            kill(DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) override final;
    async::task<void>               damage_to(const damage_list& targets) override final;
    async::task<void>               damage_to(const damage_list& targets, const damage_opts& opts) override final;
    async::task<void>               drop_items();
    async::task<void>               destroy(DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) override final;
    void                            assert_thread() const override final;
    bool                            move(DIRECTION direction) override final;
    const item_vector_t&            items() const;
    bool                            push_item(std::shared_ptr<fb::game::item> item);
    bool                            hidden(const fb::game::object& target) const override final;
    void                            hidden(bool enabled);
    appearance_ptr                  appearance() const override;

    bool                            add_part(const std::shared_ptr<mob>& part);
    parts_vector_t                  parts() const;
    std::shared_ptr<mob>            body() const;
    bool                            has_parts() const;
    void                            parts_mode(MOB_PARTS_MODE mode);
    MOB_PARTS_MODE                  parts_mode() const;
    uint64_t                        total_exp() const;
    // clang-format on
};

struct mob::listener_t : public virtual fb::game::life::listener_t
{ };

} // namespace fb::game

#endif // !__MOB_H__
