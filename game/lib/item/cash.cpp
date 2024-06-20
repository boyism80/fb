#include <fb/game/item.h>

fb::game::cash::cash(fb::game::context& context, uint32_t value) : 
    fb::model::item(context, nullptr)
{
    this->chunk(value);
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

    const fb::model::item* model = nullptr;
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