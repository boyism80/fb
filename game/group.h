#ifndef __GROUP_H__
#define __GROUP_H__

#include <vector>

namespace fb { namespace game {

class session;

class group : private std::vector<session*>
{
private:
    session*                _leader;

public:
    using std::vector<session*>::begin;
    using std::vector<session*>::end;
    using std::vector<session*>::size;

private:
    group(session& leader);
    ~group();

public:
    session*                enter(session& session);
    session*                leave(session& session);
    bool                    contains(session& session);
    session&                leader() const;

public:
    static group*           create(session& leader);
    static void             destroy(fb::game::group& group);
};

} }

#endif // !__GROUP_H__
