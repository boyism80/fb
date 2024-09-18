#ifndef __FB_PROTOCOL_INTERNAL___
#define __FB_PROTOCOL_INTERNAL___

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/flatbuffer/origin/internal_generated.h>
#include <fb/protocol/flatbuffer/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace protocol { namespace internal { 

enum class Service : int8_t
{
    Gateway = fb::protocol::internal::origin::Service::Service_Gateway,
    Login = fb::protocol::internal::origin::Service::Service_Login,
    Game = fb::protocol::internal::origin::Service::Service_Game
};


 }  }  } 

#endif