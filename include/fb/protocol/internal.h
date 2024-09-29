#ifndef __FB_PROTOCOL_INTERNAL___
#define __FB_PROTOCOL_INTERNAL___

#include "flatbuffers/flatbuffers.h"
#include <fb/protocol/origin/internal_generated.h>
#include <fb/protocol/protocol_type.h>
#include <string>
#include <vector>

namespace fb { namespace protocol { namespace internal { 

enum class Service : int8_t
{
    Gateway = fb::protocol::internal::origin::Service::Service_Gateway,
    Login = fb::protocol::internal::origin::Service::Service_Login,
    Game = fb::protocol::internal::origin::Service::Service_Game
};
enum class TransferResult : int8_t
{
    Success = fb::protocol::internal::origin::TransferResult::TransferResult_Success,
    Failed = fb::protocol::internal::origin::TransferResult::TransferResult_Failed,
    LoggedIn = fb::protocol::internal::origin::TransferResult::TransferResult_LoggedIn
};


 }  }  } 

#endif