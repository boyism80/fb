#include <service/gateway/service.gateway.h>

fb::gateway::service::gateway::gateway()
{
}

fb::gateway::service::gateway::~gateway()
{
}

void fb::gateway::service::gateway::assert_client(const fb::protocol::gateway::request::assert_version& request)
{
    if(request.version != 0x0226)
        throw std::runtime_error("클라이언트 버전이 맞지 않습니다.");

    if(request.national_key != 0xD7)
        throw std::runtime_error("국가가 올바르지 않습니다.");
}