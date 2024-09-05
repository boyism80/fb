#include <fb/game/item.h>
#include <fb/game/context.h>

fb::game::cash::cash(fb::game::context& context, uint32_t value) : 
    fb::game::item(context, match_model(context, value)), value(value)
{
    
}

fb::game::cash::~cash()
{ }

const fb::model::cash& fb::game::cash::match_model(fb::game::context& context, uint32_t value)
{
    if (value == 0)
        throw std::runtime_error("money cannot be zero");

    if(value == 1)
        return static_cast<const fb::model::cash&>(context.model.item[fb::model::const_value::item::BRONZE]);

    if (value < 50)
        return static_cast<const fb::model::cash&>(context.model.item[fb::model::const_value::item::BRONZE_BUNDLE]);

    if (value == 50)
        return static_cast<const fb::model::cash&>(context.model.item[fb::model::const_value::item::SILVER]);

    if (value == 100)
        return static_cast<const fb::model::cash&>(context.model.item[fb::model::const_value::item::GOLD]);

    if (value < 1000)
        return static_cast<const fb::model::cash&>(context.model.item[fb::model::const_value::item::SILVER_BUNDLE]);
    
    return static_cast<const fb::model::cash&>(context.model.item[fb::model::const_value::item::GOLD_BUNDLE]);
}

std::string fb::game::cash::inven_name() const
{
    auto& model = this->based<fb::model::cash>();
    auto sstream = std::stringstream();
    sstream << model.name 
            << ' ' 
            << this->value 
            << "전";

    return sstream.str();
}

async::task< fb::game::cash*> fb::game::cash::replace(uint32_t value)
{
    fb::game::cash* result = nullptr;
    if(this->empty())
    {
        result = nullptr;
    }
    else
    {
        result = this->context.make<fb::game::cash>(value);
    }
    co_await this->destroy();
    co_return result;
}

uint32_t fb::game::cash::reduce(uint32_t value)
{
    uint32_t reduce = std::min<uint32_t>(this->value, value);

    this->replace(this->value - reduce);
    return this->value;
}

bool fb::game::cash::empty() const
{
    return this->value == 0;
}