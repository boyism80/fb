#include <model.h>

fb::model::spell* fb::model::__spell::name2spell(const std::string& name) const
{
	for (auto& [k, v] : *this)
    {
        if (v.name == name)
            return &v;
    }
    return nullptr;
}