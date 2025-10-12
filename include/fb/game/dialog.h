#ifndef __DIALOG_H__
#define __DIALOG_H__

#include <stack>
#include <fb/game/npc.h>

namespace fb::game {
class character;
}

namespace fb::game::dialog {
using item_pair  = std::pair<fb::model::item&, uint32_t>;
using item_pairs = std::vector<item_pair>;

enum class interaction : uint8_t
{
    NORMAL,
    INPUT,
    INPUT_EX,
    MENU,
    LIST,
    SLOT,
    ITEM,
};

class portrait
{
protected:
    portrait() = default;

public:
    virtual ~portrait()             = default;
    portrait(const portrait& right) = delete;
    portrait(portrait&& right)      = delete;

public:
    portrait& operator= (const portrait& right) = delete;
    portrait& operator= (portrait&& right)      = delete;

public:
    virtual void serialize(fb::stream_writer<big_endian>& writer) const = 0;
};

class character_portrait : public portrait
{
public:
    SEX                     sex          = SEX::MAN;
    STATE                   state        = STATE::NORMAL;
    uint16_t                hair         = 0;
    uint8_t                 hair_color   = 0;
    std::optional<uint16_t> weapon       = std::nullopt;
    std::optional<uint8_t>  weapon_color = std::nullopt;
    std::optional<uint8_t>  armor        = std::nullopt;
    std::optional<uint8_t>  armor_color  = std::nullopt;
    std::optional<uint8_t>  shield       = std::nullopt;
    std::optional<uint8_t>  shield_color = std::nullopt;

public:
    character_portrait()  = default;
    ~character_portrait() = default;

public:
    void serialize(fb::stream_writer<big_endian>& writer) const;
};

class object_portrait : public portrait
{
public:
    uint16_t look  = 0;
    uint8_t  color = 0;

public:
    object_portrait()  = default;
    ~object_portrait() = default;

public:
    void serialize(fb::stream_writer<big_endian>& writer) const;
};

class portrait_factory
{
public:
    static std::unique_ptr<portrait> create(fb::model::model& model, const fb::model::object& obj);
    static std::unique_ptr<portrait> create(const fb::game::object& obj);

private:
    portrait_factory() = default;
};

// clang-format off
struct listener_t
{
    virtual void on_dialog(character& me, const fb::model::object& object, const std::string& message, bool button_prev, bool button_next, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, const std::string& message, bool button_prev, bool button_next, uint32_t seq = 0xFFFFFFFD)     = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, const std::string& message, const std::vector<std::string>& menus, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, const std::string& message, const std::vector<std::string>& menus, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, const std::string& message, const std::vector<std::string>& menus, bool button_prev, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, const std::string& message, const std::vector<std::string>& menus, bool button_prev, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, std::unique_ptr<portrait>&& portrait, const std::string& message, const std::vector<std::string>& menus, bool button_prev, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, const std::string& message, const std::vector<uint8_t>& item_slots, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, const std::string& message, const std::vector<uint8_t>& item_slots, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, const std::string& message, const item_pairs& pairs, uint32_t seq = 0xFFFFFFFD, uint16_t pursuit = 0xFFFF) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, const std::string& message, const item_pairs& pairs, uint32_t seq = 0xFFFFFFFD, uint16_t pursuit = 0xFFFF) = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, const std::string& message, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, const std::string& message, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::model::object& obj, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false, uint32_t seq = 0xFFFFFFFD) = 0;
    virtual void on_dialog(character& me, const fb::game::object& obj, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool prev = false, uint32_t seq = 0xFFFFFFFD) = 0;
};
// clang-format on
} // namespace fb::game::dialog
#endif