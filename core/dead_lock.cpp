#include <fb/core/dead_lock.h>

fb::dead_lock_detector::dead_lock_detector() : fb::mst<std::string>(dead_lock_detector::initializer, dead_lock_detector::comparer)
{}

fb::dead_lock_detector::dead_lock_detector(const std::string& data, const dead_lock_detector* parent) : fb::mst<std::string>(data, parent)
{ }

void fb::dead_lock_detector::assert_circulated_lock() const
{
    auto errors = std::vector<std::string>();

    this->travel([this, &errors](auto& route) -> bool
    {
        for(auto i1 = route.cbegin(), last = std::prev(route.cend()); i1 != last; i1++)
        {
            for(auto i2 = std::next(i1); i2 != route.cend(); i2++)
            {
                auto& begin = (*i1).get();
                auto& end = (*i2).get();

                if(begin.data == end.data)
                {
                    auto sstream = std::stringstream();
                    for (auto& n : route)
                    {
                        auto& key = n.get().data;
                        if (key == begin.data || key == end.data)
                            sstream << '[' << key << ']';
                        else
                            sstream << key;

                        if (key != (*last).get().data)
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

fb::mst<std::string>& fb::dead_lock_detector::add(fb::mst<std::string>& node)
{
    auto found = static_cast<fb::dead_lock_detector*>(this->search(node));
    if (found == nullptr)
    {
        auto& base = static_cast<fb::mst<std::string>&>(*this);
        return base.add(node);
    }
    else
    {
        // µð¹ö±ëÇØºÁ¾ßÇÔ
        for (auto n : *this)
        {
            if (n == nullptr)
                continue;

            found->add(*n);
        }
    }
    return *this;
}

void fb::dead_lock_detector::assert_dead_lock(const fb::dead_lock_detector& node) const
{
    auto errors = std::vector<std::string>();
    this->travel([this, &errors](auto& route) -> bool
    {
        for(auto i1 = route.cbegin(), last = std::prev(route.cend()); i1 != last; i1++)
        {
            for(auto i2 = std::next(i1); i2 != route.cend(); i2++)
            {
                auto& begin = (*i1).get();
                auto& end = (*i2).get();

                auto found = this->search(end);
                if (found != nullptr && found->search(begin) != nullptr)
                {
                    auto sstream = std::stringstream();
                    for (auto& n : route)
                    {
                        auto& key = n.get().data;
                        if (key == begin.data || key == end.data)
                            sstream << '[' << key << ']';
                        else
                            sstream << key;

                        if (key != (*last).get().data)
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

const std::string& fb::dead_lock_detector::initializer()
{
    return dead_lock_detector::DUMMY;
}

bool fb::dead_lock_detector::comparer(const std::string& val1, const std::string& val2)
{
    return val1 == val2;
}