#ifndef __LEAK_H__
#define __LEAK_H__

#if defined DEBUG || defined _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#define new     DBG_NEW

// Dumping from this object's destructor would run before statics of other translation units (model tables) are
// destroyed and report them all; the CRT flag dumps after every static destructor has run.
static class leak
{
public:
    leak()
    {
        _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
    }
} mleak;

#endif
#endif // !__LEAK_H__
