#ifndef __MST_RECIPE_H__
#define __MST_RECIPE_H__

#include <fb/game/model.h>
#include <fb/core/mst.h>
#include <list>
#include <fb/core/coroutine.h>

using namespace fb::model::enum_value;

namespace fb { namespace model {

class recipe_node : public fb::mst<const fb::model::dsl::item&>
{
public:
    using recipe_ref_type = std::reference_wrapper<const fb::model::recipe>;
    using dsl_ref_type = std::reference_wrapper<fb::model::dsl::item>;

private:
    inline static fb::model::dsl::item DUMMY = fb::model::dsl::item(0, 0, 0);
    std::vector<std::unique_ptr<fb::model::dsl::item>> _allocated_list;

private:
    std::list<recipe_ref_type> _recipes;
    std::unique_ptr<fb::model::dsl::item> _dummy_dsl;

public:
    recipe_node();
    recipe_node(const recipe_node&) = delete;
    recipe_node(const fb::model::dsl::item& data, const recipe_node* parent);
    ~recipe_node() = default;

private:
    fb::generator<recipe_ref_type> find(const std::vector<fb::model::dsl::item>& source, int i);
    recipe_node* find(uint32_t id) const;
    recipe_node* find(const fb::model::dsl::item& item) const;
    recipe_node* add(const fb::model::dsl::item& item);
    void compact(const std::vector<fb::model::dsl::item>& source, std::vector<fb::model::dsl::item>& dest);

public:
    void add(const fb::model::recipe& recipe);

    fb::generator<recipe_ref_type> find(const std::vector<fb::model::dsl::item>& source);

public:
    static const fb::model::dsl::item& initializer();
    static bool comprare(const fb::model::dsl::item& val1, const fb::model::dsl::item& val2);
};

} }

#endif