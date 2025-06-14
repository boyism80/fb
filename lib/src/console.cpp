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

void console::progress(const std::string& text, float progress)
{
    auto _ = std::lock_guard(_mutex);

    if (!_tty)
    {
        std::cout << std::format("{:>7.2f}% {}", progress, text) << std::endl;
    }
    else
    {
#ifdef _WIN32
        static auto fill_text       = std::string("-");
        static auto lead_characters = std::vector<std::string>{"-"};
#else
        static auto fill_text       = std::string("█");
        static auto lead_characters = std::vector<std::string>{" ", "▏", "▎", "▍", "▌", "▋", "▊", "▉"};
#endif
        auto os = std::ostringstream{};

        auto width           = 50;
        auto value           = std::min(1.0f, std::max(0.0f, progress / 100.0f));
        auto whole_width     = std::floor(value * width);
        auto remainder_width = fmod(value * width, 1.0f);
        auto part_width      = std::floor(remainder_width * lead_characters.size());
        auto lead_text       = lead_characters[size_t(part_width)];

        os << '\r' << '[';
        for (size_t i = 0; i < whole_width; ++i)
            os << fill_text;
        os << lead_text;
        for (size_t i = 0; i < width - whole_width; ++i)
            os << " ";
        os << "]";

        os << std::format(" {:>7.2f}% {}", progress, text);
        std::cout << os.str();
        std::cout.flush();
    }
}