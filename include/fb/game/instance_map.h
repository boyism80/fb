#ifndef __INSTANCE_MAP_H__
#define __INSTANCE_MAP_H__

#include <fb/game/map.h>

namespace fb::game {

class instance_map : public map
{
private:
    std::shared_ptr<fb::game::map> _source;
    const uint32_t                 _slot;
    bool                           _activated  = false;
    bool                           _closing    = false;
    bool                           _destroying = false;

public:
    instance_map(fb::game::server& server, uint32_t id, uint32_t slot, const std::shared_ptr<fb::game::map>& source);
    ~instance_map();

public:
    bool                           is_instance() const override;
    std::shared_ptr<fb::game::map> source() const override;
    uint32_t                       slot() const override;
    bool                           closing() const override;
    void                           on_character_enter() override;
    void                           on_character_leave() override;
    bool                           begin_destroy() override;

private:
    void schedule_destroy();
};

} // namespace fb::game

#endif // !__INSTANCE_MAP_H__
