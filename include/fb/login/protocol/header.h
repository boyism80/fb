#ifndef __PROTOCOL_HEADER_H__
#define __PROTOCOL_HEADER_H__

#include <async/task.h>
#include <fb/stream_reader.h>
#include <fb/stream_writer.h>

namespace fb::protocol {

class header
{
protected:
    header() = default;

public:
    virtual ~header() = default;

public:
    virtual async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_return;
    }
    virtual async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_return;
    }
};

} // namespace fb::protocol

#endif