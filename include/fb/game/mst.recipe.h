#ifndef __MST_RECIPE_H__
#define __MST_RECIPE_H__

#include <fb/game/model.h>
#include <fb/core/mst.h>
#include <list>

namespace fb { namespace model {

class recipe_node : public fb::mst<std::reference_wrapper<fb::model::dsl::item>>
{
private:
    std::list<std::reference_wrapper<const fb::model::combine>> _recipes;

public:
    recipe_node() = default;
    recipe_node(const recipe_node&) = delete;
    recipe_node(const std::reference_wrapper<fb::model::dsl::item>& data, const std::shared_ptr<recipe_node>& parent);
    ~recipe_node() = default;

public:
    std::shared_ptr<fb::mst<std::reference_wrapper<fb::model::dsl::item>>>* find(uint32_t id) const;
    std::shared_ptr<fb::mst<std::reference_wrapper<fb::model::dsl::item>>>* find(const fb::model::dsl::item& item) const;
    recipe_node* add(const fb::model::dsl::item& item);
};

} }

#endif