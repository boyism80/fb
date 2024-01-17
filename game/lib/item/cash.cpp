#include <fb/game/item.h>

const fb::game::cash::model fb::game::cash::BRONZE(fb::game::item::model::config
    {
        {
            .name = "엽전", 
            .look = 23 + 0xBFFF,
            .color = 0
        },
        /* id */ 0xFFFFFFFF
    });
const fb::game::cash::model fb::game::cash::BRONZE_BUNDLE(fb::game::item::model::config
    {
        {
            .name = "엽전뭉치", 
            .look = 74 + 0xBFFF
        },
        /* id */ 0xFFFFFFFF
    });
const fb::game::cash::model fb::game::cash::SILVER(fb::game::item::model::config
    {
        {
            .name = "은전", 
            .look = 70 + 0xBFFF
        },
        /* id */ 0xFFFFFFFF
    });
const fb::game::cash::model fb::game::cash::SILVER_BUNDLE(fb::game::item::model::config
    {
        {
            .name = "은전뭉치", 
            .look = 73 + 0xBFFF
        },
        /* id */ 0xFFFFFFFF
    });
const fb::game::cash::model fb::game::cash::GOLD(fb::game::item::model::config
    {
        {
            .name = "금전", 
            .look = 71 + 0xBFFF
        },
        /* id */ 0xFFFFFFFF
    });
const fb::game::cash::model fb::game::cash::GOLD_BUNDLE(fb::game::item::model::config
    {
        {
            .name = "금덩어리", 
            .look = 72 + 0xBFFF
        },
        /* id */ 0xFFFFFFFF
    });

fb::game::cash::model::model(const fb::game::item::model::config& config) : fb::game::item::model(config)
{ }

fb::game::cash::model::~model()
{ }

fb::game::item::ATTRIBUTE fb::game::cash::model::attr() const
{
    return item::ATTRIBUTE::CASH;
}


//
// class cash
//
fb::game::cash::cash(fb::game::context& context, uint32_t chunk) : 
    fb::game::item(context, nullptr)
{
    this->chunk(chunk);
}

fb::game::cash::~cash()
{ }

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

    const fb::game::item* model = nullptr;
    if(this->_chunk == 1)
        this->_model = &fb::game::cash::BRONZE;
    else if(this->_chunk < 100)
        this->_model = &fb::game::cash::BRONZE_BUNDLE;
    else if(this->_chunk == 100)
        this->_model = &fb::game::cash::SILVER;
    else if(this->_chunk < 1000)
        this->_model = &fb::game::cash::SILVER_BUNDLE;
    else if(this->_chunk == 1000)
        this->_model = &fb::game::cash::GOLD;
    else
        this->_model = &fb::game::cash::GOLD_BUNDLE;

    if(this->_chunk == 0)
    {
        this->destroy();
        return nullptr;
    }
    else
    {
        auto listener = this->get_listener<fb::game::object>();
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