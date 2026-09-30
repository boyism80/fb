#ifndef __FB_WINSOCK_H__
#define __FB_WINSOCK_H__

#ifdef _WIN32

// windows.h includes winsock.h unless this is set first. winsock.h and
// winsock2.h define the same sockets API, and linking both wsock32 and
// ws2_32 fails. Always include winsock2.h before windows.h.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <winsock2.h>

#endif

#endif
