#include <fb/core/dead_lock.h>

fb::dead_lock_detector::dead_lock_detector(const std::string& data, const std::shared_ptr<fb::dead_lock_detector>& parent) : fb::mst<std::string>(data, parent)
{ }

void fb::dead_lock_detector::assert_circulated_lock() const
{
    auto errors = std::vector<std::string>();

    this->travel([this, &errors](auto& route)
    {
        for(auto i1 = route.cbegin(), last = std::prev(route.cend()); i1 != last; i1++)
        {
            for(auto i2 = std::next(i1); i2 != route.cend(); i2++)
            {
                auto begin = *i1;
                auto end = *i2;

                if(begin->data == end->data)
                {
                    auto sstream = std::stringstream();
                    for (auto n : route)
                    {
                        if (n == begin || n == end)
                            sstream << '[' << n->data << ']';
                        else
                            sstream << n->data;

                        if (n != *last)
                            sstream << '-';
                    }
                    errors.push_back(sstream.str());
                }
            }
        }

        return false;
    });

    if (errors.size() > 0)
    {
        auto sstream = std::stringstream();
        sstream << "Circulated lock detected : " << boost::algorithm::join(errors, ", ");
        throw std::runtime_error(sstream.str());
    }
}

void fb::dead_lock_detector::assert_dead_lock(const fb::dead_lock_detector* node) const
{
    auto errors = std::vector<std::string>();
    node->travel([this, &errors](const auto& route)
    {
        for(auto i1 = route.cbegin(), last = std::prev(route.cend()); i1 != last; i1++)
        {
            for(auto i2 = std::next(i1); i2 != route.cend(); i2++)
            {
                auto begin = *i1;
                auto end = *i2;

                auto found = this->search(end);
                if (found != nullptr && found->search(begin) != nullptr)
                {
                    auto sstream = std::stringstream();
                    for (auto n : route)
                    {
                        if (n == begin || n == end)
                            sstream << '[' << n->data << ']';
                        else
                            sstream << n->data;

                        if (n != *last)
                            sstream << '-';
                    }
                    errors.push_back(sstream.str());
                }
            }
        }

        return false;
    });

     if (errors.size() > 0)
     {
         auto sstream = std::stringstream();
         sstream << "DeadLock detected : " << boost::algorithm::join(errors, ", ");
         throw std::runtime_error(sstream.str());
     }
}