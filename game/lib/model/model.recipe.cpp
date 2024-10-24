#include <mst.recipe.h>

fb::model::__recipe::__recipe() :
    fb::model::array_container<fb::model::recipe>(std::string("json/recipe.json"))
{
    this->_root      = new fb::model::recipe_node();
    this->hook.built = [this](auto& value) {
        this->_root->add(value);
    };
}

fb::model::__recipe::~__recipe()
{
    delete this->_root;
}

const fb::model::recipe* fb::model::__recipe::find(const std::vector<fb::model::dsl::item>& dsl) const
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
            auto params = fb::model::dsl::item(source.params);
            count += params.count;
        }

        if (count > source_count)
        {
            ptr          = &recipe;
            source_count = count;
        }
    }

    return ptr;
}