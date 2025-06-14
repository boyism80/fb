#ifdef _WIN32
#include <Windows.h>
#endif
#include <fb/console.h>

using namespace fb;

#ifdef _WIN32
bool ::SetConsoleIcon(int id)
{
    auto hwnd = ::GetConsoleWindow();
    if (hwnd == nullptr)
        return false;

    auto icon = ::LoadIcon(::GetModuleHandle(NULL), MAKEINTRESOURCE(id));
    if (icon == nullptr)
        return false;

    ::SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
    ::SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
    return true;
}
#endif

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

    // hide cursor
    if (_tty)
        std::cout << "\x1B[?25l";

    _width = max_width > _width ? max_width : _width;
}

console::~console()
{
    newline();
}

bool console::is_tty()
{
    return _tty;
}

void console::newline()
{
    auto _ = std::lock_guard(_mutex);
    if (!_tty)
        return;

    std::cout << std::format("\033[{}B", ++_comment_line) << '\r';
    _comment_line = 0;
}

void console::clear()
{
    auto _ = std::lock_guard(_mutex);
    if (!_tty)
        return;

    std::cout << "\x1b[2K";
}

void console::save_point()
{
    auto _ = std::lock_guard(_mutex);
    if (!_tty)
        return;

    std::cout << "\033[s";
}

void console::restore_point()
{
    auto _ = std::lock_guard(_mutex);
    if (!_tty)
        return;

    std::cout << "\033[u";
}

void console::up(uint8_t line)
{
    auto _ = std::lock_guard(_mutex);
    if (!_tty)
        return;

    std::cout << std::format("\033[{}A", line);
}

void console::down(uint8_t line)
{
    auto _ = std::lock_guard(_mutex);
    if (!_tty)
        return;

    std::cout << std::format("\033[{}B", line);
}