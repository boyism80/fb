#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

#include <fb/dead_lock.h>

namespace fb {

class lock_error : public std::runtime_error
{
public:
    lock_error();
    ~lock_error() = default;
};

class concurrent
{
protected:
    fb::dead_lock_detector root;

protected:
    concurrent()  = default;
    ~concurrent() = default;

protected:
    void add(fb::dead_lock_detector& node);
    void assert_dead_lock(const fb::dead_lock_detector& node);
};

} // namespace fb

#endif // !__CONCURRENT_H__