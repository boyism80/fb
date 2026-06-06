#include <fb/model/model.h>

const fb::model::promotion* fb::model::__promotion::operator() (fb::model::enum_value::CLASS cls,
                                                                uint8_t                      promotion) const
{
    try
    {
        auto& model = (*this)[cls][promotion];
        return &model;
    }
    catch (std::exception&)
    {
        return nullptr;
    }
}

bool fb::model::__promotion::name2class(std::string_view              name,
                                        fb::model::enum_value::CLASS& cls,
                                        uint8_t&                      promotion) const
{
    for (const auto& [k1, v1] : *this)
    {
        for (const auto& [k2, v2] : v1)
        {
            if (v2.name == name)
            {
                cls       = k1;
                promotion = v2.step;
                return true;
            }
        }
    }

    return false;
}

bool fb::model::__promotion::class2name(fb::model::enum_value::CLASS cls, uint8_t promotion, std::string& name) const
{
    if (this->contains(cls) == false)
        return false;

    if ((*this)[cls].contains(promotion) == false)
        return false;

    name = (*this)[cls][promotion].name;
    return true;
}