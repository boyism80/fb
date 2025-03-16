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

/**
 * @brief      This class describes an interaction.
 */
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

/**
 * @brief      { struct_description }
 */
struct preset
{
    SEX                     sex          = SEX::MAN;
    STATE                   state        = STATE::NORMAL;
    uint16_t                face         = 0;
    uint8_t                 hair_color   = 0;
    std::optional<uint16_t> weapon       = std::nullopt;
    std::optional<uint8_t>  weapon_color = std::nullopt;
    std::optional<uint8_t>  armor        = std::nullopt;
    std::optional<uint8_t>  armor_color  = std::nullopt;
    std::optional<uint16_t> shield       = std::nullopt;
    std::optional<uint8_t>  shield_color = std::nullopt;

    /**
     * @brief      Constructs a new instance.
     */
    preset() = default;
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  ch    { parameter_description }
     */
    preset(const character& ch);
};

/**
 * @brief      { struct_description }
 */
struct listener
{
    /**
     * @brief      Called on dialog.
     *
     * @param      me           { parameter_description }
     * @param[in]  object       The object
     * @param[in]  message      The message
     * @param[in]  button_prev  The button previous
     * @param[in]  button_next  The button next
     */
    virtual void on_dialog(character&               me,
                           const fb::model::object& object,
                           const std::string&       message,
                           bool                     button_prev,
                           bool                     button_next,
                           uint32_t                 sequence = 0xFFFFFFFD) = 0;
    /**
     * @brief      Called on dialog.
     *
     * @param      me       { parameter_description }
     * @param[in]  obj      The obj
     * @param[in]  message  The message
     * @param[in]  menus    The menus
     */
    virtual void on_dialog(character&                      me,
                           const fb::model::object&        obj,
                           const std::string&              message,
                           const std::vector<std::string>& menus,
                           uint32_t                        sequence = 0xFFFFFFFD) = 0;

    /**
     * @brief      Called on dialog.
     *
     * @param      me           { parameter_description }
     * @param[in]  obj          The object
     * @param[in]  message      The message
     * @param[in]  menus        The menus
     * @param[in]  button_prev  The button previous
     */
    virtual void on_dialog(character&                      me,
                           const fb::model::object&        obj,
                           const std::string&              message,
                           const std::vector<std::string>& menus,
                           bool                            button_prev,
                           uint32_t                        sequence = 0xFFFFFFFD) = 0;

    /**
     * @brief      Called on dialog.
     *
     * @param      me           { parameter_description }
     * @param[in]  obj          The obj
     * @param[in]  message      The message
     * @param[in]  menus        The menus
     * @param[in]  button_prev  The button previous
     * @param[in]  preset       The preset
     */
    virtual void on_dialog(character&                      me,
                           const fb::model::object&        obj,
                           const std::string&              message,
                           const std::vector<std::string>& menus,
                           bool                            button_prev,
                           const dialog::preset&           preset,
                           uint32_t                        sequence = 0xFFFFFFFD) = 0;
    /**
     * @brief      Called on dialog.
     *
     * @param      me          { parameter_description }
     * @param[in]  obj         The obj
     * @param[in]  message     The message
     * @param[in]  item_slots  The item slots
     */
    virtual void on_dialog(character&                  me,
                           const fb::model::object&    obj,
                           const std::string&          message,
                           const std::vector<uint8_t>& item_slots,
                           uint32_t                    sequence = 0xFFFFFFFD) = 0;
    /**
     * @brief      Called on dialog.
     *
     * @param      me       { parameter_description }
     * @param[in]  obj      The obj
     * @param[in]  message  The message
     * @param[in]  pairs    The pairs
     * @param[in]  pursuit  The pursuit
     */
    virtual void on_dialog(character&               me,
                           const fb::model::object& obj,
                           const std::string&       message,
                           const item_pairs&        pairs,
                           uint16_t                 pursuit  = 0xFFFF,
                           uint32_t                 sequence = 0xFFFFFFFD) = 0;
    /**
     * @brief      Called on dialog.
     *
     * @param      me       { parameter_description }
     * @param[in]  obj      The obj
     * @param[in]  message  The message
     */
    virtual void on_dialog(character&               me,
                           const fb::model::object& obj,
                           const std::string&       message,
                           uint32_t                 sequence = 0xFFFFFFFD) = 0;
    /**
     * @brief      Called on dialog.
     *
     * @param      me       { parameter_description }
     * @param[in]  obj      The obj
     * @param[in]  message  The message
     * @param[in]  top      The top
     * @param[in]  bottom   The bottom
     * @param[in]  maxlen   The maxlen
     * @param[in]  prev     The previous
     */
    virtual void on_dialog(character&               me,
                           const fb::model::object& obj,
                           const std::string&       message,
                           const std::string&       top,
                           const std::string&       bottom,
                           int                      maxlen   = 0xFF,
                           bool                     prev     = false,
                           uint32_t                 sequence = 0xFFFFFFFD) = 0;
};
} // namespace fb::game::dialog
#endif