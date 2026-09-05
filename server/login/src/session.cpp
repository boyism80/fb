#include <fb/login/session.h>

using namespace fb::login;

session::session(fb::protocol::CLIENT_VERSION client_version, fb::protocol::CLIENT_UI_MODE ui_mode) :
    client_version(client_version),
    ui_mode(ui_mode)
{ }

session::~session()
{ }
