#ifndef __FB_PROTOCOL_DB___
#define __FB_PROTOCOL_DB___

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/origin/db_generated.h>
#include <fb/protocol/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace protocol { namespace db { 


class nullable_uint
{

public:
    uint32_t value;

public:
    nullable_uint()
    { }

    nullable_uint(const nullable_uint& x)
        : value(x.value)
    { }

    nullable_uint(uint32_t value)
        : value(value)
    { }
    nullable_uint(const fb::protocol::db::origin::nullable_uint& raw)
        : value(raw.value())
    {
    }


public:
    auto Build(flatbuffers::FlatBufferBuilder& builder) const
    {
        return fb::protocol::db::origin::Createnullable_uint(builder,
            this->value);
    }

};

 }  }  } 

#endif