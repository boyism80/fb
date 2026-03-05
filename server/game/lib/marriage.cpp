#include <fb/game/marriage.h>
#include <fb/protocol/flatbuffer/protocol.h>

namespace fb::game {

fb::protocol::internal::Marriage marriage::to_protocol() const
{
    return fb::protocol::internal::Marriage{spouse_id,
                                            spouse_name,
                                            fb::model::datetime() >= remarriage_after ? ""
                                                                                      : remarriage_after.to_string(),
                                            divorce_count};
}

} // namespace fb::game
