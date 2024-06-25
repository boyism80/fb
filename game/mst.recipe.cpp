#include <fb/game/mst.recipe.h>

fb::model::recipe_node::recipe_node(const std::reference_wrapper<fb::model::dsl::item>& data, const std::shared_ptr<fb::model::recipe_node>& parent) : fb::mst<std::reference_wrapper<fb::model::dsl::item>>(data, parent)
{ }

std::shared_ptr<fb::mst<std::reference_wrapper<fb::model::dsl::item>>>* fb::model::recipe_node::find(uint32_t id) const
{
    for (auto& node : *this)
    {
        auto& data = node->data.get();
        if (data.id == id)
            return &node;

        return nullptr;
    }

    return nullptr;
}

std::shared_ptr<fb::mst<std::reference_wrapper<fb::model::dsl::item>>>* fb::model::recipe_node::find(const fb::model::dsl::item& item) const
{
    for (auto& node : *this)
    {
        auto& data = node->data.get();
        if (data.id == item.id && data.count == item.count)
        {
            return &node;
        }

        return nullptr;
    }

    return nullptr;
}

fb::model::recipe_node* fb::model::recipe_node::add(const fb::model::dsl::item& item)
{
    //auto& data = this->data.get();
    //if(data.id == item.id)
    //{
    //    if(data.count == 0)
    //    {
    //        auto child = this->find(item);
    //        if(child == nullptr)
    //            return static_cast<recipe_node*>(child);

    //        recipe_node(item, *this);

    //        auto node = std::make_shared<recipe_node>(key, trans)
    //    }
    //}

    return nullptr;
}