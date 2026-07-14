#ifndef __MOB_H__
#define __MOB_H__

#include <fb/game/life.h>
#include <fb/game/item.h>
#include <fb/game/appearance.h>
#include <async/task.h>

namespace fb::game {

using namespace std::chrono_literals;

class character;
class ai;

class map;

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

    uint32_t                        map_id() const;
    void                            decrease();
    [[nodiscard]] async::task<void> spawn(std::thread::id thread_id);
    void                            force_spawn(std::thread::id thread_id);
};

class mob : public life
{
public:
    using model_type     = fb::model::mob;
    using item_vector_t  = std::vector<std::shared_ptr<fb::game::item>>;
    using appearance_ptr = std::shared_ptr<fb::game::appearance>;

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
    fb::model::datetime _action_time;
    std::weak_ptr<life> _target;
    std::weak_ptr<life> _oblivion;
    rezen*              _rezen         = nullptr;
    lua::context*       _attack_thread = nullptr;
    item_vector_t       _items;
    bool                _hidden = false;
    std::unique_ptr<ai> _ai_strategy;

public:
    const std::weak_ptr<character> owner;
    mob::listener_t&               listener;
    fb::game::mob_stat             stat;

public:
    mob(fb::game::server& server, const fb::model::mob& model, const initial_params& params);
    ~mob();

private:
    std::weak_ptr<fb::game::life>   find_target();
    [[nodiscard]] async::task<bool> call_script();
    void                            AI(const fb::model::datetime& now);
    static bool is_cardinally_adjacent(const fb::model::point16_t& a, const fb::model::point16_t& b);
    static bool is_cover_barrier_cell(const fb::model::point16_t& cell, const fb::model::point16_t& cover_center);
    bool        cover_blocks_move(const fb::game::map&        map,
                                  const fb::model::point16_t& from,
                                  const fb::model::point16_t& to) const;

public:
    bool near_target(const std::shared_ptr<fb::game::life>& target, DIRECTION& out) const;
    bool move_step(const fb::model::point16_t& position);

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
    uint32_t                        normal_attack_damage(MOB_SIZE size) const override final;
    void                            kill(DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) override final;
    async::task<void>               damage_to(const damage_list& targets, const damage_opts& opts = {}) override final;
    async::task<void>               drop_items();
    void                            assert_thread() const override final;
    bool                            move(DIRECTION direction) override final;
    const item_vector_t&            items() const;
    bool                            push_item(std::shared_ptr<fb::game::item> item);
    bool                            hidden(const fb::game::object& target) const override final;
    void                            hidden(bool enabled);
    appearance_ptr                  appearance() const override;
    // clang-format on
};

struct mob::listener_t : public virtual fb::game::life::listener_t
{ };

} // namespace fb::game

#endif // !__MOB_H__
