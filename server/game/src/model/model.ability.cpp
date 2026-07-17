#include <model.additional.h>
#include <fb/model/model.h>

#include <algorithm>
#include <sstream>
#include <vector>

fb::model::__ability::__ability() :
    fb::model::kv_container<fb::model::enum_value::CLASS, fb::model::kv_container<uint8_t, fb::model::ability>>(
        std::string_view("json/ability.json"))
{ }

void fb::model::__ability::build_stacked_exp()
{
    this->_stacked_exp.clear();

    std::unordered_map<fb::model::enum_value::CLASS, std::unordered_map<uint8_t, uint64_t>> intra;

    for (auto& [cls, levels] : *this)
    {
        std::vector<uint8_t> sorted;
        sorted.reserve(levels.size());
        for (auto& [level, ability] : levels)
            sorted.push_back(level);

        std::sort(sorted.begin(), sorted.end());

        auto& map = intra[cls];
        auto  sum = uint64_t{0};
        for (auto level : sorted)
        {
            sum        += levels[level].exp;
            map[level]  = sum;
        }
    }

    const auto none_cls = fb::model::enum_value::CLASS::NONE;
    for (auto& [cls, class_intra] : intra)
    {
        auto& map = this->_stacked_exp[cls];

        if (cls == none_cls)
        {
            map = class_intra;
            continue;
        }

        auto none_base = uint32_t{0};
        if (auto none_iter = intra.find(none_cls); none_iter != intra.end())
        {
            auto min_level =
                std::min_element(class_intra.begin(), class_intra.end(), [](const auto& lhs, const auto& rhs) {
                    return lhs.first < rhs.first;
                })->first;

            auto prev_level = static_cast<uint8_t>(min_level - 1);
            if (prev_level > 0)
            {
                auto prev_iter = none_iter->second.find(prev_level);
                if (prev_iter != none_iter->second.end())
                    none_base = prev_iter->second;
            }
        }

        for (auto& [level, value] : class_intra)
            map[level] = none_base + value;
    }
}

uint64_t fb::model::__ability::stacked_exp(fb::model::enum_value::CLASS cls, uint8_t level) const
{
    auto cls_iter = this->_stacked_exp.find(cls);
    if (cls_iter == this->_stacked_exp.cend())
    {
        auto sstream = std::stringstream();
        sstream << enum_tostring<fb::model::enum_value::CLASS>(cls) << " does not exists in ability stacked cache";
        throw std::runtime_error(sstream.str());
    }

    auto level_iter = cls_iter->second.find(level);
    if (level_iter == cls_iter->second.cend())
    {
        auto sstream = std::stringstream();
        sstream << enum_tostring<fb::model::enum_value::CLASS>(cls) << " level " << static_cast<int>(level)
                << " does not exists in ability stacked cache";
        throw std::runtime_error(sstream.str());
    }

    return level_iter->second;
}
