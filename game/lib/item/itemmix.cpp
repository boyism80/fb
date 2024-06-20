#include <fb/game/item.h>
#include <fb/game/session.h>
#include <fb/game/model.h>

fb::game::itemmix::builder::builder(session& owner) : 
    _owner(owner)
{ }

fb::game::itemmix::builder::~builder()
{ }

fb::game::itemmix::builder& fb::game::itemmix::builder::push(uint8_t index)
{
    auto item = this->_owner.items[index];
    if(item != nullptr)
        this->push_back(item);
    
    return *this;
}

bool fb::game::itemmix::builder::mix()
{
    auto listener = this->_owner.get_listener<fb::game::session>();

    try
    {
        auto                    itemmix = fb::game::old_model::mixes.find(*this);
        if(itemmix == nullptr)
            throw itemmix::no_match_exception();

        auto                    free_size = this->_owner.items.free_size();
        if(int(itemmix->success.size()) - int(itemmix->require.size()) > free_size)
            throw item::full_inven_exception();


        for(auto& require : itemmix->require)
        {
            auto                index = this->_owner.items.index(require.item);
            if(index == 0xFF)
                return true;

            auto item = this->_owner.items.remove(index, require.count);
            if(item != nullptr)
                item->destroy();
        }

        auto                success = (std::rand() % 100) < itemmix->percentage;
        if(success)
        {
            for(auto& success : itemmix->success)
            {
                auto        item = success.item->make(this->_owner.context);
                item->count(success.count);
                this->_owner.items.add(item);
            }

            if(listener != nullptr)
                listener->on_notify(this->_owner, fb::game::message::mix::SUCCESS);
        }
        else
        {
            for(auto& failed : itemmix->failed)
            {
                auto        item = static_cast<fb::game::item*>(failed.item->make(this->_owner.context));
                item->count(failed.count);
                this->_owner.items.add(item);
            }

            if(listener != nullptr)
                listener->on_notify(this->_owner, fb::game::message::mix::FAILED);
        }

        return true;
    }
    catch(std::exception& e)
    { 
        if(listener != nullptr)
            listener->on_notify(this->_owner, e.what());
        return false;
    }
}




bool fb::game::itemmix::contains(const item* item) const
{
    for(auto i : this->require)
    {
        if(item->based() == i.item && item->count() >= i.count)
            return true;
    }

    return false;
}

void fb::game::itemmix::require_add(fb::model::item* item, uint32_t count)
{
    this->require.push_back(element(item, count));
}

void fb::game::itemmix::success_add(fb::model::item* item, uint32_t count)
{
    this->success.push_back(element(item, count));
}

void fb::game::itemmix::failed_add(fb::model::item* item, uint32_t count)
{
    this->failed.push_back(element(item, count));
}

bool fb::game::itemmix::matched(const std::vector<item*>& items) const
{
    if(this->require.size() != items.size())
        return false;

    for(const auto i : items)
    {
        if(this->contains(i) == false)
            return false;
    }

    return true;
}