#include <model.h>

const fb::model::promotion* fb::model::__promotion::operator() (fb::model::enum_value::CLASS cls, uint8_t promotion) const
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

bool fb::model::__promotion::name2class(const std::string& name, fb::model::enum_value::CLASS& cls, uint8_t& promotion) const
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