#include <fb/thread.h>

// clang-format off
IMPLEMENT_LUA_EXTENSION(fb::thread, "fb.thread")
{"assert_ptr",          fb::thread::builtin_assert_ptr},
END_LUA_EXTENSION; // clang-format on

int fb::thread::builtin_assert_ptr(lua_State* lua)
{
    auto thread = fb::lua::get(lua);
    if (thread == nullptr)
        return 0;

    auto argc = thread->argc();
    auto ist  = thread->touserdata<fb::thread>(1);
    if (ist == nullptr)
        return 0;

    auto passed = true;
    for (int i = 2; i <= argc; i++)
    {
        try
        {
            auto ptr = (void*)(uint64_t)thread->tonumber(i);
            ist->assert_ptr(ptr);
        }
        catch (std::exception&)
        {
            passed = false;
            break;
        }
    }

    thread->pushboolean(passed);
    return 1;
}