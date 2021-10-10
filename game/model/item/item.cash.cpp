#include "model/item/item.cash.h"

const fb::game::cash::master fb::game::cash::BRONZE(0xFFFFFFFF, "엽전", 23 + 0xBFFF);
const fb::game::cash::master fb::game::cash::BRONZE_BUNDLE(0xFFFFFFFF, "엽전뭉치", 74 + 0xBFFF);
const fb::game::cash::master fb::game::cash::SILVER(0xFFFFFFFF, "은전", 70 + 0xBFFF);
const fb::game::cash::master fb::game::cash::SILVER_BUNDLE(0xFFFFFFFF, "은전뭉치", 73 + 0xBFFF);
const fb::game::cash::master fb::game::cash::GOLD(0xFFFFFFFF, "금전", 71 + 0xBFFF);
const fb::game::cash::master fb::game::cash::GOLD_BUNDLE(0xFFFFFFFF, "금덩어리", 72 + 0xBFFF);

fb::game::cash::master::master(uint32_t id, const std::string& name, uint16_t look, uint8_t color) : 
    fb::game::item::master(name, look, color, id, 0, fb::game::item::conditions(), fb::game::item::penalties::NONE, 0, fb::game::item::trade(false), fb::game::item::storage(false, 0), "", "")
{}

fb::game::cash::master::~master()
{}

fb::game::item::attrs fb::game::cash::master::attr() const
{
    return item::attrs::ITEM_ATTR_CASH;
}

fb::game::item* fb::game::cash::master::make(fb::game::item::listener* listener) const
{
    return new cash(0, listener);
}



//
// class cash
//
fb::game::cash::cash(uint32_t chunk, listener* listener) : 
    fb::game::item(nullptr, listener)
{
    this->chunk(chunk);
}

fb::game::cash::~cash()
{
}

const std::string fb::game::cash::name_styled() const
{
    std::stringstream sstream;
    sstream << this->name() << ' ' << this->_chunk << "전";

    return sstream.str();
}

uint32_t fb::game::cash::chunk() const
{
    return this->_chunk;
}

fb::game::cash* fb::game::cash::chunk(uint32_t value)
{
    this->_chunk = value;

    const fb::game::item* master = nullptr;
    if(this->_chunk == 1)
        this->_master = &fb::game::cash::BRONZE;
    else if(this->_chunk < 100)
        this->_master = &fb::game::cash::BRONZE_BUNDLE;
    else if(this->_chunk == 100)
        this->_master = &fb::game::cash::SILVER;
    else if(this->_chunk < 1000)
        this->_master = &fb::game::cash::SILVER_BUNDLE;
    else if(this->_chunk == 1000)
        this->_master = &fb::game::cash::GOLD;
    else
        this->_master = &fb::game::cash::GOLD_BUNDLE;

    if(this->_chunk == 0)
    {
        if(this->_map != nullptr)
            this->map(nullptr);

        delete this;
        return nullptr;
    }
    else
    {
        auto listener = this->get_listener<fb::game::object>();
        if(listener != nullptr)
            listener->on_show(*this, true);

        return this;
    }
}

uint32_t fb::game::cash::chunk_reduce(uint32_t value)
{
    uint32_t reduce = std::min(this->_chunk, value);

    this->chunk(this->_chunk - reduce);
    return this->_chunk;
}

bool fb::game::cash::empty() const
{
    return this->_chunk == 0;
}