#ifndef __CONSOLE_H__
#define	__CONSOLE_H__

#ifdef  _WIN32
#include <Windows.h>
#include <string>

#define	CONSOLE_TITLE	"Private kingdom of the wind - http://cshyeon.com"

bool SetConsoleIcon(int id);
#endif

#endif // !__CONSOLE_H__