#ifndef __DIALOG_H__
#define __DIALOG_H__

#include <stdint.h>
#include <string>
#include <stack>
#include <npc.h>
#include <item.h>
#include <model.h>

namespace fb { namespace game {

/**
 * @brief      This class describes a character.
 */
class character;

/**
 * @brief      This class describes a dialog.
 */
class dialog
{
public:
    /**
     * @brief      This class describes an inactive error.
     */
    class inactive_error;

public:
    using item_pair  = std::pair<fb::model::item&, uint32_t>;
    using item_pairs = std::vector<item_pair>;

public:
    /**
     * @brief      This class describes an interaction.
     */
    enum class interaction : uint8_t
    {
        NORMAL,
        INPUT,
        INPUT_EX,
        MENU,
        SLOT,
        ITEM,
    };

public:
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
         * @param[in]  interaction  The interaction
         */
        virtual void on_dialog(character&               me,
                               const fb::model::object& object,
                               const std::string&       message,
                               bool                     button_prev,
                               bool                     button_next,
                               interaction              interaction = interaction::NORMAL) = 0;
        /**
         * @brief      Called on dialog.
         *
         * @param      me           { parameter_description }
         * @param[in]  npc          The npc
         * @param[in]  message      The message
         * @param[in]  menus        The menus
         * @param[in]  interaction  The interaction
         */
        virtual void on_dialog(character&                      me,
                               const fb::model::npc&           npc,
                               const std::string&              message,
                               const std::vector<std::string>& menus,
                               interaction                     interaction = interaction::NORMAL) = 0;
        /**
         * @brief      Called on dialog.
         *
         * @param      me           { parameter_description }
         * @param[in]  npc          The npc
         * @param[in]  message      The message
         * @param[in]  item_slots   The item slots
         * @param[in]  interaction  The interaction
         */
        virtual void on_dialog(character&                  me,
                               const fb::model::npc&       npc,
                               const std::string&          message,
                               const std::vector<uint8_t>& item_slots,
                               interaction                 interaction = interaction::NORMAL) = 0;
        /**
         * @brief      Called on dialog.
         *
         * @param      me           { parameter_description }
         * @param[in]  npc          The npc
         * @param[in]  message      The message
         * @param[in]  pairs        The pairs
         * @param[in]  pursuit      The pursuit
         * @param[in]  interaction  The interaction
         */
        virtual void on_dialog(character&            me,
                               const fb::model::npc& npc,
                               const std::string&    message,
                               const item_pairs&     pairs,
                               uint16_t              pursuit     = 0xFFFF,
                               interaction           interaction = interaction::NORMAL) = 0;
        /**
         * @brief      Called on dialog.
         *
         * @param      me           { parameter_description }
         * @param[in]  npc          The npc
         * @param[in]  message      The message
         * @param[in]  interaction  The interaction
         */
        virtual void on_dialog(character&            me,
                               const fb::model::npc& npc,
                               const std::string&    message,
                               interaction           interaction = interaction::NORMAL) = 0;
        /**
         * @brief      Called on dialog.
         *
         * @param      me           { parameter_description }
         * @param[in]  npc          The npc
         * @param[in]  message      The message
         * @param[in]  top          The top
         * @param[in]  bottom       The bottom
         * @param[in]  maxlen       The maxlen
         * @param[in]  prev         The previous
         * @param[in]  interaction  The interaction
         */
        virtual void on_dialog(character&            me,
                               const fb::model::npc& npc,
                               const std::string&    message,
                               const std::string&    top,
                               const std::string&    bottom,
                               int                   maxlen      = 0xFF,
                               bool                  prev        = false,
                               interaction           interaction = interaction::NORMAL) = 0;
    };

public:
    /**
     * @brief      { struct_description }
     */
    struct listener;

public:
    using lua_stack = std::stack<lua::context*>;

private:
    character& _owner;
    lua_stack  _scripts;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      owner  The owner
     */
    dialog(character& owner);
    /**
     * @brief      Destroys the object.
     */
    ~dialog();

private:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    lua::context* current() const;

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  fmt   The format
     * @param      args  The arguments
     *
     * @tparam     Args  { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <class... Args>
    dialog& from(const std::string& fmt, Args&&... args);
    /**
     * @brief      { function_description }
     *
     * @param[in]  fmt   The format
     * @param      args  The arguments
     *
     * @tparam     Args  { description }
     *
     * @return     { description_of_the_return_value }
     */
    template <class... Args>
    dialog& func(const std::string& fmt, Args&&... args);
    /**
     * @brief      { function_description }
     *
     * @param[in]  argc  The count of arguments
     *
     * @return     { description_of_the_return_value }
     */
    dialog& resume(int argc);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    dialog& release();
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool active() const;

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    dialog& new_table();
    /**
     * @brief      Sets the table.
     *
     * @return     { description_of_the_return_value }
     */
    dialog& set_table();
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    dialog& pushstring(const std::string& value);
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    dialog& pushinteger(lua_Integer value);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    dialog& pushnil();
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     *
     * @return     { description_of_the_return_value }
     */
    dialog& pushboolean(bool value);
    /**
     * @brief      { function_description }
     *
     * @param[in]  object  The object
     *
     * @return     { description_of_the_return_value }
     */
    dialog& pushobject(const lua::luable* object);
    /**
     * @brief      { function_description }
     *
     * @param[in]  object  The object
     *
     * @return     { description_of_the_return_value }
     */
    dialog& pushobject(const lua::luable& object);

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  object       The object
     * @param[in]  message      The message
     * @param[in]  button_prev  The button previous
     * @param[in]  button_next  The button next
     * @param[in]  interaction  The interaction
     */
    void show(const fb::model::object& object,
              const std::string&       message,
              bool                     button_prev = false,
              bool                     button_next = true,
              interaction              interaction = interaction::NORMAL);
    /**
     * @brief      { function_description }
     *
     * @param[in]  npc          The npc
     * @param[in]  message      The message
     * @param[in]  menus        The menus
     * @param[in]  interaction  The interaction
     */
    void show(const fb::model::npc&           npc,
              const std::string&              message,
              const std::vector<std::string>& menus,
              interaction                     interaction = interaction::MENU);
    /**
     * @brief      { function_description }
     *
     * @param[in]  npc          The npc
     * @param[in]  message      The message
     * @param[in]  item_slots   The item slots
     * @param[in]  interaction  The interaction
     */
    void show(const fb::model::npc&       npc,
              const std::string&          message,
              const std::vector<uint8_t>& item_slots,
              interaction                 interaction = interaction::SLOT);
    /**
     * @brief      { function_description }
     *
     * @param[in]  npc          The npc
     * @param[in]  message      The message
     * @param[in]  pairs        The pairs
     * @param[in]  pursuit      The pursuit
     * @param[in]  interaction  The interaction
     */
    void show(const fb::model::npc& npc,
              const std::string&    message,
              const item_pairs&     pairs,
              uint16_t              pursuit     = 0xFFFF,
              interaction           interaction = interaction::ITEM);
    /**
     * @brief      { function_description }
     *
     * @param[in]  npc          The npc
     * @param[in]  message      The message
     * @param[in]  interaction  The interaction
     */
    void input(const fb::model::npc& npc, const std::string& message, interaction interaction = interaction::INPUT);
    /**
     * @brief      { function_description }
     *
     * @param[in]  npc          The npc
     * @param[in]  message      The message
     * @param[in]  top          The top
     * @param[in]  bottom       The bottom
     * @param[in]  maxlen       The maxlen
     * @param[in]  prev         The previous
     * @param[in]  interaction  The interaction
     */
    void input(const fb::model::npc& npc,
               const std::string&    message,
               const std::string&    top,
               const std::string&    bottom,
               int                   maxlen      = 0xFF,
               bool                  prev        = false,
               interaction           interaction = interaction::INPUT_EX);
};

/**
 * @brief      This class describes an inactive error.
 */
class dialog::inactive_error : public std::runtime_error
{
public:
    /**
     * @brief      Constructs a new instance.
     */
    inactive_error() :
        runtime_error("current lua context is empty")
    { }
    ~inactive_error() = default;
};

}} // namespace fb::game

/**
 * @brief      { function_description }
 *
 * @param[in]  fmt   The format
 * @param      args  The arguments
 *
 * @tparam     Args  { description }
 *
 * @return     { description_of_the_return_value }
 */
template <class... Args>
fb::game::dialog& fb::game::dialog::from(const std::string& fmt, Args&&... args)
{
    auto buffer = std::vformat(fmt, std::make_format_args(args...));
    auto ctx    = fb::game::lua::get();
    ctx->from(buffer.c_str());
    this->_scripts.push(ctx);
    return *this;
}

/**
 * @brief      { function_description }
 *
 * @param[in]  fmt   The format
 * @param      args  The arguments
 *
 * @tparam     Args  { description }
 *
 * @return     { description_of_the_return_value }
 */
template <class... Args>
fb::game::dialog& fb::game::dialog::func(const std::string& fmt, Args&&... args)
{
    auto ctx = this->current();
    if (ctx == nullptr)
        throw inactive_error();

    auto buffer = std::vformat(fmt, std::make_format_args(args...));
    ctx->func(buffer.c_str());
    return *this;
}

#endif