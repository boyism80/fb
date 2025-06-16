#ifndef __MST_RECIPE_H__
#define __MST_RECIPE_H__

#include <fb/model/model.h>
#include <fb/mst.h>
#include <fb/game/item.h>
#include <fb/generator.h>

using namespace fb::model::enum_value;

namespace fb { namespace model {

/**
 * @brief      Represents a node in a recipe matching tree for crafting systems.
 *
 *             This class implements a tree-based data structure for efficiently matching
 *             crafting recipes against available ingredients. It uses a multiway search tree
 *             (MST) approach to organize recipes hierarchically, allowing for fast
 *             recipe lookup and ingredient validation during crafting operations.
 *
 *             Key features:
 *             - Hierarchical recipe organization for efficient matching
 *             - Support for complex multi-ingredient recipes
 *             - Generator-based recipe enumeration for memory efficiency
 *             - Ingredient compaction and optimization
 *             - Integration with the game's DSL item system
 */
class recipe_node : public fb::mst<const fb::model::dsl::item&>
{
public:
    using recipe_ref_type = std::reference_wrapper<const fb::model::recipe>; ///< Reference wrapper for recipes
    using dsl_ref_type    = std::reference_wrapper<fb::model::dsl::item>;    ///< Reference wrapper for DSL items

private:
    const fb::model::dsl::item _dsl;     ///< The DSL item data for this node
    std::list<recipe_ref_type> _recipes; ///< List of recipes associated with this node

public:
    /**
     * @brief      Constructs a new recipe node with specified parameters.
     *
     *             Creates a recipe node for the given item ID and count, optionally
     *             linking it to a parent node in the recipe tree hierarchy.
     *
     * @param[in]  id      The item identifier for this node.
     * @param[in]  count   The required count of the item.
     * @param[in]  parent  The parent node in the recipe tree (optional).
     */
    recipe_node(uint32_t id = 0, uint32_t count = 0, const recipe_node* parent = nullptr);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The source recipe node to copy from
     */
    recipe_node(const recipe_node&) = delete;
    /**
     * @brief      Constructs a new recipe node from DSL item data.
     *
     *             Creates a recipe node using existing DSL item data, linking it
     *             to the specified parent node in the recipe tree.
     *
     * @param[in]  data    The DSL item data to use for this node.
     * @param[in]  parent  The parent node in the recipe tree.
     */
    recipe_node(const fb::model::dsl::item& data, const recipe_node* parent);
    /**
     * @brief      Destroys the object.
     */
    ~recipe_node() = default;

private:
    /**
     * @brief      Initializes the DSL item data for this node.
     *
     *             Creates and configures the DSL item data structure using the provided
     *             item ID and count, setting up the node for recipe matching operations.
     *
     * @param[in]  id     The item identifier to initialize.
     * @param[in]  count  The required count of the item.
     *
     * @return     Reference to the initialized DSL item data.
     */
    const fb::model::dsl::item& init(uint32_t id, uint32_t count);

    /**
     * @brief      Recursively searches for matching recipes in the ingredient list.
     *
     *             Performs a recursive search through the multiway search tree to find
     *             recipes that match the provided ingredient list, starting from the
     *             specified index position.
     *
     * @param[in]  source  The list of available ingredients to match against.
     * @param[in]  i       The current index position in the ingredient list.
     *
     * @return     Generator yielding matching recipe references.
     */
    fb::generator<recipe_ref_type> find(const std::vector<fb::model::dsl::item>& source, int i);

    /**
     * @brief      Finds a child node with the specified item ID.
     *
     *             Searches through the child nodes of this recipe node to find one
     *             that matches the given item identifier.
     *
     * @param[in]  id    The item identifier to search for.
     *
     * @return     Pointer to the matching child node, or nullptr if not found.
     */
    recipe_node* find(uint32_t id) const;

    /**
     * @brief      Finds a child node matching the specified DSL item.
     *
     *             Searches through the child nodes to find one that matches the
     *             provided DSL item data, including both ID and count requirements.
     *
     * @param[in]  item  The DSL item to search for.
     *
     * @return     Pointer to the matching child node, or nullptr if not found.
     */
    recipe_node* find(const fb::model::dsl::item& item) const;

    /**
     * @brief      Adds a new child node for the specified DSL item.
     *
     *             Creates and adds a new child node to represent the given DSL item
     *             in the recipe tree hierarchy, expanding the search tree structure.
     *
     * @param[in]  item  The DSL item to add as a child node.
     *
     * @return     Reference to the newly created child node.
     */
    recipe_node& add(const fb::model::dsl::item& item);

    /**
     * @brief      Compacts and optimizes the ingredient list for efficient matching.
     *
     *             Processes the source ingredient list to remove duplicates, combine
     *             stackable items, and optimize the data structure for faster recipe
     *             matching operations.
     *
     * @param[in]  source  The original ingredient list to compact.
     * @param      dest    The destination list for the compacted ingredients.
     */
    void compact(const std::vector<fb::model::dsl::item>& source, std::vector<fb::model::dsl::item>& dest);

public:
    /**
     * @brief      Adds a recipe to the multiway search tree.
     *
     *             Integrates the specified recipe into the tree structure, creating
     *             the necessary nodes and paths to enable efficient recipe matching
     *             based on the recipe's ingredient requirements.
     *
     * @param[in]  recipe  The recipe to add to the search tree.
     */
    void add(const fb::model::recipe& recipe);

    /**
     * @brief      Finds all recipes that can be crafted with the given ingredients.
     *
     *             Searches through the multiway search tree to identify all recipes
     *             that can be crafted using the provided ingredient list, returning
     *             a generator for efficient iteration over matching recipes.
     *
     * @param[in]  source  The list of available ingredients for crafting.
     *
     * @return     Generator yielding references to matching recipes.
     */
    fb::generator<recipe_ref_type> find(const std::vector<fb::model::dsl::item>& source);

    /**
     * @brief      Compares this node's DSL item with another DSL item.
     *
     *             Performs a comparison between this recipe node's DSL item data
     *             and the provided DSL item, used for tree traversal and matching
     *             operations in the multiway search tree.
     *
     * @param[in]  item  The DSL item to compare against this node's item.
     *
     * @return     True if the items match according to the comparison criteria.
     */
    bool compare(const fb::model::dsl::item& item) const;
};

}} // namespace fb::model

#endif