#ifndef __GROUP_H__
#define __GROUP_H__

#include <vector>

namespace fb { namespace game {

class session;

class group
{
private:
    session*                _leader;
    std::vector<session*>   _members;

public:
    group(session& leader);
    ~group();

public:
    bool                    add(session& session);
    bool                    remove(session& session);
};

} }

#endif // !__GROUP_H__
