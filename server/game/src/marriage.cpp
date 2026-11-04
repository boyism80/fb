#include <fb/game/marriage.h>

#include <fb/protocol/flatbuffer/protocol.h>

namespace fb::game {

marriage::marriage(const fb::protocol::internal::Marriage& row) :
    spouse_id(row.spouse_id),
    spouse_name(row.spouse_name),
    remarriage_after(fb::model::datetime(row.remarriage_after)),
    divorce_count(row.divorce_count)
{ }

} // namespace fb::game
