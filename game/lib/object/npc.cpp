#include <fb/game/context.h>
#include <fb/game/npc.h>

using namespace fb::game;

npc::npc(fb::game::context& context, const fb::model::npc& model) :
    fb::game::object(context, model, initial_params())
{ }

npc::npc(const npc& right) :
    object(right)
{ }

npc::~npc()
{ }

bool npc::buy(fb::game::character& ch, const fb::model::item* item_model, std::optional<uint16_t> count, bool bought)
{
    this->assert_thread();

    try
    {
        auto& model = this->based<fb::model::npc>();
        if (model.buy.has_value() == false)
            return false;

        if (item_model == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_INVALID_SELL_ITEM));

        auto slots = ch.items.index_all(*item_model);
        if (slots.size() == 0)
            throw std::runtime_error(_TEXT(MESSAGE_NO_HAVE_ITEM));

        auto buy = this->context.model.buy.find(model, *item_model);
        if (buy == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_ITEM_NOT_BUY));

        auto price = buy->price.value_or(item_model->price / 2);
        if (item_model->attr(ITEM_ATTRIBUTE::BUNDLE))
        {
            auto item = ch.items.at(slots[0]);
            if (count.has_value() == false)
                count = item->count();

            if (count <= 0)
            {
                throw std::runtime_error(_TEXT(MESSAGE_INVALID_ITEM_COUNT));
            }
            else if (count > item->count())
            {
                throw std::runtime_error(_TEXT(MESSAGE_NOT_ENOUGH_ITEM_COUNT));
            }
            else
            {
                ch.items.remove(slots[0], count.value(), ITEM_DELETE_TYPE::SELL);
                ch.money_add(price * count.value());

                if (count == 1)
                    this->chat(std::format("{} {}전에 샀습니다.", name_with(item_model->name), price));
                else
                    this->chat(std::format("{} {}개를 {}전에 샀습니다.",
                                           item_model->name,
                                           count.value(),
                                           price * count.value()));
            }
        }
        else
        {
            if (count.has_value() == false)
                count = slots.size();

            if (count <= 0)
                throw std::runtime_error(_TEXT(MESSAGE_INVALID_ITEM_COUNT));

            auto sell_count = 0;
            for (auto slot : slots)
            {
                ch.items.remove(slot, 1, ITEM_DELETE_TYPE::SELL);
                sell_count++;

                if (sell_count >= count.value())
                    break;
            }
            ch.money_add(price * sell_count);
            if (sell_count == 1)
                this->chat(std::format("{} {}전에 샀습니다.", name_with(item_model->name), price * sell_count));
            else
                this->chat(std::format("{} {}개를 {}전에 샀습니다.", item_model->name, sell_count, price * sell_count));
        }

        return true;
    }
    catch (std::runtime_error& e)
    {
        this->chat(e.what());
    }

    return false;
}

bool npc::sell(fb::game::character& ch, const fb::model::item* item_model, uint16_t count, bool sold)
{
    this->assert_thread();

    try
    {
        auto& model = this->based<fb::model::npc>();
        if (model.sell.size() == 0)
            return false;

        if (item_model == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_INVALID_BUY_ITEM_NAME));

        auto sell = this->context.model.sell.find(model, *item_model);
        if (sell == nullptr)
            throw std::runtime_error(_TEXT(MESSAGE_ITEM_NOT_SELL));

        if (sold)
            return false;

        auto exist       = ch.items.find(*item_model);
        auto exist_count = exist != nullptr ? exist->count() : 0;
        auto price       = sell->price.value_or(item_model->price) * count;

        if (count <= 0)
        {
            throw std::runtime_error(_TEXT(MESSAGE_INVALID_ITEM_COUNT));
        }
        else if (count > item_model->capacity)
        {
            throw std::runtime_error(_TEXT(MESSAGE_TOO_MANY_COUNT));
        }
        else if (item_model->attr(ITEM_ATTRIBUTE::BUNDLE) && exist_count + count > item_model->capacity)
        {
            throw std::runtime_error(_TEXT(MESSAGE_ITEM_FULL));
        }
        else if (ch.items.free() == false)
        {
            throw std::runtime_error(_TEXT(MESSAGE_ITEM_FULL));
        }
        else if (ch.money() < price)
        {
            throw std::runtime_error(_TEXT(MESSAGE_NOT_ENOUGH_MONEY));
        }
        else
        {
            ch.items.add(item_model->make(this->context, count));
            ch.money_reduce(price);
            if (count == 1)
                this->chat(std::format("{} {}전에 팔았습니다.", name_with(item_model->name), price));
            else
                this->chat(std::format("{} {}개를 {}전에 팔았습니다.", item_model->name, count, price));
        }
        return true;
    }
    catch (std::runtime_error& e)
    {
        this->chat(e.what());
    }

    return false;
}

void npc::sell_list()
{
    this->assert_thread();

    auto& model = this->based<fb::model::npc>();
    if (model.sell.size() == 0)
        return;

    auto overflow = 0;
    auto items    = std::vector<std::string>();
    for (auto& pursuit : model.sell)
    {
        for (auto& [_, x] : this->context.model.sell[pursuit])
        {
            auto& item = this->context.model.item[x.item];
            if (items.size() >= 3)
                overflow++;
            else
                items.push_back(item.name);
        }
    }

    auto sstream = std::stringstream();
    auto names   = boost::algorithm::join(items, ", ");
    if (overflow > 0)
        sstream << names << " 등 여러가지를";
    else
        sstream << name_with(names);

    sstream << " 판매하고 있습니다.";
    this->chat(sstream.str());
}

void npc::buy_list()
{
    this->assert_thread();

    auto& model = this->based<fb::model::npc>();
    if (model.buy.has_value() == false)
        return;

    auto overflow = 0;
    auto items    = std::vector<std::string>();
    for (auto& [_, x] : this->context.model.buy[model.buy.value()])
    {
        auto& item = this->context.model.item[x.item];
        if (items.size() >= 3)
            overflow++;
        else
            items.push_back(item.name);
    }

    auto sstream = std::stringstream();
    auto names   = boost::algorithm::join(items, ", ");
    if (overflow > 0)
        sstream << names << " 등 여러가지를";
    else
        sstream << name_with(names);

    sstream << " 사고 있습니다.";
    this->chat(sstream.str());
}

void npc::sell_price(const fb::model::item* item)
{
    this->assert_thread();

    auto& model = this->based<fb::model::npc>();
    if (model.sell.size() == 0)
        return;

    try
    {
        if (item != nullptr)
        {
            auto sell = this->context.model.sell.find(model, *item);
            if (sell != nullptr)
            {
                auto price = sell->price.value_or(item->price);
                this->chat(std::format("{} {}전에 팔고 있습니다.", name_with(item->name, {"은", "는"}), price));
            }
        }

        throw std::runtime_error(_TEXT(MESSAGE_ITEM_NOT_SELL));
    }
    catch (std::runtime_error& e)
    {
        this->chat(e.what());
    }
}

void npc::buy_price(const fb::model::item* item)
{
    this->assert_thread();

    auto& model = this->based<fb::model::npc>();
    if (model.buy.has_value() == false)
        return;

    try
    {
        if (item != nullptr)
        {
            auto sell = this->context.model.buy.find(model, *item);
            if (sell != nullptr)
            {
                auto price = sell->price.value_or(item->price / 2);
                this->chat(std::format("{} {}전에 사고 있습니다.", name_with(item->name, {"은", "는"}), price));
            }
        }

        throw std::runtime_error(_TEXT(MESSAGE_ITEM_NOT_BUY));
    }
    catch (std::runtime_error& e)
    {
        this->chat(e.what());
    }
}

void npc::assert_thread() const
{
    if (this->_map != nullptr)
        object::assert_thread();
    else
        return;
}