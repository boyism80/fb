#include <fb/game/model.h>

fb::model::spell* fb::model::__spell::name2spell(const std::string& name) const
{
	auto i = std::find_if(this->begin(), this->end(), 
        [&name](auto& pair)
        {
            return pair.second.name == name;
        });

    return i != this->end() ? i->second.get() : nullptr;
}