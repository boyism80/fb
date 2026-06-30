#ifndef __MODEL_PREPROCESSOR_H__
#define __MODEL_PREPROCESSOR_H__

#include <fb/model/substring_matcher.h>

#ifdef DELETE
#undef DELETE
#endif

#ifndef LUA
#define LUA
#endif

#include <fb/lua.h>
#include <fb/model/datetime.h>
#include <string_view>

#define OVERRIDE_OBJECT_TYPE(v)                   \
    enum_value::OBJECT_TYPE what() const override \
    {                                             \
        return (v);                               \
    }

#define BEGIN_PREPROCESSOR            \
    namespace fb::model::enum_value { \
                                      \
    enum class DIRECTION;             \
    }                                 \
                                      \
    namespace fb::model {             \
    class recipe_node;                \
    }                                 \
                                      \
    namespace fb::game {              \
                                      \
    class server;                     \
    class map;                        \
    class character;                  \
    class buffs;                      \
    class objects;                    \
    class item;                       \
    class items;                      \
    class sector;                     \
    class sectors;                    \
    struct listener;                  \
    class appearance;                 \
    }

#define DECLARE_AFTER_TYPE             \
    using point8_t  = point<uint8_t>;  \
    using point16_t = point<uint16_t>; \
    using point32_t = point<uint32_t>; \
    using point64_t = point<uint64_t>; \
                                       \
    using size8_t  = size<uint8_t>;    \
    using size16_t = size<uint16_t>;   \
    using size32_t = size<uint32_t>;   \
    using size64_t = size<uint64_t>;   \
                                       \
    using range8_t  = range<uint8_t>;  \
    using range16_t = range<uint16_t>; \
    using range32_t = range<uint32_t>; \
    using range64_t = range<uint64_t>;

#define DECLARE_POINT_EXTENSION                                                        \
                                                                                       \
public:                                                                                \
    struct point<T>& move(fb::model::enum_value::DIRECTION direction, T step = 1);     \
    struct point<T>& forward(fb::model::enum_value::DIRECTION direction, T step = 1);  \
    struct point<T>& backward(fb::model::enum_value::DIRECTION direction, T step = 1); \
    struct point<T>& left(fb::model::enum_value::DIRECTION direction, T step = 1);     \
    struct point<T>& right(fb::model::enum_value::DIRECTION direction, T step = 1);

#define DECLARE_SIZE_EXTENSION

#define DECLARE_SPELL_INHERIT  : public fb::lua::luable

#define DECLARE_ACHIEVEMENT_INHERIT : public fb::lua::luable

#define DECLARE_OBJECT_EXTENSION                                                    \
                                                                                    \
public:                                                                             \
    LUA_PROTOTYPE                                                                   \
                                                                                    \
public:                                                                             \
    template <typename T, typename... Args>                                         \
    std::shared_ptr<T> make(fb::game::server& server, Args&&... args) const;        \
                                                                                    \
protected:                                                                          \
    uint8_t dialog_look_type() const;                                               \
                                                                                    \
public:                                                                             \
    virtual enum_value::OBJECT_TYPE               what() const;                     \
    virtual std::shared_ptr<fb::game::appearance> create_appearance() const;        \
    bool                                          operator== (const object&) const; \
    bool                                          operator!= (const object&) const;

#define DECLARE_LIFE_EXTENSION \
                               \
public:                        \
    LUA_PROTOTYPE

#define DECLARE_ITEM_EXTENSION                                                                       \
                                                                                                     \
public:                                                                                              \
    LUA_PROTOTYPE                                                                                    \
                                                                                                     \
public:                                                                                              \
    OVERRIDE_OBJECT_TYPE(enum_value::OBJECT_TYPE::ITEM)                                              \
                                                                                                     \
public:                                                                                              \
    virtual enum_value::ITEM_ATTRIBUTE attr() const;                                                 \
    bool                               attr(enum_value::ITEM_ATTRIBUTE flag) const;                  \
    std::optional<fb::model::datetime> expire_time(const fb::model::datetime& now) const;            \
                                                                                                     \
public:                                                                                              \
    virtual std::shared_ptr<fb::game::item> make(fb::game::server&                  server,          \
                                                 uint16_t                           count       = 1, \
                                                 std::optional<fb::model::datetime> expire_time = std::nullopt) const;

#define DECLARE_CASH_EXTENSION                                                                          \
                                                                                                        \
public:                                                                                                 \
    std::shared_ptr<fb::game::item> make(fb::game::server&                  server,                     \
                                         uint16_t                           count       = 1,            \
                                         std::optional<fb::model::datetime> expire_time = std::nullopt) \
        const override              final;                                                              \
                                                                                                        \
    virtual enum_value::ITEM_ATTRIBUTE attr() const                                                     \
    {                                                                                                   \
        return enum_value::ITEM_ATTRIBUTE::CASH;                                                        \
    }

#define DECLARE_CONSUME_EXTENSION                                                                       \
                                                                                                        \
public:                                                                                                 \
    std::shared_ptr<fb::game::item> make(fb::game::server&                  server,                     \
                                         uint16_t                           count       = 1,            \
                                         std::optional<fb::model::datetime> expire_time = std::nullopt) \
        const override              final;                                                              \
                                                                                                        \
    virtual enum_value::ITEM_ATTRIBUTE attr() const                                                     \
    {                                                                                                   \
        auto attr = enum_value::ITEM_ATTRIBUTE::CONSUME;                                                \
        if (this->capacity > 1)                                                                         \
            attr = enum_value::ITEM_ATTRIBUTE(attr | enum_value::ITEM_ATTRIBUTE::BUNDLE);               \
                                                                                                        \
        return attr;                                                                                    \
    }

#define DECLARE_PACK_EXTENSION                                                                          \
                                                                                                        \
public:                                                                                                 \
    std::shared_ptr<fb::game::item> make(fb::game::server&                  server,                     \
                                         uint16_t                           count       = 1,            \
                                         std::optional<fb::model::datetime> expire_time = std::nullopt) \
        const override              final;                                                              \
                                                                                                        \
    virtual enum_value::ITEM_ATTRIBUTE attr() const                                                     \
    {                                                                                                   \
        return enum_value::ITEM_ATTRIBUTE::PACK;                                                        \
    }

#define DECLARE_EQUIPMENT_EXTENSION                   \
                                                      \
public:                                               \
    LUA_PROTOTYPE                                     \
                                                      \
public:                                               \
    virtual enum_value::ITEM_ATTRIBUTE attr() const   \
    {                                                 \
        return enum_value::ITEM_ATTRIBUTE::EQUIPMENT; \
    }

#define DECLARE_WEAPON_EXTENSION                                                                        \
                                                                                                        \
public:                                                                                                 \
    LUA_PROTOTYPE                                                                                       \
                                                                                                        \
public:                                                                                                 \
    std::shared_ptr<fb::game::item> make(fb::game::server&                  server,                     \
                                         uint16_t                           count       = 1,            \
                                         std::optional<fb::model::datetime> expire_time = std::nullopt) \
        const override              final;                                                              \
                                                                                                        \
    virtual enum_value::ITEM_ATTRIBUTE attr() const                                                     \
    {                                                                                                   \
        return enum_value::ITEM_ATTRIBUTE::WEAPON;                                                      \
    }                                                                                                   \
    enum_value::WEAPON_TYPE weapon_type() const                                                         \
    {                                                                                                   \
        switch (this->dress / 10000)                                                                    \
        {                                                                                               \
        case 0:                                                                                         \
            return enum_value::WEAPON_TYPE::NORMAL;                                                     \
                                                                                                        \
        case 1:                                                                                         \
            return enum_value::WEAPON_TYPE::SPEAR;                                                      \
                                                                                                        \
        case 2:                                                                                         \
            return enum_value::WEAPON_TYPE::BOW;                                                        \
                                                                                                        \
        case 3:                                                                                         \
            return enum_value::WEAPON_TYPE::FAN;                                                        \
                                                                                                        \
        default:                                                                                        \
            return enum_value::WEAPON_TYPE::UNKNOWN;                                                    \
        }                                                                                               \
    }

#define DECLARE_ARMOR_EXTENSION                                                                         \
                                                                                                        \
public:                                                                                                 \
    std::shared_ptr<fb::game::item> make(fb::game::server&                  server,                     \
                                         uint16_t                           count       = 1,            \
                                         std::optional<fb::model::datetime> expire_time = std::nullopt) \
        const override              final;                                                              \
                                                                                                        \
    virtual enum_value::ITEM_ATTRIBUTE attr() const                                                     \
    {                                                                                                   \
        return enum_value::ITEM_ATTRIBUTE::ARMOR;                                                       \
    }

#define DECLARE_HELMET_EXTENSION                                                                        \
                                                                                                        \
public:                                                                                                 \
    std::shared_ptr<fb::game::item> make(fb::game::server&                  server,                     \
                                         uint16_t                           count       = 1,            \
                                         std::optional<fb::model::datetime> expire_time = std::nullopt) \
        const override              final;                                                              \
                                                                                                        \
    virtual enum_value::ITEM_ATTRIBUTE attr() const                                                     \
    {                                                                                                   \
        return enum_value::ITEM_ATTRIBUTE::HELMET;                                                      \
    }

#define DECLARE_SHIELD_EXTENSION                                                                        \
                                                                                                        \
public:                                                                                                 \
    std::shared_ptr<fb::game::item> make(fb::game::server&                  server,                     \
                                         uint16_t                           count       = 1,            \
                                         std::optional<fb::model::datetime> expire_time = std::nullopt) \
        const override              final;                                                              \
                                                                                                        \
    virtual enum_value::ITEM_ATTRIBUTE attr() const                                                     \
    {                                                                                                   \
        return enum_value::ITEM_ATTRIBUTE::SHIELD;                                                      \
    }

#define DECLARE_RING_EXTENSION                                                                          \
                                                                                                        \
public:                                                                                                 \
    std::shared_ptr<fb::game::item> make(fb::game::server&                  server,                     \
                                         uint16_t                           count       = 1,            \
                                         std::optional<fb::model::datetime> expire_time = std::nullopt) \
        const override              final;                                                              \
                                                                                                        \
    virtual enum_value::ITEM_ATTRIBUTE attr() const                                                     \
    {                                                                                                   \
        return enum_value::ITEM_ATTRIBUTE::RING;                                                        \
    }

#define DECLARE_AUXILIARY_EXTENSION                                                                     \
                                                                                                        \
public:                                                                                                 \
    std::shared_ptr<fb::game::item> make(fb::game::server&                  server,                     \
                                         uint16_t                           count       = 1,            \
                                         std::optional<fb::model::datetime> expire_time = std::nullopt) \
        const override              final;                                                              \
                                                                                                        \
    virtual enum_value::ITEM_ATTRIBUTE attr() const                                                     \
    {                                                                                                   \
        return enum_value::ITEM_ATTRIBUTE::AUXILIARY;                                                   \
    }

#define DECLARE_NPC_EXTENSION                          \
                                                       \
public:                                                \
    LUA_PROTOTYPE                                      \
                                                       \
public:                                                \
    OVERRIDE_OBJECT_TYPE(enum_value::OBJECT_TYPE::NPC) \
    std::shared_ptr<fb::game::appearance> create_appearance() const override;

#define DECLARE_MOB_EXTENSION                          \
                                                       \
public:                                                \
    LUA_PROTOTYPE                                      \
                                                       \
public:                                                \
    OVERRIDE_OBJECT_TYPE(enum_value::OBJECT_TYPE::MOB) \
    std::shared_ptr<fb::game::appearance> create_appearance() const override;

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

#define DECLARE_MAP_INHERIT : public fb::lua::luable

#define DECLARE_MAP_EXTENSION \
                              \
public:                       \
    LUA_PROTOTYPE             \
                              \
public:                       \
    std::optional<point16_t> spawn_position() const;

#define DECLARE_MAP_CONTAINER_EXTENSION \
                                        \
public:                                 \
    fb::model::map* name2map(std::string_view name) const;

#define DECLARE_DOOR_EXTENSION                                                             \
                                                                                           \
public:                                                                                    \
    const uint16_t width;                                                                  \
                                                                                           \
public:                                                                                    \
    bool matched(const fb::game::map& map, const point16_t& position, bool is_open) const; \
    bool matched(const fb::game::map& map, const point16_t& position, bool* opened) const;

#define DECLARE_DOOR_INITIALIZER , width(static_cast<uint16_t>(this->pairs.size()))

#define DECLARE_SPELL_CONTAINER_EXTENSION \
                                          \
public:                                   \
    fb::model::spell* name2spell(std::string_view name) const;

#define DECLARE_SPELL_EXTENSION \
                                \
public:                         \
    struct listener;            \
                                \
public:                         \
    LUA_PROTOTYPE

#define DECLARE_PROMOTION_CONTAINER_EXTENSION                                                             \
                                                                                                          \
public:                                                                                                   \
    const promotion* operator() (enum_value::CLASS cls, uint8_t promotion) const;                         \
    bool             name2class(std::string_view name, enum_value::CLASS& cls, uint8_t& promotion) const; \
    bool             class2name(enum_value::CLASS cls, uint8_t promotion, std::string& name) const;

#define DECLARE_SELL_CONTAINER_EXTENSION                                              \
                                                                                      \
public:                                                                               \
    const fb::model::sell* find(uint32_t pursuit, const fb::model::item& item) const; \
    const fb::model::sell* find(const fb::model::npc& npc, const fb::model::item& item) const;

#define DECLARE_BUY_CONTAINER_EXTENSION                                              \
                                                                                     \
public:                                                                              \
    const fb::model::buy* find(uint32_t pursuit, const fb::model::item& item) const; \
    const fb::model::buy* find(const fb::model::npc& npc, const fb::model::item& item) const;

#define DECLARE_BLOCKED_WORD_CONTAINER_CUSTOM_CONSTRUCTOR \
    __blocked_word();                                     \
    __blocked_word(const __blocked_word&) = delete;       \
    ~__blocked_word() = default;

#define DECLARE_BLOCKED_WORD_CONTAINER_EXTENSION \
                                                 \
public:                                          \
    std::string filter(std::string_view message) const; \
                                                   \
private:                                           \
    fb::model::substring_matcher _matcher;

#define DECLARE_RECIPE_EXTENSION                       \
                                                       \
public:                                                \
    recipe(const std::vector<fb::model::dsl>& source,  \
           const std::vector<fb::model::dsl>& success, \
           const std::vector<fb::model::dsl>& failed,  \
           double                             percent) :                           \
        source(source),                                \
        success(success),                              \
        failed(failed),                                \
        percent(percent)                               \
    { }

#define DECLARE_RECIPE_CONTAINER_CUSTOM_CONSTRUCTOR \
    __recipe();                                     \
    __recipe(const __recipe&) = delete;             \
    ~__recipe();

#define DECLARE_RECIPE_CONTAINER_EXTENSION         \
                                                   \
private:                                           \
    std::unique_ptr<fb::model::recipe_node> _root; \
                                                   \
public:                                            \
    const fb::model::recipe* find(const std::vector<fb::model::dsl::item>& dsl) const;

#define DECLARE_ABILITY_CONTAINER_CUSTOM_CONSTRUCTOR \
    __ability();                                     \
    __ability(const __ability&) = delete;            \
    ~__ability()                = default;

#define DECLARE_ABILITY_CONTAINER_EXTENSION                                                               \
                                                                                                          \
private:                                                                                                  \
    std::unordered_map<fb::model::enum_value::CLASS, std::unordered_map<uint8_t, uint32_t>> _stacked_exp; \
                                                                                                          \
    void build_stacked_exp();                                                                             \
                                                                                                          \
public:                                                                                                   \
    void load() override                                                                                  \
    {                                                                                                     \
        fb::model::kv_container<fb::model::enum_value::CLASS,                                             \
                                fb::model::kv_container<uint8_t, fb::model::ability>>::load();            \
        this->build_stacked_exp();                                                                        \
    }                                                                                                     \
                                                                                                          \
    uint32_t stacked_exp(fb::model::enum_value::CLASS cls, uint8_t level) const;

#define DECLARE_AFTER_ENUM                                                                  \
    template <typename T>                                                                   \
    struct point<T>& point<T>::move(fb::model::enum_value::DIRECTION direction, T step)     \
    {                                                                                       \
        switch (direction)                                                                  \
        {                                                                                   \
        case fb::model::enum_value::DIRECTION::TOP:                                         \
            this->y -= step;                                                                \
            break;                                                                          \
                                                                                            \
        case fb::model::enum_value::DIRECTION::BOTTOM:                                      \
            this->y += step;                                                                \
            break;                                                                          \
                                                                                            \
        case fb::model::enum_value::DIRECTION::LEFT:                                        \
            this->x -= step;                                                                \
            break;                                                                          \
                                                                                            \
        case fb::model::enum_value::DIRECTION::RIGHT:                                       \
            this->x += step;                                                                \
            break;                                                                          \
        }                                                                                   \
                                                                                            \
        return *this;                                                                       \
    }                                                                                       \
                                                                                            \
    template <typename T>                                                                   \
    struct point<T>& point<T>::forward(fb::model::enum_value::DIRECTION direction, T step)  \
    {                                                                                       \
        return this->move(direction, step);                                                 \
    }                                                                                       \
                                                                                            \
    template <typename T>                                                                   \
    struct point<T>& point<T>::backward(fb::model::enum_value::DIRECTION direction, T step) \
    {                                                                                       \
        auto newdir = direction;                                                            \
        switch (direction)                                                                  \
        {                                                                                   \
        case fb::model::enum_value::DIRECTION::TOP:                                         \
            newdir = fb::model::enum_value::DIRECTION::BOTTOM;                              \
            break;                                                                          \
                                                                                            \
        case fb::model::enum_value::DIRECTION::BOTTOM:                                      \
            newdir = fb::model::enum_value::DIRECTION::TOP;                                 \
            break;                                                                          \
                                                                                            \
        case fb::model::enum_value::DIRECTION::LEFT:                                        \
            newdir = fb::model::enum_value::DIRECTION::RIGHT;                               \
            break;                                                                          \
                                                                                            \
        case fb::model::enum_value::DIRECTION::RIGHT:                                       \
            newdir = fb::model::enum_value::DIRECTION::LEFT;                                \
            break;                                                                          \
        }                                                                                   \
                                                                                            \
        return this->move(newdir, step);                                                    \
    }                                                                                       \
                                                                                            \
    template <typename T>                                                                   \
    struct point<T>& point<T>::left(fb::model::enum_value::DIRECTION direction, T step)     \
    {                                                                                       \
        auto newdir = direction;                                                            \
        switch (direction)                                                                  \
        {                                                                                   \
        case fb::model::enum_value::DIRECTION::TOP:                                         \
            newdir = fb::model::enum_value::DIRECTION::LEFT;                                \
            break;                                                                          \
                                                                                            \
        case fb::model::enum_value::DIRECTION::BOTTOM:                                      \
            newdir = fb::model::enum_value::DIRECTION::RIGHT;                               \
            break;                                                                          \
                                                                                            \
        case fb::model::enum_value::DIRECTION::LEFT:                                        \
            newdir = fb::model::enum_value::DIRECTION::BOTTOM;                              \
            break;                                                                          \
                                                                                            \
        case fb::model::enum_value::DIRECTION::RIGHT:                                       \
            newdir = fb::model::enum_value::DIRECTION::TOP;                                 \
            break;                                                                          \
        }                                                                                   \
                                                                                            \
        return this->move(newdir, step);                                                    \
    }                                                                                       \
                                                                                            \
    template <typename T>                                                                   \
    struct point<T>& point<T>::right(fb::model::enum_value::DIRECTION direction, T step)    \
    {                                                                                       \
        auto newdir = direction;                                                            \
        switch (direction)                                                                  \
        {                                                                                   \
        case fb::model::enum_value::DIRECTION::TOP:                                         \
            newdir = fb::model::enum_value::DIRECTION::RIGHT;                               \
            break;                                                                          \
                                                                                            \
        case fb::model::enum_value::DIRECTION::BOTTOM:                                      \
            newdir = fb::model::enum_value::DIRECTION::LEFT;                                \
            break;                                                                          \
                                                                                            \
        case fb::model::enum_value::DIRECTION::LEFT:                                        \
            newdir = fb::model::enum_value::DIRECTION::TOP;                                 \
            break;                                                                          \
                                                                                            \
        case fb::model::enum_value::DIRECTION::RIGHT:                                       \
            newdir = fb::model::enum_value::DIRECTION::BOTTOM;                              \
            break;                                                                          \
        }                                                                                   \
                                                                                            \
        return this->move(newdir, step);                                                    \
    }

#define DECLARE_OBJECT_INHERIT  : public fb::lua::luable

#define DECLARE_ACHIEVEMENT_EXTENSION \
                                      \
public:                               \
    LUA_PROTOTYPE

#define DECLARE_QUEST_INHERIT : public fb::lua::luable

#define DECLARE_QUEST_EXTENSION \
                                \
public:                         \
    LUA_PROTOTYPE

#define DECLARE_SCHEDULE_EXTENSION \
                                   \
public:                            \
    std::optional<fb::model::datetime> next_execution(const fb::model::datetime& now) const;

#endif