#ifndef __LEAK_H__
#define __LEAK_H__

#if defined DEBUG | defined _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#define new     DBG_NEW

static class leak
{
public:
    leak()
    {
        //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    }

    ~leak()
    {
        // _CRTDBG_LEAK_CHECK_DF 플래그 설정시 맨 마지막에 invoke해줌
        _CrtDumpMemoryLeaks();
    }
} mleak;

#endif
#endif // !__LEAK_H__
