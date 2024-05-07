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
                    this->chat(fb::format("%s %d개를 %d전에 샀습니다.", item_model->name.c_str(), count.value(), price * count.value()));
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
                this->chat(fb::format("%s %d개를 %d전에 샀습니다.", item_model->name.c_str(), sell_count, price * sell_count));
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
        else if(item_model->attr(fb::game::item::ATTRIBUTE::BUNDLE) && exist_count + count > item_model->capacity)
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
                this->chat(fb::format("%s %d개를 %d전에 팔았습니다.", item_model->name.c_str(), count, price));
        }
        return true;
    }
    catch(std::runtime_error& e)
    {
        this->chat(e.what());
        return false;
    }
}

bool fb::game::npc::repair(fb::game::session& session, fb::game::item::model* item_model, bool done)
{
    try
    {
        auto model = this->based<fb::game::npc>();
        if (model->repair == false)
            return false;

        auto equipments = std::vector<fb::game::equipment*>();
        auto all = item_model == nullptr;
        if (all)
        {
            for (auto& [parts, equipment] : session.items.equipments())
            {
                if (equipment == nullptr)
                    continue;

                auto equipment_model = equipment->based<fb::game::equipment>();
                if (equipment_model->repair.has_value() == false)
                    continue;

                if (equipment->durability() >= equipment_model->durability)
                    continue;

                equipments.push_back(static_cast<fb::game::equipment*>(equipment));
            }

            for (auto i = 0; i < fb::game::CONTAINER_CAPACITY; i++)
            {
                auto item = session.items[i];
                if (item == nullptr)
                    continue;

                if (item->attr(fb::game::item::ATTRIBUTE::EQUIPMENT) == false)
                    continue;

                auto equipment_model = item->based<fb::game::equipment>();
                if (equipment_model->repair.has_value() == false)
                    continue;

                auto equipment = static_cast<fb::game::equipment*>(item);
                if (equipment->durability() >= equipment_model->durability)
                    continue;

                equipments.push_back(equipment);
            }
        }
        else
        {
            if (item_model == nullptr) // 정의되지 않은 아이템
                throw std::runtime_error("뭘 고쳐줘?");

            if(item_model->attr(fb::game::item::ATTRIBUTE::EQUIPMENT) == false)
                throw std::runtime_error("뭘 고쳐줘?");

            auto item = session.items.find(*item_model);
            if (item == nullptr)
                throw std::runtime_error("가지고 있지 않은데요");

            auto equipment_model = static_cast<fb::game::equipment::model*>(item_model);
            if(equipment_model->repair.has_value() == false)
                throw std::runtime_error(fb::format("%s 고칠 수 없습니다.", name_with(item_model->name).c_str()));

            auto equipment = static_cast<fb::game::equipment*>(item);
            if (equipment->durability() >= equipment_model->durability)
                throw std::runtime_error("이미 고쳐져 있습니다.");

            equipments.push_back(equipment);
        }


        if (equipments.size() == 0)
            throw std::runtime_error("고칠 물건이 없습니다.");

        auto price = 0;
        for (auto equipment : equipments)
        {
            auto equipment_model = equipment->based<fb::game::equipment>();
            price += (uint32_t)(equipment_model->repair.value() * (equipment_model->durability - equipment->durability().value()));
        }

        if (session.money() < price)
            throw std::runtime_error("돈이 부족합니다.");

        session.money_reduce(price);
        for (auto equipment : equipments)
        {
            auto equipment_model = equipment->based<fb::game::equipment>();
            equipment->durability(equipment_model->durability);
        }

        if (price == 0)
        {
            this->chat("거의 새거라 그냥 고쳐드렸습니다.");
        }
        else
        {
            this->chat(fb::format("고치는데 %d전이 들었습니다.", price));
        }

        return true;
    }
    catch (std::runtime_error& e)
    {
        this->chat(e.what());
        return false;
    }
}

bool fb::game::npc::hold_money(fb::game::session& session, std::optional<uint32_t> money)
{
    try
    {
        auto model = this->based<fb::game::npc>();
        if (model->hold_money == false)
            return false;

        if (money.has_value() == false)
            money = session.money();

        if(money.value() == 0)
            return false;

        if (money.value() > session.money())
            throw std::runtime_error("그만큼 가지고 있지 않습니다.");

        auto capacity = 0xFFFFFFFF - session.deposited_money();
        if (money > capacity)
            throw std::runtime_error("더 이상 맡길 수 없습니다.");

        session.money_reduce(money.value());
        session.deposit_money(money.value());
        this->chat(fb::format("금전 %d전을 맡았습니다.", money.value()));

        return true;
    }
    catch (std::runtime_error& e)
    {
        this->chat(e.what());
        return false;
    }
}

bool fb::game::npc::return_money(fb::game::session& session, std::optional<uint32_t> money)
{
    try
    {
        auto model = this->based<fb::game::npc>();
        if (model->hold_money == false)
            return false;

        if(session.deposited_money() == 0)
            throw std::runtime_error("맡아둔 돈이 없습니다.");

        if (money.has_value() == false)
            money = session.deposited_money();

        if (money > session.deposited_money())
            throw std::runtime_error("그만큼 맡기지 않았습니다.");

        auto capacity = 0xFFFFFFFF - session.money();
        if (money.value() > capacity)
            throw std::runtime_error("소지금이 너무 많습니다.");

        session.withdraw_money(money.value());
        session.money_add(money.value());
        this->chat(fb::format("금전 %d전을 돌려드렸습니다.", money.value()));

        return true;
    }
    catch (std::runtime_error& e)
    {
        this->chat(e.what());
        return false;
    }
}

bool fb::game::npc::hold_item(fb::game::session& session, fb::game::item::model* item, std::optional<uint16_t> count)
{
    try
    {
        auto model = this->based<fb::game::npc>();
        if (model->hold_item == false)
            return false;

        if (item == nullptr)
            return true;

        auto exists = session.items.find(*item);
        if (exists == nullptr)
            throw std::runtime_error("가지고 있지 않습니다.");

        if (count.has_value() == false)
            count = exists->count();

        if (count.value() == 0)
            return true;

        if (item->attr(fb::game::item::ATTRIBUTE::BUNDLE))
        {
            if (count.value() > exists->count())
                throw std::runtime_error("갯수가 모자랍니다.");
        }
        else
        {
            count = 1;
        }

        auto index = session.items.index(*exists);
        session.deposit_item(index, count.value());
        if (item->attr(fb::game::item::ATTRIBUTE::BUNDLE))
            this->chat(fb::format("%s %d개를 맡았습니다.", item->name.c_str(), count.value()));
        else
            this->chat(fb::format("%s 맡았습니다.", name_with(item->name).c_str()));

        return true;
    }
    catch (std::runtime_error& e)
    {
        this->chat(e.what());
        return false;
    }
}

bool fb::game::npc::return_item(fb::game::session& session, fb::game::item::model* item, std::optional<uint16_t> count)
{
    try
    {
        auto model = this->based<fb::game::npc>();
        if (model->hold_item == false)
            return false;

        if (item == nullptr)
            return true;

        auto deposited_item = session.deposited_item(*item);
        if (deposited_item == nullptr)
            throw std::runtime_error("그런 물품은 맡아두고 있지 않습니다.");

        if (count.has_value() == false)
            count = deposited_item->count();

        if (count.value() == 0)
            return true;

        if (count.value() > deposited_item->count())
            throw std::runtime_error("그만큼 가지고 있지 않습니다.");

        if (item->attr(fb::game::item::ATTRIBUTE::BUNDLE))
        {
            auto exists = session.items.find(*item);
            if (exists != nullptr && exists->count() + count.value() > item->capacity)
                throw std::runtime_error("더 이상 가질 수 없습니다.");
        }
        else
        {
            if(session.items.free() == false)
                throw std::runtime_error("더 이상 가질 수 없습니다.");
        }

        if (session.withdraw_item(*item, count.value()) == nullptr)
            throw std::runtime_error("알 수 없는 에러");

        if (item->attr(fb::game::item::ATTRIBUTE::BUNDLE))
            this->chat(fb::format("%s %d개를 돌려드렸습니다.", item->name.c_str(), count.value()));
        else
            this->chat(fb::format("%s 돌려드렸습니다.", name_with(item->name).c_str()));

        return true;
    }
    catch (std::runtime_error& e)
    {
        this->chat(e.what());
        return false;
    }
}

void fb::game::npc::sell_list()
{
    auto model = this->based<fb::game::npc>();
    if(model->sell.size() == 0)
        return;

    auto overflow = 0;
    auto items = std::vector<std::string>();
    for (auto& [_, pursuit] : model->sell)
    {
        for (auto& [item, price] : *fb::game::model::sell[pursuit])
        {
            if (items.size() >= 3)
                overflow++;
            else
                items.push_back(item->name);
        }
    }

    auto sstream = std::stringstream();
    auto names = boost::algorithm::join(items, ", ");
    if (overflow > 0)
        sstream << names << " 등 여러가지를";
    else
        sstream << name_with(names);

    sstream << " 판매하고 있습니다.";
    this->chat(sstream.str());
}

void fb::game::npc::buy_list()
{
    auto model = this->based<fb::game::npc>();
    if(model->buy.has_value() == false)
        return;

    auto overflow = 0;
    auto items = std::vector<std::string>();
    for (auto& [item, price] : *fb::game::model::buy[model->buy.value()])
    {
        if (items.size() >= 3)
            overflow++;
        else
            items.push_back(item->name);
    }

    auto sstream = std::stringstream();
    auto names = boost::algorithm::join(items, ", ");
    if (overflow > 0)
        sstream << names << " 등 여러가지를";
    else
        sstream << name_with(names);

    sstream << " 사고 있습니다.";
    this->chat(sstream.str());
}

void fb::game::npc::sell_price(const fb::game::item::model* item)
{
    auto model = this->based<fb::game::npc>();
    if (model->sell.size() == 0)
        return;

    try
    {
        auto price = uint32_t(0);
        if (item == nullptr || model->contains_sell(*item, price) == false)
            throw std::runtime_error("그런 물건은 안 팝니다.");

        this->chat(fb::format("%s %d전에 팔고 있습니다.", name_with(item->name, { "은", "는" }).c_str(), price));
    }
    catch (std::runtime_error& e)
    {
        this->chat(e.what());
    }
}

void fb::game::npc::buy_price(const fb::game::item::model* item)
{
    auto model = this->based<fb::game::npc>();
    if (model->buy.has_value() == false)
        return;

    try
    {
        auto price = uint32_t(0);
        if (item == nullptr || model->contains_buy(*item, price) == false)
            throw std::runtime_error("그런 물건은 안 삽니다.");

        this->chat(fb::format("%s %d전에 사고 있습니다.", name_with(item->name, { "은", "는" }).c_str(), price));
    }
    catch (std::runtime_error& e)
    {
        this->chat(e.what());
    }
}

bool fb::game::npc::deposited_money(const fb::game::session& session)
{
    auto model = this->based<fb::game::npc>();
    if(model->hold_money == false)
        return false;

    auto money = session.deposited_money();
    if(money == 0)
        this->chat("맡긴 돈이 없습니다.");
    else
        this->chat(fb::format("%d전을 맡아두고 있습니다.", money));
    
    return true;
}

bool fb::game::npc::rename_weapon(fb::game::session& session, const fb::game::item::model* item, const std::string& name)
{
    auto model = this->based<fb::game::npc>();
    if(model->rename == false)
        return false;

    try
    {
        if(item == nullptr) // 등록되지 않은 아이템
            throw std::runtime_error("그게 뭐야?");

        if(item->attr(fb::game::item::ATTRIBUTE::WEAPON) == false)
            throw std::runtime_error(fb::format("%s 무기가 아닙니다.", name_with(item->name).c_str()));

        auto weapon = session.items.find(*item);
        if (weapon == nullptr)
            throw std::runtime_error(fb::format("%s 가지고 있지 않습니다.", name_with(item->name).c_str()));

        auto weapon_model = static_cast<const fb::game::weapon::model*>(item);
        if (weapon_model->rename.has_value() == false)
            throw std::runtime_error(fb::format("%s 별칭을 부여할 수 없습니다.", name_with(item->name, {"은", "는"})));

        auto money = session.money();
        if (weapon_model->rename.value() > money)
            throw std::runtime_error(fb::format("돈이 모자랍니다. %s에 별칭을 부여하려면 %d전이 필요합니다.", item->name.c_str(), weapon_model->rename.value()));

        auto cp949 = CP949(name);
        if(cp949.size() < 4)
            throw std::runtime_error("이름이 너무 짧습니다.");

        if (cp949.size() > 32)
            throw std::runtime_error("이름이 너무 깁니다.");

        if (assert_korean(cp949) == false)
            throw std::runtime_error("그렇게 바꿀 수 없습니다.");

        static_cast<fb::game::weapon*>(weapon)->custom_name(name);
        session.money_reduce(weapon_model->rename.value());
        this->chat(fb::format("%s의 이름을 %s로 변경했습니다.", item->name.c_str(), name_with(name, { "으", "" }).c_str()));
    }
    catch(std::runtime_error& e)
    {
        this->chat(e.what());
        return false;
    }

    return true;
}

fb::game::npc::model::model(const fb::game::npc::model::config& config) : 
    fb::game::object::model(config),
    script(config.script),
    sell(config.sell),
    buy(config.buy),
    repair(config.repair),
    hold_money(config.hold_money),
    hold_item(config.hold_item),
    rename(config.rename)
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