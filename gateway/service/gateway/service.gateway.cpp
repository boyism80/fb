#include <service/gateway/service.gateway.h>

fb::gateway::service::gateway::gateway()
{
}

fb::gateway::service::gateway::~gateway()
{
}

void fb::gateway::service::gateway::assert_client(const fb::protocol::request::gateway::assert_version& request)
{
    if(request.version != 0x0226)
        throw std::exception("Ŭ���̾�Ʈ ������ ���� �ʽ��ϴ�.");

    if(request.national_key != 0xD7)
        throw std::exception("������ �ùٸ��� �ʽ��ϴ�.");
}