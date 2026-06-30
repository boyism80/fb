#ifndef __ITEM_BASE_H__
#define __ITEM_BASE_H__

#include <fb/game/object.h>
#include <fb/game/appearance.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/model/datetime.h>
#include <async/task.h>
#include <optional>
#include <memory>

namespace fb::game {

// Forward declarations
class character;
class items;

class item : public object
{
public:
    using model_type = fb::model::item;

public:
    using object::map;

public:
    friend class items;

public:
    struct listener_t;
    struct conditions;

public:
    LUA_PROTOTYPE

public:
    struct initial_params : fb::game::object::initial_params
    {
    public:
        uint16_t                           count       = 1;
        std::optional<fb::model::datetime> expire_time;
    };

public:
    static const conditions DEFAULT_CONDITION;

public:
    using nullable_time = std::optional<fb::model::datetime>;
    using map_ptr_t     = std::shared_ptr<fb::game::map>;

protected:
    uint16_t                _count        = 0;
    uint16_t                _trade_count  = 0;
    items*                  _container    = nullptr;
    std::optional<uint32_t> _death_uid    = std::nullopt;
    nullable_time           _dropped_time = std::nullopt;

public:
    const std::optional<fb::model::datetime> expire_time = std::nullopt;

public:
    listener_t& listener;

public:
    item(fb::game::server&      server,
         const fb::model::item& model,
         const initial_params&  params = initial_params{.count = 1});
    item(const item& right);
    virtual ~item();

public:
    std::shared_ptr<fb::game::character>    owner() const;
    uint16_t                                fill(uint16_t count);
    uint16_t                                free_space() const;
    uint16_t                                count() const;
    void                                    count(uint16_t value);
    uint16_t                                trade_count() const;
    void                                    trade_count(uint16_t value);
    const nullable_time&                    dropped_time() const;
    void                                    death_uid(std::optional<uint32_t> cid);
    std::optional<uint32_t>                 death_uid() const;
    fb::thread*                             thread() const override final;
    void                                    assert_thread() const override;
    virtual std::string                     tip_message() const;
    virtual std::optional<uint32_t>         durability() const;
    virtual void                            durability(uint32_t value);
    virtual std::string                     inven_name() const;
    virtual std::string                     trade_name() const;
    virtual bool                            empty() const;
    virtual bool                            active();
    virtual std::shared_ptr<fb::game::item> split(uint16_t count = 1);
    virtual void                            merge(std::shared_ptr<fb::game::item> item);
    virtual async::task<bool>               map(map_ptr_t                           map,
                                                std::optional<fb::model::point16_t> position = std::nullopt,
                                                map_options                         options  = {}) override;
    std::shared_ptr<fb::game::appearance>   appearance() const override;
    bool                                    expired() const;

public:
    void container(fb::game::items* container);

public:
    virtual fb::protocol::internal::Item to_protocol(EQUIPMENT_PARTS parts = EQUIPMENT_PARTS::UNKNOWN) const;
};

} // namespace fb::game

#endif // !__ITEM_BASE_H__
