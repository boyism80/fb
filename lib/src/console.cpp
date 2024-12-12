#include <fb/console.h>

using namespace fb;

console::console()
{
    static constexpr uint32_t max_width = 120;

#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO screen;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screen);

    _width  = screen.dwSize.X;
    _height = screen.dwSize.Y;
#else
    setlocale(LC_ALL, "C.UTF-8");
    initscr();
    getmaxyx(stdscr, _height, _width);
    noecho();
#endif

    _width = max_width > _width ? max_width : _width;
}

console::~console()
{
    newline();
}

void console::raw_put(const std::string& text, uint16_t x, uint16_t y)
{
    auto _ = std::lock_guard(_mutex);

#ifdef _WIN32
    DWORD written;
    ::WriteConsoleOutputCharacterA(GetStdHandle(STD_OUTPUT_HANDLE),
                                   text.c_str(),
                                   text.length(),
                                   COORD{(SHORT)x, (SHORT)y},
                                   &written);
#else
    ::mvprintw(y, x, text.c_str());
    ::refresh();
#endif
}

void console::position(uint16_t x, uint16_t y)
{
    auto _ = std::lock_guard(_mutex);

#ifdef _WIN32
    COORD pos = {x, y};
    ::SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
#else
    ::move(y, x);
#endif
}

void console::position(uint16_t* x, uint16_t* y)
{
    auto _ = std::lock_guard(_mutex);

#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO screen;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screen);

    if (x != nullptr)
        *x = screen.dwCursorPosition.X;

    if (y != nullptr)
        *y = screen.dwCursorPosition.Y;
#else
    uint16_t unused_x, unused_y;
    if (x == nullptr)
        x = &unused_x;
    if (y == nullptr)
        y = &unused_y;
    getyx(stdscr, *y, *x);
#endif
}

void console::newline()
{
    auto _ = std::lock_guard(_mutex);

    uint16_t y;
    position(nullptr, &y);

    position(0, y + _comment_line + 1);
    _comment_line = 0;
}

void console::clear(uint16_t line)
{
    auto _ = std::lock_guard(_mutex);

    uint16_t y;
    position(nullptr, &y);
    position(0, y);
    raw_put(std::string(_width, ' '), 0, y);
}