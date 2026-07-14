#include <fb/game/server.h>
#include <fb/game/item.h>

using namespace fb::game;
using table = fb::model::table;

cash::cash(fb::game::server& server, uint32_t value) :
    fb::game::item(server, match_model(server, value)),
    value(value)
{ }

cash::~cash()
{ }

const fb::model::cash& cash::match_model(fb::game::server& server, uint32_t value)
{
    if (value == 0)
        throw std::runtime_error("money cannot be zero");

    if (value == 1)
        return static_cast<const fb::model::cash&>(table::item[fb::model::const_value::item::BRONZE]);

    if (value < 50)
        return static_cast<const fb::model::cash&>(table::item[fb::model::const_value::item::BRONZE_BUNDLE]);

    if (value == 50)
        return static_cast<const fb::model::cash&>(table::item[fb::model::const_value::item::SILVER]);

    if (value == 100)
        return static_cast<const fb::model::cash&>(table::item[fb::model::const_value::item::GOLD]);

    if (value < 1000)
        return static_cast<const fb::model::cash&>(table::item[fb::model::const_value::item::SILVER_BUNDLE]);

    return static_cast<const fb::model::cash&>(table::item[fb::model::const_value::item::GOLD_BUNDLE]);
}

std::string cash::inven_name() const
{
    auto& model   = this->based<fb::model::cash>();
    auto  sstream = std::stringstream();
    sstream << model.name << ' ' << this->value << "전";

    return sstream.str();
}

async::task<std::shared_ptr<cash>> cash::replace(uint32_t value)
{
    std::shared_ptr<cash> result = nullptr;
    if (this->empty() == false)
        result = this->server.make<cash>(value);

    co_await this->destroy();
    co_return result;
}

async::task<uint32_t> cash::reduce(uint32_t value)
{
    uint32_t reduce_amount = std::min<uint32_t>(this->value, value);
    auto     remain        = this->value - reduce_amount;
    auto     replaced      = co_await this->replace(remain);
    co_return replaced != nullptr ? replaced->value : 0;
}

bool cash::empty() const
{
    return this->value == 0;
}