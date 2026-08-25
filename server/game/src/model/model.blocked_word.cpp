#include <model.additional.h>

#include <fb/model/model.h>
#include <fb/model/substring_matcher.h>

fb::model::___blocked_word::___blocked_word() :
    fb::model::kv_container<std::string, fb::model::blocked_word>(std::string_view("json/blocked_word.json"))
{
    this->hook.built = [this](auto& value) {
        this->_matcher.add_replacement(value.src, value.dst);
    };
}

std::string fb::model::___blocked_word::filter(std::string_view message) const
{
    return this->_matcher.filter(message);
}
