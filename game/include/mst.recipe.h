#ifndef __MST_RECIPE_H__
#define __MST_RECIPE_H__

#include <model.h>
#include <fb/mst.h>
#include <item.h>
#include <fb/generator.h>

using namespace fb::model::enum_value;

namespace fb { namespace model {

class recipe_node : public fb::mst<const fb::model::dsl::item&>
{
public:
    using recipe_ref_type = std::reference_wrapper<const fb::model::recipe>;
    using dsl_ref_type    = std::reference_wrapper<fb::model::dsl::item>;

private:
    const fb::model::dsl::item _dsl;
    std::list<recipe_ref_type> _recipes;

public:
    recipe_node(uint32_t id = 0, uint32_t count = 0, const recipe_node* parent = nullptr);
    recipe_node(const recipe_node&) = delete;
    recipe_node(const fb::model::dsl::item& data, const recipe_node* parent);
    ~recipe_node() = default;

private:
    const fb::model::dsl::item&    init(uint32_t id, uint32_t count);
    fb::generator<recipe_ref_type> find(const std::vector<fb::model::dsl::item>& source, int i);
    recipe_node*                   find(uint32_t id) const;
    recipe_node*                   find(const fb::model::dsl::item& item) const;
    recipe_node&                   add(const fb::model::dsl::item& item);
    void                           compact(const std::vector<fb::model::dsl::item>& source, std::vector<fb::model::dsl::item>& dest);

public:
    void                           add(const fb::model::recipe& recipe);
    fb::generator<recipe_ref_type> find(const std::vector<fb::model::dsl::item>& source);
    bool                           compare(const fb::model::dsl::item&) const;
};

}} // namespace fb::model

#endif