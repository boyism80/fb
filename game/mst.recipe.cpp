#include <fb/game/mst.recipe.h>

fb::model::recipe_node::recipe_node() : fb::mst<std::reference_wrapper<const fb::model::dsl::item>>(fb::model::recipe_node::initializer, fb::model::recipe_node::comparer)
{ }

fb::model::recipe_node::recipe_node(const fb::model::dsl::item& data, const recipe_node* parent) : fb::mst<std::reference_wrapper<const fb::model::dsl::item>>(std::ref(data), parent)
{ }

fb::model::recipe_node* fb::model::recipe_node::find(uint32_t id) const
{
    for (auto& node : *this)
    {
        auto& data = node->data.get();
        if (data.id == id)
            return static_cast<fb::model::recipe_node*>(node);

        return nullptr;
    }

    return nullptr;
}

fb::model::recipe_node* fb::model::recipe_node::find(const fb::model::dsl::item& item) const
{
    for (auto& node : *this)
    {
        auto& data = node->data.get();
        if (data.id == item.id && data.count == item.count)
            return static_cast<fb::model::recipe_node*>(node);

        return nullptr;
    }

    return nullptr;
}

fb::model::recipe_node* fb::model::recipe_node::add(const fb::model::dsl::item& item)
{
    auto& data = this->data.get();
    if(data.id == item.id)
    {
        if(data.count == 0)
        {
            auto child = this->find(item);
            if (child != nullptr)
                return child;
            
            
            auto raw = static_cast<fb::mst<std::reference_wrapper<const fb::model::dsl::item>>*>(child);
            return raw->add<recipe_node>(std::ref(item));
        }
        else
        {
            throw std::runtime_error("you must sort data before add");
        }
    }
    else
    {
        auto child = this->find(item.id);
        if (child == nullptr)
        {
            this->_dummy_dsl = std::make_unique<fb::model::dsl::item>(item.id, 0, 0.0);
            auto raw = static_cast<fb::mst<std::reference_wrapper<const fb::model::dsl::item>>*>(this);
            child = raw->add<recipe_node>(std::ref(*_dummy_dsl.get()));
        }

        return child->add(item);
    }
}

void fb::model::recipe_node::add(const fb::model::combine& mix)
{
    auto node = this;
    for(auto& dsl : mix.source)
    {
        if (dsl.header != DSL::item)
            continue;

        auto params = fb::model::dsl::item(dsl.params);
        node = node->add(params);
    }
    node->_recipes.push_back(mix);
}

fb::generator<std::reference_wrapper<const fb::model::combine>> fb::model::recipe_node::find(const std::vector<std::reference_wrapper<fb::model::dsl::item>>& mix, int i)
{
    if (mix.size() <= i)
        co_return;

    auto& item = mix[i].get();
    for (auto raw : *this)
    {
        auto node = static_cast<recipe_node*>(raw);
        auto& data = node->data.get();
        if (data.id != item.id)
            continue;

        if (data.count <= item.count)
        {
            for (auto& x : node->_recipes)
                co_yield x;
        }

        auto gen = (data.count == 0) ?
            node->find(mix, i) : node->find(mix, i + 1);

        while (gen.next())
        {
            co_yield gen.value();
        }
    }
}

fb::generator<std::reference_wrapper<const fb::model::combine>> fb::model::recipe_node::find(const std::vector<std::reference_wrapper<fb::model::dsl::item>>& mix)
{
    for(int i = 0; i < mix.size(); i++)
    {
        auto gen = this->find(mix, i);
        while(gen.next())
            co_yield gen.value();
    }
}

const std::reference_wrapper<const fb::model::dsl::item>& fb::model::recipe_node::initializer()
{
    return std::ref(fb::model::recipe_node::DUMMY);
}

bool fb::model::recipe_node::comprare(const std::reference_wrapper<const fb::model::dsl::item>& val1, const std::reference_wrapper<const fb::model::dsl::item>& val2)
{
    auto& dsl1 = val1.get();
    auto& dsl2 = val2.get();

    return dsl1.id == dsl2.id && dsl1.count == dsl2.count;
}