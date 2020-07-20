#include <service/gateway/service.gateway.h>

fb::gateway::service::gateway::gateway()
{
}

fb::gateway::service::gateway::~gateway()
{
}

void fb::gateway::service::gateway::assert_client(fb::gateway::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd     = istream.read_u8();
    auto                    version = istream.read_u16();
    if(version != 0x0226)
        throw std::exception("Ŭ���̾�Ʈ ������ ���� �ʽ��ϴ�.");

    auto                    national_key    = istream.read_u8();
    if(national_key != 0xD7)
        throw std::exception("������ �ùٸ��� �ʽ��ϴ�.");
}