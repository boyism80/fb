#ifndef __MODEL_PREPROCESSOR_H__
#define __MODEL_PREPROCESSOR_H__

#include <fb/model/datetime.h>
#include <fb/lua.h>

#define DECLARE_MAP_INHERIT : public fb::lua::luable

#define DECLARE_MAP_EXTENSION \
                              \
public:                       \
    LUA_PROTOTYPE

#define DECLARE_PROMOTION_CONTAINER_EXTENSION                                                             \
                                                                                                          \
public:                                                                                                   \
    const promotion* operator() (enum_value::CLASS cls, uint8_t promotion) const;                         \
    bool             name2class(std::string_view name, enum_value::CLASS& cls, uint8_t& promotion) const; \
    bool             class2name(enum_value::CLASS cls, uint8_t promotion, std::string& name) const;

#define DECLARE_MOB_CONTAINER_EXTENSION \
                                        \
public:                                 \
    fb::model::mob* name2mob(std::string_view name) const;

#define DECLARE_NPC_CONTAINER_EXTENSION \
                                        \
public:                                 \
    fb::model::npc* name2npc(std::string_view name) const;

#define DECLARE_ITEM_CONTAINER_EXTENSION                                  \
                                                                          \
public:                                                                   \
    fb::model::item*              name2item(std::string_view name) const; \
    std::vector<fb::model::item*> name2item_prefix(std::string_view prefix) const;

#define DECLARE_MAP_CONTAINER_EXTENSION \
                                        \
public:                                 \
    fb::model::map* name2map(std::string_view name) const;

#define DECLARE_SPELL_CONTAINER_EXTENSION \
                                          \
public:                                   \
    fb::model::spell* name2spell(std::string_view name) const;

#endif
