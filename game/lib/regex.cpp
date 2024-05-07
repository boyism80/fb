#include <fb/game/regex.h>

bool fb::game::regex::match(const std::string& message, fb::game::container::regex::TYPE v)
{
    auto& regex = fb::game::model::regex[v];
    auto what = boost::xpressive::smatch();
    
    return boost::xpressive::regex_search(message, what, regex);
}

bool fb::game::regex::match_sell_message(const std::string& message, fb::game::item::model*& item, std::optional<uint16_t>& count)
{
    auto& regex = fb::game::model::regex[fb::game::container::regex::TYPE::SELL];
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    auto name = what["name"].str();
    item = fb::game::model::items.name2item(name);

    if (what["count"].matched)
    {
        try
        {
            count = std::stoi(what["count"].str());
            if(count > 0xFFFF)
                throw std::out_of_range(fb::format("count cannot be %s", what["count"].str().c_str()));
        }
        catch (std::out_of_range&)
        {
            count = 0;
            return false;
        }
    }
    else if (what["all"].matched)
    {
        count.reset();
    }
    else
    {
        count = 1;
    }
    return true;
}

bool fb::game::regex::match_buy_message(const std::string& message, fb::game::item::model*& item, uint16_t& count)
{
    auto& regex = fb::game::model::regex[fb::game::container::regex::TYPE::BUY];
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    auto name = what["name"].str();
    item = fb::game::model::items.name2item(name);

    if (what["count"].matched)
    {
        try
        {
            count = std::stoi(what["count"].str());
            if (count > 0xFFFF)
                throw std::out_of_range(fb::format("count cannot be %s", what["count"].str().c_str()));
        }
        catch (std::out_of_range&)
        {
            count = 0;
            return false;
        }
    }
    else
    {
        count = 1;
    }
    return true;
}

bool fb::game::regex::match_repair_message(const std::string& message, fb::game::item::model*& item)
{
    auto& regex = fb::game::model::regex[fb::game::container::regex::TYPE::REPAIR];
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    auto all = what["all"].matched;
    if (all)
    {
        item = nullptr;
    }
    else
    {
        item = fb::game::model::items.name2item(what["name"].str());
    }

    return true;
}

bool fb::game::regex::match_deposit_money_message(const std::string& message, std::optional<uint32_t>& money)
{
    auto& regex = fb::game::model::regex[fb::game::container::regex::TYPE::DEPOSIT_MONEY];
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    try
    {
        auto all = what["all"].matched;
        if(all)
        {
            money.reset();
        }
        else
        {
            money = std::stoul(what["money"].str());
            if (money > 0xFFFFFFFF)
                throw std::out_of_range(fb::format("money cannot be %s", what["money"].str().c_str()));
        }
    }
    catch (std::out_of_range&)
    {
        money.reset();
        return false;
    }
    return true;
}

bool fb::game::regex::match_withdraw_money_message(const std::string& message, std::optional<uint32_t>& money)
{
    auto& regex = fb::game::model::regex[fb::game::container::regex::TYPE::WITHDRAW_MONEY];
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    try
    {
        auto all = what["all"].matched;
        if(all)
        {
            money.reset();
        }
        else
        {
            money = std::stoul(what["money"].str());
            if (money > 0xFFFFFFFF)
                throw std::out_of_range(fb::format("money cannot be %s", what["money"].str().c_str()));
        }
    }
    catch (std::out_of_range&)
    {
        money.reset();
        return false;
    }
    return true;
}

bool fb::game::regex::match_deposit_item_message(const std::string& message, fb::game::item::model*& item, std::optional<uint16_t>& count)
{
    auto& regex = fb::game::model::regex[fb::game::container::regex::TYPE::DEPOSIT_ITEM];
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    try
    {
        auto all = what["all"].matched;
        if(all)
        {
            count.reset();
        }
        else
        {
            count = std::stoul(what["count"].str());
            if (count > 0XFFFF)
                throw std::out_of_range(fb::format("count cannot be %s", what["count"].str().c_str()));
        }

        item = fb::game::model::items.name2item(what["name"].str());
    }
    catch (std::out_of_range&)
    {
        item = nullptr;
        count.reset();
        return false;
    }
    return true;
}

bool fb::game::regex::match_withdraw_item_message(const std::string& message, fb::game::item::model*& item, std::optional<uint16_t>& count)
{
    auto& regex = fb::game::model::regex[fb::game::container::regex::TYPE::WITHDRAW_ITEM];
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    try
    {
        auto all = what["all"].matched;
        if(all)
        {
            count.reset();
        }
        else
        {
            count = std::stoul(what["count"].str());
            if (count > 0XFFFF)
                throw std::out_of_range(fb::format("count cannot be %s", what["count"].str().c_str()));
        }

        item = fb::game::model::items.name2item(what["name"].str());
    }
    catch (std::out_of_range&)
    {
        item = nullptr;
        count.reset();
        return false;
    }
    return true;
}

bool fb::game::regex::match_sell_list(const std::string& message)
{
    auto& regex = fb::game::model::regex[fb::game::container::regex::TYPE::SELL_LIST];
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    return true;
}

bool fb::game::regex::match_buy_list(const std::string& message)
{
    auto& regex = fb::game::model::regex[fb::game::container::regex::TYPE::BUY_LIST];
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    return true;
}

bool fb::game::regex::match_sell_price(const std::string& message, fb::game::item::model*& item)
{
    auto& regex = fb::game::model::regex[fb::game::container::regex::TYPE::SELL_PRICE];
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    item = fb::game::model::items.name2item(what["name"].str());
    return true;
}

bool fb::game::regex::match_buy_price(const std::string& message, fb::game::item::model*& item)
{
    auto& regex = fb::game::model::regex[fb::game::container::regex::TYPE::BUY_PRICE];
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    item = fb::game::model::items.name2item(what["name"].str());
    return true;
}

bool fb::game::regex::match_deposited_money(const std::string& message)
{
    auto& regex = fb::game::model::regex[fb::game::container::regex::TYPE::DEPOSITED_MONEY];
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    return true;
}

bool fb::game::regex::match_rename_weapon(const std::string& message, fb::game::item::model*& item, std::string& name)
{
    auto& regex = fb::game::model::regex[fb::game::container::regex::TYPE::RENAME_WEAPON];
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    item = fb::game::model::items.name2item(what["weapon"].str());
    name = what["name"].str();
    return true;
}