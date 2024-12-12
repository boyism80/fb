#include <fb/console.h>

using namespace fb;

console::console()
{
    static constexpr uint32_t max_width = 120;

#ifdef _WIN32
    _tty = static_cast<bool>(_isatty(_fileno(const_cast<FILE*>(stdin))));
#else
    _tty = ::isatty(::fileno(const_cast<FILE*>(stdin)));
#endif

    if (_tty)
    {
#ifdef _WIN32
        auto hwnd   = GetStdHandle(STD_OUTPUT_HANDLE);
        auto screen = CONSOLE_SCREEN_BUFFER_INFO{};
        GetConsoleScreenBufferInfo(hwnd, &screen);
        _width  = screen.dwSize.X;
        _height = screen.dwSize.Y;

        auto cursor_info = CONSOLE_CURSOR_INFO{};
        GetConsoleCursorInfo(hwnd, &cursor_info);
        cursor_info.bVisible = false;
        SetConsoleCursorInfo(hwnd, &cursor_info);
#else
        setlocale(LC_ALL, "C.UTF-8");
        initscr();
        getmaxyx(stdscr, _height, _width);
        noecho();
#endif
    }
    else
    {
        _width = max_width;
    }

    _width = max_width > _width ? max_width : _width;
}

console::~console()
{
    newline();
}

void console::raw_put(const std::string& text, uint16_t x, uint16_t y)
{
    auto _ = std::lock_guard(_mutex);

    if (_tty)
    {
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
    else
    {
        std::cout << text;
    }
}

bool console::is_tty()
{
    return _tty;
}

void console::position(uint16_t x, uint16_t y)
{
    if (!_tty)
        return;

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
    if (!_tty)
    {
        if (x != nullptr)
            *x = 0;

        if (y != nullptr)
            *y = 0;

        return;
    }

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

    if (_tty)
    {
        uint16_t y;
        position(nullptr, &y);

        position(0, y + _comment_line + 1);
        _comment_line = 0;
    }
    else
    {
        std::cout << std::endl;
    }
}

void console::clear(uint16_t line)
{
    if (!_tty)
        return;

    auto _ = std::lock_guard(_mutex);

    uint16_t y;
    position(nullptr, &y);
    position(0, y);
    raw_put(std::string(_width, ' '), 0, y);
    position(0, y);
}