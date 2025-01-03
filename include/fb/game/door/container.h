#ifndef __DOOR_CONTAINER_H__
#define __DOOR_CONTAINER_H__

#include <fb/game/door.h>

namespace fb::game {

/**
 * @brief      This class describes door_container.
 */
class door_container : private std::unordered_map<uint64_t, std::unique_ptr<door>>
{
public:
    /**
     * @brief      This class describes an iterator.
     */
    class iterator;
    /**
     * @brief      This class describes a constant iterator.
     */
    class const_iterator;

    using base_iterator       = std::unordered_map<uint64_t, std::unique_ptr<door>>::iterator;
    using const_base_iterator = std::unordered_map<uint64_t, std::unique_ptr<door>>::const_iterator;

    friend class iterator;
    friend class const_iterator;

public:
    const fb::game::map& map;

public:
    using unordered_map<uint64_t, std::unique_ptr<door>>::begin;
    using unordered_map<uint64_t, std::unique_ptr<door>>::end;
    using unordered_map<uint64_t, std::unique_ptr<door>>::cbegin;
    using unordered_map<uint64_t, std::unique_ptr<door>>::cend;
    using unordered_map<uint64_t, std::unique_ptr<door>>::size;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  map   The map
     */
    door_container(const fb::game::map& map);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    door_container(const door_container&) = delete;
    /**
     * @brief      Destroys the object.
     */
    ~door_container();

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    iterator begin();
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    iterator end();
    /**
     * @brief      { function_description }
     *
     * @return     The constant iterator.
     */
    const_iterator begin() const;
    /**
     * @brief      { function_description }
     *
     * @return     The constant iterator.
     */
    const_iterator end() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  position  The position
     * @param[in]  pivot     The pivot
     * @param[in]  model     The model
     * @param[in]  opened    Indicates if opened
     */
    void add(const fb::model::point16_t& position, const fb::model::point16_t& pivot, const fb::model::door& model, bool opened);

    /**
     * @brief      Searches for the first match.
     *
     * @param[in]  ch    The ch
     *
     * @return     { description_of_the_return_value }
     */
    door* find(const character& ch) const;
};

/**
 * @brief      This class describes an iterator.
 */
class door_container::iterator : public door_container::base_iterator
{
public:
    std::optional<std::pair<fb::model::point16_t, door&>> pair;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  i          { parameter_description }
     * @param[in]  container  The container
     */
    iterator(const door_container::base_iterator& i, const door_container& container);
    /**
     * @brief      Destroys the object.
     */
    ~iterator() = default;

public:
    /**
     * @brief      Multiplication operator.
     *
     * @return     The result of the multiplication
     */
    std::pair<fb::model::point16_t, door&> operator* ();
};

/**
 * @brief      This class describes a constant iterator.
 */
class door_container::const_iterator : public door_container::const_base_iterator
{
public:
    const std::optional<std::pair<fb::model::point16_t, door&>> pair;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  i          { parameter_description }
     * @param[in]  container  The container
     */
    const_iterator(const door_container::const_base_iterator& i, const door_container& container);
    /**
     * @brief      Destroys the object.
     */
    ~const_iterator() = default;

public:
    /**
     * @brief      Multiplication operator.
     *
     * @return     The result of the multiplication
     */
    const std::pair<fb::model::point16_t, door&> operator* () const;
};

} // namespace fb::game

#endif