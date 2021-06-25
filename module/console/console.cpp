#include "console.h"

std::unique_ptr<fb::console> fb::console::_ist;

#ifdef _WIN32
bool SetConsoleIcon(int id)
{
    auto hwnd = ::GetConsoleWindow();
    if(hwnd == nullptr)
        return false;

    auto icon = ::LoadIcon(::GetModuleHandle(NULL), MAKEINTRESOURCE(id));
    if(icon == nullptr)
        return false;

    ::SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
    ::SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
    return true;
}
#endif

fb::console::console()
#ifdef _WIN32
    : _x(0), _y(0)
#endif
{
#ifdef _WIN32
    this->_stdout = GetStdHandle(STD_OUTPUT_HANDLE);

    // get size
    CONSOLE_SCREEN_BUFFER_INFO      screen;
    GetConsoleScreenBufferInfo(this->_stdout, &screen);
    this->_width = screen.dwSize.X;
    this->_height = screen.dwSize.Y;

    CONSOLE_CURSOR_INFO             cursor;
    GetConsoleCursorInfo(this->_stdout, &cursor);
    cursor.bVisible = false;
    SetConsoleCursorInfo(this->_stdout, &cursor);
#else
    setlocale(LC_ALL, "C.UTF-8");

    initscr();
    getmaxyx(stdscr, this->_height, this->_width);
    noecho();
#endif
}

fb::console::~console()
{
#ifdef __linux__
    endwin();
#endif
}

std::string fb::console::format(const std::string& f, va_list* args)
{
    va_list clone;
    va_copy(clone, *args);
    auto size = vsnprintf(nullptr, 0, f.c_str(), clone) + 1;
    if(size == -1)
        throw std::exception();
    va_end(clone);

    auto buffer = new char[size];
    if(buffer == nullptr)
        throw std::exception();

    if(vsprintf(buffer, f.c_str(), *args) == -1)
        throw std::exception();

    auto result = std::string(buffer);
    delete[] buffer;

    return result;
}

bool fb::console::line(uint16_t x, uint16_t y, uint16_t width, char content, char side)
{
    if(width < 3)
        return false;

    int  offset = 0;
    char buffer[256] = {0,};

    buffer[offset] = side;                          offset++;
    std::memset(buffer+offset, content, width-1);   offset += (width-1);
    buffer[offset] = side;

    this->cursor(x, y).puts(buffer);
    return true;
}

fb::console& fb::console::put(const char* format, ...)
{
    va_list                 args;

    va_start(args, format);
    auto                    combined = this->format(format, &args);
    va_end(args);

    uint16_t                x, y;
    this->cursor(&x, &y);

#ifdef _WIN32
    DWORD                   written;
    this->_x += WriteConsoleOutputCharacterA(this->_stdout, combined.c_str(), combined.length(), COORD {(SHORT)x, (SHORT)y}, &written) == 0 ? 0 : combined.length();
#else
    mvprintw(y, x, combined.c_str());
    refresh();
#endif

    this->next();
    return *this;
}

fb::console& fb::console::puts(const char* format, ...)
{
    va_list                 args;

    va_start(args, format);
    auto                    combined = this->format(format, &args);
    va_end(args);

    uint16_t                x, y;
    this->cursor(&x, &y);

#ifdef _WIN32
    DWORD                   written;
    this->_x += WriteConsoleOutputCharacterA(this->_stdout, combined.c_str(), combined.length(), COORD {(SHORT)x, (SHORT)y}, &written) == 0 ? 0 : combined.length();
#else
    mvprintw(y, x, combined.c_str());
    refresh();
#endif

    this->trim().next();
    return *this;
}

fb::console& fb::console::clear(uint16_t x, uint16_t y)
{
#ifdef _WIN32
    COORD                   coord = {x, y};
    DWORD                   written;
    FillConsoleOutputCharacterA(this->_stdout, ' ', this->_width - x, coord, &written);
#else
    uint16_t                _x, _y;
    this->cursor(&_x, &_y);

    this->cursor(x, y);
    clrtoeol();

    this->cursor(_x, _y);
#endif

    return *this;
}

fb::console& fb::console::trim()
{
    uint16_t x, y;
    this->cursor(&x, &y);

    return this->clear(x, y);
}

bool fb::console::box(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    if(height < 3)
        return false;

    int rows = 0;
    this->line(x, y + rows++, width, '-', '+');
    while(rows < height - 1)
    {
        this->line(x, y + rows++, width, ' ', '+');
    }
    this->line(x, y + rows++, width, '-', '+');
    return true;
}

uint16_t fb::console::x() const
{
#ifdef _WIN32
    return this->_x;
#else
    uint16_t                x, y;
    this->cursor(&x, &y);
    return x;
#endif
}

uint16_t fb::console::y() const
{
#ifdef _WIN32
    return this->_y;
#else
    uint16_t                x, y;
    this->cursor(&x, &y);
    return y;
#endif
}

uint16_t fb::console::width() const
{
    return this->_width;
}

uint16_t fb::console::height() const
{
    return this->_height;
}

fb::console& fb::console::next()
{
#ifdef _WIN32
    this->_x = 0;
    this->_y++;
#else
    uint16_t                x, y;
    this->cursor(x, y+1);
#endif
    return *this;
}

fb::console& fb::console::cursor(uint16_t x, uint16_t y)
{
#ifdef _WIN32
    this->_x = x;
    this->_y = y;
#else
    ::move(y, x);
#endif

    return *this;
}

void fb::console::cursor(uint16_t* x, uint16_t* y) const
{
#ifdef _WIN32
    *x = this->_x;
    *y = this->_y;
#else
    getyx(stdscr, *y, *x);
#endif
}

fb::console& fb::console::get()
{
    if(_ist.get() == nullptr)
        _ist.reset(new console());

    return *_ist;
}