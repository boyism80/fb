#ifndef __MST_RECIPE_H__
#define __MST_RECIPE_H__

#include <fb/game/model.h>
#include <fb/core/mst.h>
#include <list>
#include <fb/core/coroutine.h>

namespace fb { namespace model {

class recipe_node : public fb::mst<std::reference_wrapper<const fb::model::dsl::item>>
{
private:
    inline static fb::model::dsl::item DUMMY = fb::model::dsl::item(0, 0, 0);

private:
    std::list<std::reference_wrapper<const fb::model::combine>> _recipes;
    std::unique_ptr<fb::model::dsl::item> _dummy_dsl;

public:
    recipe_node();
    recipe_node(const recipe_node&) = delete;
    recipe_node(const fb::model::dsl::item& data, const recipe_node* parent);
    ~recipe_node() = default;

private:
    fb::generator<std::reference_wrapper<const fb::model::combine>> find(const std::vector<std::reference_wrapper<fb::model::dsl::item>>& mix, int i);

public:
    recipe_node* find(uint32_t id) const;
    recipe_node* find(const fb::model::dsl::item& item) const;
    recipe_node* add(const fb::model::dsl::item& item);

    fb::generator<std::reference_wrapper<const fb::model::combine>> find(const std::vector<std::reference_wrapper<fb::model::dsl::item>>& mix);

public:
    static const std::reference_wrapper<const fb::model::dsl::item>& initializer();
    static bool comprare(const std::reference_wrapper<const fb::model::dsl::item>& val1, const std::reference_wrapper<const fb::model::dsl::item>& val2);
};

} }

#endif