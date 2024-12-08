#ifndef __GROUP_H__
#define __GROUP_H__

#include <character.h>

namespace fb::game {

/**
 * @brief      This class describes a group.
 */
class group : public fb::thread_switchable
{
private:
    context&                 _context;
    uint32_t                 _id;
    std::string              _master;
    std::vector<std::string> _members;
    std::vector<character*>  _active_members;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  id       The identifier
     */
    group(context& context, uint32_t id);

    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     * @param[in]  id       The identifier
     * @param[in]  master   The master
     * @param[in]  members  The members
     */
    group(context& context, uint32_t id, const std::string& master, const std::vector<std::string>& members);

    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    group(const group&) = delete;

    /**
     * @brief      Constructs a new instance.
     *
     * @param      g     { parameter_description }
     */
    group(group&& g);

    /**
     * @brief      Destroys the object.
     */
    ~group() = default;

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t id() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  name  The name
     */
    void master(const std::string& name);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool inited() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const std::string& master() const;

    /**
     * @brief      { function_description }
     *
     * @param[in]  name  The name
     */
    void enter(const std::string& name);

    /**
     * @brief      { function_description }
     *
     * @param      ch    { parameter_description }
     */
    void enter(fb::game::character& ch);

    /**
     * @brief      { function_description }
     *
     * @param      ch    { parameter_description }
     */
    void leave_active_member(fb::game::character& ch);

    /**
     * @brief      { function_description }
     *
     * @param[in]  name  The name
     */
    void leave(const std::string& name);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<fb::game::character*> active_members() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<std::string> members() const;

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::thread* thread() const override;
};

} // namespace fb::game

#endif // !__GROUP_H__
