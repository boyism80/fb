#ifndef FB_LOGIN_HANDLER_H
#define FB_LOGIN_HANDLER_H

// Protocol handlers
#include <fb/login/handler/protocol/agreement.h>
#include <fb/login/handler/protocol/change_password.h>
#include <fb/login/handler/protocol/complete.h>
#include <fb/login/handler/protocol/create_account.h>
#include <fb/login/handler/protocol/login.h>
#include <fb/login/handler/protocol/meta_dat.h>

// AMQP handlers
#include <fb/login/handler/amqp/shutdown.h>

#endif // FB_LOGIN_HANDLER_H
