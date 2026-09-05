#include <fb/game/mst.recipe.h>

fb::model::___recipe::___recipe() :
    fb::model::array_container<fb::model::recipe>(std::string_view("json/recipe.json"))
{
    this->_root      = std::make_unique<fb::model::recipe_node>();
    this->hook.built = [this](auto& value) {
        this->_root->add(value);
    };
}

fb::model::___recipe::~___recipe() = default;

const fb::model::recipe* fb::model::___recipe::find(const std::vector<fb::model::dsl::item>& dsl) const
{
    auto gen          = this->_root->find(dsl);
    auto ptr          = (const fb::model::recipe*)nullptr;
    auto source_count = 0;
    while (gen.next())
    {
        auto& recipe = gen.value().get();
        auto  count  = 0;
        for (auto& source : recipe.source)
        {
            auto params  = fb::model::dsl::item(source.params);
            count       += params.count;
        }

        if (count > source_count)
        {
            ptr          = &recipe;
            source_count = count;
        }
    }

    return ptr;
}