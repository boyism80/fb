#ifndef __FB_PROTOCOL___
#define __FB_PROTOCOL___

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/flatbuffer/origin/nullable_generated.h>
#include <fb/protocol/flatbuffer/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace protocol { 


class nullable_uint32
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::nullable_uint32;

public:
    uint32_t value;

public:
    nullable_uint32()
    { }

    nullable_uint32(const nullable_uint32& x)
        : value(x.value)
    { }

    nullable_uint32(uint32_t value)
        : value(value)
    { }
    nullable_uint32(const fb::protocol::origin::nullable_uint32& raw)
        : value(raw.value())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::origin::Createnullable_uint32(builder,
            this->value);
    }

};

class nullable_uint16
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::nullable_uint16;

public:
    uint16_t value;

public:
    nullable_uint16()
    { }

    nullable_uint16(const nullable_uint16& x)
        : value(x.value)
    { }

    nullable_uint16(uint16_t value)
        : value(value)
    { }
    nullable_uint16(const fb::protocol::origin::nullable_uint16& raw)
        : value(raw.value())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::origin::Createnullable_uint16(builder,
            this->value);
    }

};

class nullable_bool
{
public:
    static inline FlatBufferProtocolType FlatBufferProtocolType = FlatBufferProtocolType::nullable_bool;

public:
    bool value;

public:
    nullable_bool()
    { }

    nullable_bool(const nullable_bool& x)
        : value(x.value)
    { }

    nullable_bool(bool value)
        : value(value)
    { }
    nullable_bool(const fb::protocol::origin::nullable_bool& raw)
        : value(raw.value())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::origin::Createnullable_bool(builder,
            this->value);
    }

};

 }  } 

#endif