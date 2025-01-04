#include <fb/gateway/util.h>

void fb::gateway::util::assert_client(const fb::protocol::gateway::request::version& request)
{
    if (request.v != 550)
        throw std::runtime_error("클라이언트 버전이 맞지 않습니다.");

    if (request.nation != 0xD7)
        throw std::runtime_error("국가가 올바르지 않습니다.");
}