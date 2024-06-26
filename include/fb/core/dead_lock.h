#ifndef __DEAD_LOCK_H__
#define __DEAD_LOCK_H__

#include <fb/core/mst.h>

namespace fb {

class dead_lock_detector : public fb::mst<std::string>
{
private:
    inline static std::string   DUMMY = std::string();

public:
    dead_lock_detector();
    dead_lock_detector(const dead_lock_detector&) = delete;
    dead_lock_detector(const std::string& data, const dead_lock_detector* parent);
    ~dead_lock_detector() = default;

public:
    void                        assert_circulated_lock() const;
    void                        assert_dead_lock(const dead_lock_detector& node) const;

public:
    static const std::string&   initializer();
    static bool                 comparer(const std::string& val1, const std::string& val2);
};

}

#endif