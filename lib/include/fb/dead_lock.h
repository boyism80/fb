#ifndef __DEAD_LOCK_H__
#define __DEAD_LOCK_H__

#include <fb/mst.h>

namespace fb {

class dead_lock_detector : public fb::mst<std::string>
{
public:
    dead_lock_detector(const std::string& data = "", const dead_lock_detector* parent = nullptr);
    dead_lock_detector(const dead_lock_detector&) = delete;
    ~dead_lock_detector()                         = default;

public:
    void add(dead_lock_detector& node);

public:
    void assert_circulated_lock() const;
    void assert_dead_lock(const dead_lock_detector& node) const;

public:
    bool compare(const std::string& val) const;
};

} // namespace fb

#endif