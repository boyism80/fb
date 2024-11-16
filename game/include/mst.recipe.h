#ifndef __MST_RECIPE_H__
#define __MST_RECIPE_H__

#include <fb/model/model.h>
#include <fb/mst.h>
#include <item.h>
#include <fb/generator.h>

using namespace fb::model::enum_value;

namespace fb { namespace model {

/**
 * @brief      This class describes a recipe node.
 */
class recipe_node : public fb::mst<const fb::model::dsl::item&>
{
public:
    using recipe_ref_type = std::reference_wrapper<const fb::model::recipe>;
    using dsl_ref_type    = std::reference_wrapper<fb::model::dsl::item>;

private:
    const fb::model::dsl::item _dsl;
    std::list<recipe_ref_type> _recipes;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  id      The identifier
     * @param[in]  count   The count
     * @param[in]  parent  The parent
     */
    recipe_node(uint32_t id = 0, uint32_t count = 0, const recipe_node* parent = nullptr);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    recipe_node(const recipe_node&) = delete;
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  data    The data
     * @param[in]  parent  The parent
     */
    recipe_node(const fb::model::dsl::item& data, const recipe_node* parent);
    /**
     * @brief      Destroys the object.
     */
    ~recipe_node() = default;

private:
    /**
     * @brief      { function_description }
     *
     * @param[in]  id     The identifier
     * @param[in]  count  The count
     *
     * @return     { description_of_the_return_value }
     */
    const fb::model::dsl::item& init(uint32_t id, uint32_t count);
    /**
     * @brief      Searches for the first match.
     *
     * @param[in]  source  The source
     * @param[in]  i       { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::generator<recipe_ref_type> find(const std::vector<fb::model::dsl::item>& source, int i);
    /**
     * @brief      Searches for the first match.
     *
     * @param[in]  id    The identifier
     *
     * @return     { description_of_the_return_value }
     */
    recipe_node* find(uint32_t id) const;
    /**
     * @brief      Searches for the first match.
     *
     * @param[in]  item  The item
     *
     * @return     { description_of_the_return_value }
     */
    recipe_node* find(const fb::model::dsl::item& item) const;
    /**
     * @brief      Adds the specified item.
     *
     * @param[in]  item  The item
     *
     * @return     { description_of_the_return_value }
     */
    recipe_node& add(const fb::model::dsl::item& item);
    /**
     * @brief      { function_description }
     *
     * @param[in]  source  The source
     * @param      dest    The destination
     */
    void compact(const std::vector<fb::model::dsl::item>& source, std::vector<fb::model::dsl::item>& dest);

public:
    /**
     * @brief      Adds the specified recipe.
     *
     * @param[in]  recipe  The recipe
     */
    void add(const fb::model::recipe& recipe);
    /**
     * @brief      Searches for the first match.
     *
     * @param[in]  source  The source
     *
     * @return     { description_of_the_return_value }
     */
    fb::generator<recipe_ref_type> find(const std::vector<fb::model::dsl::item>& source);
    /**
     * @brief      { function_description }
     *
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool compare(const fb::model::dsl::item&) const;
};

}} // namespace fb::model

#endif