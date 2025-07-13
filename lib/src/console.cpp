#ifdef _WIN32
#include <Windows.h>
#endif
#include <fb/console.h>

using namespace fb;

#ifdef _WIN32
bool ::set_console_icon(int id)
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

    // Detect system TTY capability
#ifdef _WIN32
    _system_tty = static_cast<bool>(_isatty(_fileno(const_cast<FILE*>(stdin))));
#else
    _system_tty = ::isatty(::fileno(const_cast<FILE*>(stdin)));
#endif

    // Set default mode based on system TTY capability
    _mode = _system_tty ? mode::tty : mode::plain;

    if (_system_tty)
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

    // Initialize both policies
    _policies[mode::tty]   = std::make_unique<tty_policy>();
    _policies[mode::plain] = std::make_unique<plain_policy>();

    // Set current policy based on mode
    update_policy();

    // hide cursor if TTY mode is enabled
    if (_mode == mode::tty)
        std::cout << "\x1B[?25l";

    _width = max_width > _width ? max_width : _width;
}

console::~console()
{
    newline();
}

bool console::is_tty()
{
    return _system_tty;
}

console::mode console::get_mode()
{
    auto _ = std::lock_guard(_mutex);
    return _mode;
}

void console::set_mode(console::mode new_mode)
{
    auto _ = std::lock_guard(_mutex);
    _mode  = new_mode;
    update_policy();
}

void console::update_policy()
{
    if (_system_tty && _mode == mode::tty)
        _current_policy = _policies[mode::tty].get();
    else
        _current_policy = _policies[mode::plain].get();
}

std::string console::colorize(color color)
{
    auto it = _color_codes.find(color);
    return (it != _color_codes.end()) ? it->second : _color_codes[color::reset];
}

void console::newline()
{
    auto _ = std::lock_guard(_mutex);
    _current_policy->newline();
}

void console::clear()
{
    auto _ = std::lock_guard(_mutex);
    _current_policy->clear();
}

void console::save_point()
{
    auto _ = std::lock_guard(_mutex);
    _current_policy->save_point();
}

void console::restore_point()
{
    auto _ = std::lock_guard(_mutex);
    _current_policy->restore_point();
}

void console::up(uint8_t line)
{
    auto _ = std::lock_guard(_mutex);
    _current_policy->up(line);
}

void console::down(uint8_t line)
{
    auto _ = std::lock_guard(_mutex);
    _current_policy->down(line);
}

void console::progress(const std::string& text, float progress)
{
    auto _ = std::lock_guard(_mutex);
    _current_policy->progress(text, progress);
}

// Parent class common implementation
std::string console::console_policy::get_aligned_text(align_type align, const std::string& text)
{
    auto padding = uint16_t{0};
    switch (align)
    {
    case align_type::right:
        padding = _width - text.size() - 1;
        break;
    case align_type::center:
        padding = (_width - text.size() - 1) / 2;
        break;
    default:
        padding = 0;
        break;
    }
    return std::string(padding, ' ') + text;
}

// TTY policy implementation methods
void console::tty_policy::put(align_type align, color color, const std::string& text)
{
    auto padded = get_aligned_text(align, text);
    save_point();
    clear();
    std::cout << "\r" << colorize(color) << padded << colorize(color::reset);
    restore_point();
}

void console::tty_policy::puts(align_type align, color color, const std::string& text)
{
    put(align, color, text);
    newline();
}

void console::tty_policy::comment(color color, const std::string& text)
{
    save_point();
    down(++this->_comment_line);
    std::cout << "\r" << colorize(color) << text << colorize(color::reset);
    restore_point();
}

void console::tty_policy::newline()
{
    std::cout << std::format("\033[{}B", ++this->_comment_line) << '\r';
    this->_comment_line = 0;
}

void console::tty_policy::clear()
{
    std::cout << "\x1b[2K";
}

void console::tty_policy::save_point()
{
    std::cout << "\033[s";
}

void console::tty_policy::restore_point()
{
    std::cout << "\033[u";
}

void console::tty_policy::up(uint8_t line)
{
    std::cout << std::format("\033[{}A", line);
}

void console::tty_policy::down(uint8_t line)
{
    std::cout << std::format("\033[{}B", line);
}

void console::tty_policy::progress(const std::string& text, float progress)
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

// Plain policy implementation methods
void console::plain_policy::put(align_type align, color color, const std::string& text)
{
    auto padded = get_aligned_text(align, text);
    std::cout << colorize(color) << padded << colorize(color::reset) << std::endl;
}

void console::plain_policy::puts(align_type align, color color, const std::string& text)
{
    put(align, color, text);
}

void console::plain_policy::comment(color color, const std::string& text)
{
    std::cout << colorize(color) << text << colorize(color::reset) << std::endl;
}

void console::plain_policy::newline()
{
    // Plain mode: newline does nothing
}

void console::plain_policy::clear()
{
    // Plain mode: clear does nothing
}

void console::plain_policy::save_point()
{
    // Plain mode: save_point does nothing
}

void console::plain_policy::restore_point()
{
    // Plain mode: restore_point does nothing
}

void console::plain_policy::up(uint8_t line)
{
    // Plain mode: up does nothing
}

void console::plain_policy::down(uint8_t line)
{
    // Plain mode: down does nothing
}

void console::plain_policy::progress(const std::string& text, float progress)
{
    std::cout << std::format("{:>7.2f}% {}", progress, text) << std::endl;
}