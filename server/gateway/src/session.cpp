#include <fb/gateway/session.h>

using namespace fb::gateway;

session::session(fb::protocol::CLIENT_VERSION client_version) :
    client_version(client_version)
{ }

session::~session()
{ }
