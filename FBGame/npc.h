#ifndef __NPC_H__
#define __NPC_H__

#include "item.h"
#include "map.h"

namespace fb { namespace game {

class npc : public object
{
public:
    class core : public object::core
    {
        friend class npc;

    public:
        core(const std::string& name, uint16_t look, uint8_t color = 0);
        ~core();

    public:
        using object::core::make_dialog_stream;

    public:
        object::types       type() const { return object::types::NPC; }
        object*             make() const;

    public:
        fb::ostream         make_dialog_stream(const std::string& message, const std::vector<fb::game::item::core*>& items, fb::game::map* map = nullptr) const;
    };

private:
    std::string             _script;

public:
    using object::make_dialog_stream;

public:
    npc(const core* core);
    npc(const npc& right);
    ~npc();

public:
    const std::string&      script() const;
    void                    script(const std::string& value);

public:
    npc*                    make();
    object::types           type() const;

public:
    fb::ostream             make_dialog_stream(const std::string& message, const std::vector<fb::game::item::core*>& items) const;
};


} }

#endif // !__NPC_H__
