#ifndef __SERVICE_GATEWAY_H__
#define __SERVICE_GATEWAY_H__

#include <string>
#include <iostream>
#include <memory>
#include <fstream>
#include <json/json.h>
#include <regex>
#include <ctime>
#include <locale>
#include <codecvt>
#include <module/stream/stream.h>
#include <module/config/config.h>
#include <model/session/session.gateway.h>
#include <module/service/service.h>
#include <model/gateway/gateway.h>

namespace fb { namespace gateway { namespace service { 

class gateway : public service
{
public:
    gateway();
    ~gateway();

public:
    void                        assert_client(fb::gateway::session& session);
};

} } }
#endif // !__SERVICE_GATEWAY_H__
