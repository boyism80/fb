#ifndef __PROTOCOL_HEADER_H__
#define __PROTOCOL_HEADER_H__

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
    virtual void serialize(fb::stream_writer<big_endian>& writer) const
    { }
    virtual void deserialize(fb::stream_reader<big_endian>& reader)
    { }
};

} // namespace fb::protocol

#endif
