#ifndef __LEAK_H__
#define __LEAK_H__

#if defined DEBUG | defined _DEBUG

#include <crtdbg.h>

static class leak
{
public:
    leak()
    {
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    }

    ~leak()
    {
        _CrtDumpMemoryLeaks();
    }
} mleak;

#endif
#endif // !__LEAK_H__
