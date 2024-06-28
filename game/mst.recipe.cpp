#include <fb/game/mst.recipe.h>

fb::model::recipe_node::recipe_node() : fb::mst<const fb::model::dsl::item&>(fb::model::recipe_node::initializer, fb::model::recipe_node::comparer)
{ }

fb::model::recipe_node::recipe_node(const fb::model::dsl::item& data, const recipe_node* parent) : fb::mst<const fb::model::dsl::item&>(data, parent)
{ }

fb::model::recipe_node* fb::model::recipe_node::find(uint32_t id) const
{
    for (auto& node : *this)
    {
        auto& data = node->data;
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
        auto& data = node->data;
        if (data.id == item.id && data.count == item.count)
            return static_cast<fb::model::recipe_node*>(node);

        return nullptr;
    }

    return nullptr;
}

fb::model::recipe_node* fb::model::recipe_node::add(const fb::model::dsl::item& item)
{
    auto& data = this->data;
    if(data.id == item.id)
    {
        if(data.count == 0)
        {
            auto child = this->find(item);
            if (child != nullptr)
                return child;
            
            return child->add(item);
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
            const auto& dummy = *this->_dummy_dsl.get();

            auto& based = static_cast<fb::mst<const fb::model::dsl::item&>&>(*this);
            child = &based.add<recipe_node>(dummy);
        }

        return child->add(item);
    }
}

void fb::model::recipe_node::compact(const std::vector<fb::model::dsl::item>& source, std::vector<fb::model::dsl::item>& dest)
{
    auto dict = std::unordered_map<uint32_t, uint32_t>();
    for (auto& params : source)
    {
        if (dict.contains(params.id))
            dict[params.id] += params.count;
        else
            dict.insert({ params.id, params.count });
    }

    auto keys = std::vector<uint32_t>();
    for (auto& [k, v] : dict)
    {
        keys.push_back(k);
    }
    std::sort(keys.begin(), keys.end());

    dest.clear();
    for (auto k : keys)
    {
        dest.push_back(fb::model::dsl::item(k, dict.at(k), 0.0));
    }
}

void fb::model::recipe_node::add(const fb::model::recipe& recipe)
{
    auto dict = std::unordered_map<uint32_t, uint32_t>();
    for (auto& dsl : recipe.source)
    {
        auto params = fb::model::dsl::item(dsl.params);
        if (dict.contains(params.id))
            dict[params.id] += params.count;
        else
            dict.insert({ params.id, params.count });
    }

    auto source = std::vector<fb::model::dsl::item*>();
    for (auto& [id, count] : dict)
    {
        auto allocated = new fb::model::dsl::item(id, count, 0.0);
        source.push_back(allocated);
        this->_allocated_list.push_back(std::unique_ptr<fb::model::dsl::item>(allocated));
    }

    std::sort(source.begin(), source.end(), [](auto x1, auto x2)
    {
        return x1->id < x2->id;
    });

    auto node = this;
    for (auto ptr : source)
    {
        node = node->add(*ptr);
    }
    node->_recipes.push_back(recipe);
}

fb::generator<fb::model::recipe_node::recipe_ref_type> fb::model::recipe_node::find(const std::vector<fb::model::dsl::item>& source, int i)
{
    if (source.size() <= i)
        co_return;

    auto& item = source[i];
    for (auto raw : *this)
    {
        auto node = static_cast<recipe_node*>(raw);
        auto& data = node->data;
        if (data.id != item.id)
            continue;

        if (data.count <= item.count)
        {
            for (auto& x : node->_recipes)
                co_yield x;
        }

        auto gen = (data.count == 0) ?
            node->find(source, i) : node->find(source, i + 1);

        while (gen.next())
        {
            co_yield gen.value();
        }
    }
}

fb::generator<fb::model::recipe_node::recipe_ref_type> fb::model::recipe_node::find(const std::vector<fb::model::dsl::item>& source)
{
    auto dict = std::unordered_map<uint32_t, uint32_t>();
    for (auto& params : source)
    {
        if (dict.contains(params.id))
            dict[params.id] += params.count;
        else
            dict.insert({ params.id, params.count });
    }

    auto compact = std::vector<fb::model::dsl::item>();
    this->compact(source, compact);

    for(int i = 0; i < compact.size(); i++)
    {
        auto gen = this->find(compact, i);
        while(gen.next())
            co_yield gen.value();
    }
}

const fb::model::dsl::item& fb::model::recipe_node::initializer()
{
    return std::ref(fb::model::recipe_node::DUMMY);
}

bool fb::model::recipe_node::comprare(const fb::model::dsl::item& val1, const fb::model::dsl::item& val2)
{
    auto& dsl1 = val1;
    auto& dsl2 = val2;

    return dsl1.id == dsl2.id && dsl1.count == dsl2.count;
}