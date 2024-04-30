#include <fb/game/regex.h>

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
        count = std::stoi(what["count"].str());
        if (count <= 0)
            return false;
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
        count = std::stoi(what["count"].str());
        if (count <= 0)
            return false;
    }
    else
    {
        count = 1;
    }
    return true;
}

bool fb::game::regex::match_repair_message(const std::string& message, fb::game::item::model*& item, bool& all)
{
    auto& regex = fb::game::model::regex[fb::game::container::regex::TYPE::REPAIR];
    auto what = boost::xpressive::smatch();
    if (boost::xpressive::regex_search(message, what, regex) == false)
        return false;

    all = what["all"].matched;
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