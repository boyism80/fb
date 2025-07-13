#include <fb/model/model.h>
#include <fb/model/datetime.h>

#define DECLARE_PROMOTION_CONTAINER_EXTENSION                                                               \
                                                                                                            \
public:                                                                                                     \
    const promotion* operator() (enum_value::CLASS cls, uint8_t promotion) const;                           \
    bool             name2class(const std::string& name, enum_value::CLASS& cls, uint8_t& promotion) const; \
    bool             class2name(enum_value::CLASS cls, uint8_t promotion, std::string& name) const;

#define DECLARE_MOB_CONTAINER_EXTENSION \
                                        \
public:                                 \
    fb::model::mob* name2mob(const std::string& name) const;

#define DECLARE_NPC_CONTAINER_EXTENSION \
                                        \
public:                                 \
    fb::model::npc* name2npc(const std::string& name) const;

#define DECLARE_ITEM_CONTAINER_EXTENSION \
                                         \
public:                                  \
    fb::model::item* name2item(const std::string& name) const;

#define DECLARE_MAP_CONTAINER_EXTENSION \
                                        \
public:                                 \
    fb::model::map* name2map(const std::string& name) const;

#define DECLARE_SPELL_CONTAINER_EXTENSION \
                                          \
public:                                   \
    fb::model::spell* name2spell(const std::string& name) const;