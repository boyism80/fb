#ifndef __DEAD_LOCK_H__
#define __DEAD_LOCK_H__

#include <fb/mst.h>

namespace fb {

/**
 * @brief      This class describes a dead lock detector.
 */
class dead_lock_detector : public fb::mst<std::string>
{
public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  data    The data
     * @param[in]  parent  The parent
     */
    dead_lock_detector(const std::string& data = "", const dead_lock_detector* parent = nullptr);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    dead_lock_detector(const dead_lock_detector&) = delete;
    /**
     * @brief      Destroys the object.
     */
    ~dead_lock_detector() = default;

public:
    /**
     * @brief      Adds the specified node.
     *
     * @param      node  The node
     */
    void add(dead_lock_detector& node);

public:
    /**
     * @brief      { function_description }
     */
    void assert_circulated_lock() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  node  The node
     */
    void assert_dead_lock(const dead_lock_detector& node) const;

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  val   The value
     *
     * @return     { description_of_the_return_value }
     */
    bool compare(const std::string& val) const;
};

} // namespace fb

#endif