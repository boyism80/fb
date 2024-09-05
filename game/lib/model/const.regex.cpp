#include <fb/game/model.h>

bool fb::model::const_value::regex::match_sell_message(const std::string& message, std::string& item, std::optional<uint16_t>& count)
{
    static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::SELL);
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    item = what["name"].str();
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

bool fb::model::const_value::regex::match_buy_message(const std::string& message, std::string& item, uint16_t& count)
{
    static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::BUY);
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    item = what["name"].str();
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

bool fb::model::const_value::regex::match_repair_message(const std::string& message, std::string& item)
{
    static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::REPAIR);
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    auto all = what["all"].matched;
    if (all)
    {
        item = std::string();
    }
    else
    {
        item = what["name"].str();
    }

    return true;
}

bool fb::model::const_value::regex::match_deposit_money_message(const std::string& message, std::optional<uint32_t>& money)
{
    static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::DEPOSIT_MONEY);
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    try
    {
        auto all = what["all"].matched;
        if (all)
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

bool fb::model::const_value::regex::match_withdraw_money_message(const std::string& message, std::optional<uint32_t>& money)
{
    static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::WITHDRAW_MONEY);
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    try
    {
        auto all = what["all"].matched;
        if (all)
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

bool fb::model::const_value::regex::match_deposit_item_message(const std::string& message, std::string& item, std::optional<uint16_t>& count)
{
    static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::DEPOSIT_ITEM);
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    try
    {
        if (what["all"].matched)
        {
            count.reset();
        }
        else if (what["count"].matched)
        {
            count = std::stoul(what["count"].str());
            if (count > 0XFFFF)
                throw std::out_of_range(fb::format("count cannot be %s", what["count"].str().c_str()));
        }
        else
        {
            count = 1;
        }

        item = what["name"].str();
    }
    catch (std::out_of_range&)
    {
        item = std::string();
        count.reset();
        return false;
    }
    return true;
}

bool fb::model::const_value::regex::match_withdraw_item_message(const std::string& message, std::string& item, std::optional<uint16_t>& count)
{
    static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::WITHDRAW_ITEM);
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    try
    {
        if (what["all"].matched)
        {
            count.reset();
        }
        else if (what["count"].matched)
        {
            count = std::stoul(what["count"].str());
            if (count > 0XFFFF)
                throw std::out_of_range(fb::format("count cannot be %s", what["count"].str().c_str()));
        }
        else
        {
            count = 1;
        }

        item = what["name"].str();
    }
    catch (std::out_of_range&)
    {
        item = std::string();
        count.reset();
        return false;
    }
    return true;
}

bool fb::model::const_value::regex::match_sell_list(const std::string& message)
{
    static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::SELL_LIST);
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    return true;
}

bool fb::model::const_value::regex::match_buy_list(const std::string& message)
{
    static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::BUY_LIST);
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    return true;
}

bool fb::model::const_value::regex::match_sell_price(const std::string& message, std::string& item)
{
    static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::SELL_PRICE);
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    item = what["name"].str();
    return true;
}

bool fb::model::const_value::regex::match_buy_price(const std::string& message, std::string& item)
{
    static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::BUY_PRICE);
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    item = what["name"].str();
    return true;
}

bool fb::model::const_value::regex::match_deposited_money(const std::string& message)
{
    static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::DEPOSITED_MONEY);
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    return true;
}

bool fb::model::const_value::regex::match_rename_weapon(const std::string& message, std::string& weapon, std::string& name)
{
    static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::RENAME_WEAPON);
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    weapon = what["weapon"].str();
    name = what["name"].str();
    return true;
}

bool fb::model::const_value::regex::match_hold_item_list(const std::string& message)
{
    static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::HOLD_ITEM_LIST);
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    return true;
}

bool fb::model::const_value::regex::match_hold_item_count(const std::string& message, std::string& item)
{
    static const auto regex = boost::xpressive::sregex::compile(fb::model::const_value::regex::HOLD_ITEM_COUNT);
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    try
    {
        item = what["name"].str();
    }
    catch (std::out_of_range&)
    {
        item = std::string();
        return false;
    }

    return true;
}