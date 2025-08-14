#ifndef __FB_LOGIN_HANDLER_H__
#define __FB_LOGIN_HANDLER_H__

/**
 * @file    handler.h
 * @brief   Central include for all login server packet handlers
 * @author  FB Development Team
 */

#include <fb/login/handler/protocol/agreement.h>
#include <fb/login/handler/protocol/create_account.h>
#include <fb/login/handler/protocol/complete.h>
#include <fb/login/handler/protocol/change_password.h>
#include <fb/login/handler/protocol/login.h>

#include <fb/login/handler/amqp/shutdown.h>

#endif // !__FB_LOGIN_HANDLER_H__
