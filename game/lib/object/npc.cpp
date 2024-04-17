#include <fb/game/npc.h>
#include <fb/game/context.h>

using namespace fb::game;

fb::game::npc::npc(fb::game::context& context, const fb::game::npc::model* model) : 
    fb::game::object(context, model, fb::game::object::config())
{ }

fb::game::npc::npc(const npc& right) :
    object(right)
{ }

fb::game::npc::~npc()
{ }

bool fb::game::npc::buy(fb::game::session& session, fb::game::item::model* item_model, std::optional<uint16_t> count, bool bought)
{
    try
    {
        auto model = this->based<fb::game::npc>();
        if(model->buy.has_value() == false)
            return false;

        if (item_model == nullptr)
            throw std::runtime_error("뭘 팔아?");

        auto slots = session.items.index_all(item_model);
        if(slots.size() == 0)
            throw std::runtime_error("가지고 있지도 않으면서...");

        auto price = uint32_t(0);
        if (model->contains_buy(*item_model, price) == false)
            throw std::runtime_error("그런 물건은 안 삽니다.");

        if (item_model->attr(fb::game::item::ATTRIBUTE::BUNDLE))
        {
            auto item = session.items.at(slots[0]);
            if (count.has_value() == false)
                count = item->count();

            if(count <= 0)
            {
                throw std::runtime_error("뭐래는거야..");
            }
            else if (count > item->count())
            {
                throw std::runtime_error("갯수가 모자라는데요?");
            }
            else
            {
                session.items.remove(slots[0], count.value(), fb::game::item::DELETE_TYPE::SELL);
                session.money_add(price * count.value());

                if(count == 1)
                    this->chat(fb::format("%s %d전에 샀습니다.", name_with(item_model->name).c_str(), price));
                else
                    this->chat(fb::format("%s %d개를 %d전에 샀습니다.", item_model->name, count, price * count.value()));
            }
        }
        else
        {
            if (count.has_value() == false)
                count = slots.size();

            if(count <= 0)
                throw std::runtime_error("뭐래는거야..");

            auto sell_count = 0;
            for (auto slot : slots)
            {
                session.items.remove(slot, 1, fb::game::item::DELETE_TYPE::SELL);
                sell_count++;

                if (sell_count >= count.value())
                    break;
            }
            session.money_add(price * sell_count);
            if(sell_count == 1)
                this->chat(fb::format("%s %d전에 샀습니다.", name_with(item_model->name).c_str(), price * sell_count));
            else
                this->chat(fb::format("%s %d개를 %d전에 샀습니다.", item_model->name, sell_count, price * sell_count));
        }

        return true;
    }
    catch(std::runtime_error& e)
    {
        this->chat(e.what());
        return false;
    }
}

bool fb::game::npc::sell(fb::game::session& session, fb::game::item::model* item_model, uint16_t count, bool sold)
{
    try
    {
        auto model = this->based<fb::game::npc>();
        if(model->sell.size() == 0)
            return false;

        if(item_model == nullptr)
            throw std::runtime_error("뭘 사?");

        auto sell_price = uint32_t(0);
        if(model->contains_sell(*item_model, sell_price) == false)
            throw std::runtime_error("그런 물건은 안 팝니다.");

        if(sold)
            return false;

        auto exist = session.items.find(*item_model);
        auto exist_count = exist != nullptr ? exist->count() : 0;
        auto price = sell_price * count;

        if (count <= 0)
        {
            throw std::runtime_error("뭐래는거야..");
        }
        else if (count > item_model->capacity)
        {
            throw std::runtime_error("그렇게나 많이요?");
        }
        else if(exist_count + count > item_model->capacity)
        {
            throw std::runtime_error("더 이상 가질 수 없습니다.");
        }
        else if (session.items.free() == false)
        {
            throw std::runtime_error("더 이상 가질 수 없습니다.");
        }
        else if (session.money() < price)
        {
            throw std::runtime_error("돈이 모자랍니다.");
        }
        else
        {
            session.items.add(item_model->make(this->context, count));
            session.money_reduce(price);
            if (count == 1)
                this->chat(fb::format("%s %d전에 팔았습니다.", name_with(item_model->name).c_str(), price));
            else
                this->chat(fb::format("%s %d개를 %d전에 팔았습니다.", item_model->name, count, price));
        }
        return true;
    }
    catch(std::runtime_error& e)
    {
        this->chat(e.what());
        return false;
    }
}

fb::game::npc::model::model(const fb::game::npc::model::config& config) : 
    fb::game::object::model(config),
    script(config.script),
    sell(config.sell),
    buy(config.buy)
{ }

fb::game::npc::model::~model()
{ }

bool fb::game::npc::model::contains_sell(const fb::game::item::model& item, uint32_t& price) const
{
    for (auto& [msg, i] : this->sell)
    {
        for (auto& sell : *fb::game::model::sell[i])
        {
            if (sell.first == &item)
            {
                price = sell.second.has_value() ? sell.second.value() : sell.first->price;
                return true;
            }
        }
    }

    price = 0;
    return false;
}

bool fb::game::npc::model::contains_buy(const fb::game::item::model& item, uint32_t& price) const
{
    if (this->buy.has_value() == false)
        return false;

    for (auto& buy : *fb::game::model::buy[this->buy.value()])
    {
        if (buy.first == &item)
        {
            price = buy.second.has_value() ? buy.second.value() : buy.first->price / 2;
            return true;
        }
    }

    price = 0;
    return false;
}