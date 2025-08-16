#include <fb/game/server.h>
#include <fb/game/item.h>
#include <fb/model/model.h>

ITEM_ATTRIBUTE fb::model::item::attr() const
{
    auto attr = ITEM_ATTRIBUTE::NONE;
    if (this->capacity > 1)
        attr = ITEM_ATTRIBUTE((uint32_t)attr | (uint32_t)ITEM_ATTRIBUTE::BUNDLE);
    return attr;
}

bool fb::model::item::attr(ITEM_ATTRIBUTE flag) const
{
    return ((uint32_t)this->attr() & (uint32_t)flag) == (uint32_t)flag;
}

fb::model::item* fb::model::__item::name2item(const std::string& name) const
{
    for (auto& [k, v] : *this)
    {
        if (v.name == name)
            return &v;
    }

    return nullptr;
}

std::shared_ptr<fb::game::item> fb::model::item::make(fb::game::server& server, uint16_t count) const
{
    return server.make<fb::game::item>(*this, fb::game::item::initial_params{.count = count});
}

std::shared_ptr<fb::game::item> fb::model::cash::make(fb::game::server& server, uint16_t count) const
{
    return server.make<fb::game::cash>(count);
}

std::shared_ptr<fb::game::item> fb::model::consume::make(fb::game::server& server, uint16_t count) const
{
    return server.make<fb::game::consume>(*this, count);
}

std::shared_ptr<fb::game::item> fb::model::pack::make(fb::game::server& server, uint16_t count) const
{
    return server.make<fb::game::pack>(*this);
}

std::shared_ptr<fb::game::item> fb::model::weapon::make(fb::game::server& server, uint16_t count) const
{
    return server.make<fb::game::weapon>(*this);
}

std::shared_ptr<fb::game::item> fb::model::armor::make(fb::game::server& server, uint16_t count) const
{
    return server.make<fb::game::armor>(*this);
}

std::shared_ptr<fb::game::item> fb::model::helmet::make(fb::game::server& server, uint16_t count) const
{
    return server.make<fb::game::helmet>(*this);
}

std::shared_ptr<fb::game::item> fb::model::shield::make(fb::game::server& server, uint16_t count) const
{
    return server.make<fb::game::shield>(*this);
}

std::shared_ptr<fb::game::item> fb::model::ring::make(fb::game::server& server, uint16_t count) const
{
    return server.make<fb::game::ring>(*this);
}

std::shared_ptr<fb::game::item> fb::model::auxiliary::make(fb::game::server& server, uint16_t count) const
{
    return server.make<fb::game::auxiliary>(*this);
}