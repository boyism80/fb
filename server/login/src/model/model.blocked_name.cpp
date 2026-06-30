#include <model.additional.h>

#include <fb/model/model.h>
#include <fb/model/substring_matcher.h>

fb::model::__blocked_name::__blocked_name() :
    fb::model::kv_container<std::string, fb::model::blocked_name>(std::string_view("json/blocked_name.json"))
{
    this->hook.built = [this](auto& value) {
        this->_matcher.add_pattern(value.id);
    };
}

bool fb::model::__blocked_name::contains_substring(std::string_view name) const
{
    return this->_matcher.contains(name);
}
