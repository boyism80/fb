#include <mst.recipe.h>

fb::model::recipe_node::recipe_node(uint32_t id, uint32_t count, const recipe_node* parent) : 
    _dsl(id, count, 0.0),
    fb::mst<const fb::model::dsl::item&>(_dsl, parent)
{ }

fb::model::recipe_node::recipe_node(const fb::model::dsl::item& data, const recipe_node* parent) : 
    _dsl(data.id, data.count, data.percent),
    fb::mst<const fb::model::dsl::item&>(_dsl, parent)
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

fb::model::recipe_node& fb::model::recipe_node::add(const fb::model::dsl::item& item)
{
    auto& data = this->data;
    if(data.id == item.id)
    {
        if(data.count == 0)
        {
            auto child = this->find(item);
            if (child != nullptr)
                return *child;
            
            auto& based = static_cast<fb::mst<const fb::model::dsl::item&>&>(*this);
            return based.add<recipe_node>(item.id, item.count, this);
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
            auto& based = static_cast<fb::mst<const fb::model::dsl::item&>&>(*this);
            child = &based.add<recipe_node>(item.id, 0, this);
        }

        return child->add(item);
    }
}

void fb::model::recipe_node::compact(const std::vector<fb::model::dsl::item>& source, std::vector<fb::model::dsl::item>& dest)
{
    auto buffer = std::unordered_map<uint32_t, uint32_t>();
    for (auto& params : source)
    {
        if (buffer.contains(params.id))
            buffer[params.id] += params.count;
        else
            buffer.insert({ params.id, params.count });
    }

    auto keys = std::vector<uint32_t>();
    for (auto& [k, v] : buffer)
    {
        keys.push_back(k);
    }
    std::sort(keys.begin(), keys.end());

    dest.clear();
    for (auto k : keys)
    {
        dest.push_back(fb::model::dsl::item(k, buffer.at(k), 0.0));
    }
}

void fb::model::recipe_node::add(const fb::model::recipe& recipe)
{
    auto buffer = std::unordered_map<uint32_t, uint32_t>();
    for (auto& dsl : recipe.source)
    {
        auto params = fb::model::dsl::item(dsl.params);
        if (buffer.contains(params.id))
            buffer[params.id] += params.count;
        else
            buffer.insert({ params.id, params.count });
    }

    auto dsls = std::vector<fb::model::dsl::item>();
    for (auto& [id, count] : buffer)
    {
        dsls.push_back(fb::model::dsl::item(id, count, 0.0));
    }

    auto ptrs = std::vector<fb::model::dsl::item*>();
    for (auto& dsl : dsls)
    {
        ptrs.push_back(&dsl);
    }

    std::sort(ptrs.begin(), ptrs.end(), [](auto x1, auto x2)
    {
        return x1->id < x2->id;
    });

    auto node = this;
    for (auto& ptr : ptrs)
    {
        node = &node->add(*ptr);
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
    auto buffer = std::unordered_map<uint32_t, uint32_t>();
    for (auto& params : source)
    {
        if (buffer.contains(params.id))
            buffer[params.id] += params.count;
        else
            buffer.insert({ params.id, params.count });
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

bool fb::model::recipe_node::compare(const fb::model::dsl::item& val) const
{
    return this->data.id == val.id && this->data.count == val.count;
}