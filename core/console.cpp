#include <fb/console.h>

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

bool fb::console::line(uint16_t width, char content, char side)
{
    if(width < 3)
        return false;

    int  offset = 0;
    char buffer[256] = {0,};

    buffer[offset] = side;                          offset++;
    std::memset(buffer+offset, content, width-1);   offset += (width-1);
    buffer[offset] = side;

    uint16_t before_x, before_y;
    this->cursor(&before_x, &before_y);

    this->put(buffer);
    this->cursor(before_x, before_y);
    return true;
}

fb::console& fb::console::put(const char* fmt, ...)
{
    auto _ = std::lock_guard(this->_mutex);
    
    va_list                 args;

    va_start(args, fmt);
    auto                    combined = fb::format(fmt, &args);
    va_end(args);

#ifdef _WIN32
    DWORD                   written;
    auto width = WriteConsoleOutputCharacterA(this->_stdout, combined.c_str(), combined.length(), COORD {(SHORT)this->_x, (SHORT)this->_y}, &written) == 0 ? 0 : combined.length();
    this->clear(this->_x + width, this->_y);
#else
    auto width = combined.size();
    mvaddstr(this->_y, this->_x, combined.c_str());
    this->clear(this->_x + width, this->_y);
    refresh();
#endif
    return *this;
}

fb::console& fb::console::puts(const char* fmt, ...)
{
    auto _ = std::lock_guard(this->_mutex);
    
    va_list                 args;

    va_start(args, fmt);
    auto                    combined = fb::format(fmt, &args);
    va_end(args);

#ifdef _WIN32
    DWORD                   written;
    auto width = WriteConsoleOutputCharacterA(this->_stdout, combined.c_str(), combined.length(), COORD {(SHORT)this->_x, (SHORT)this->_y}, &written) == 0 ? 0 : combined.length();
    this->clear(this->_x + width, this->_y);
#else
    auto width = combined.size();
    mvaddstr(this->_y, this->_x, combined.c_str());
    this->clear(this->_x + width, this->_y);
    refresh();
#endif

    this->next();
    return *this;
}

fb::console& fb::console::render(const char* fmt, ...)
{
    auto _ = std::lock_guard(this->_mutex);
    
    va_list                 args;

    va_start(args, fmt);
    auto                    combined = fb::format(fmt, &args);
    va_end(args);

#ifdef _WIN32
    DWORD                   written;
    WriteConsoleOutputCharacterA(this->_stdout, combined.c_str(), combined.length(), COORD {(SHORT)this->_x, (SHORT)this->_y}, &written) == 0 ? 0 : combined.length();
#else
    mvaddstr(this->_y, this->_x, combined.c_str());
    refresh();
#endif

    return *this;
}

fb::console& fb::console::comment(const char* fmt, ...)
{
    auto _ = std::lock_guard(this->_mutex);
    
    va_list                 args;

    va_start(args, fmt);
    auto                    combined = fb::format(fmt, &args);
    va_end(args);

    auto y = this->_y + (++this->_additional_y);

#ifdef _WIN32
    DWORD                   written;
    auto width = WriteConsoleOutputCharacterA(this->_stdout, combined.c_str(), combined.length(), COORD {(SHORT)this->_x, (SHORT)y}, &written) == 0 ? 0 : combined.length();
    this->clear(width, y);
#else
    auto width = combined.size();
    mvaddstr(y, this->_x, combined.c_str());
    this->clear(this->_x + width, y);
    refresh();
#endif

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

bool fb::console::box(uint16_t width, uint16_t height)
{
    if(height < 3)
        return false;

    auto rows = 0;
    auto pivot = this->y();
    this->y(pivot + rows++);  this->line(width, '-', '+');
    while(rows < height - 1)
    {
        this->y(pivot + rows++); this->line(width, ' ', '+');
    }
    this->y(pivot + rows++); this->line(width, '-', '+');

    this->y(pivot);
    return true;
}

uint16_t fb::console::x() const
{
    return this->_x;
}

void fb::console::x(uint16_t val)
{
    this->cursor(val, this->y());
}

uint16_t fb::console::y() const
{
    return this->_y;
}

void fb::console::y(uint16_t val)
{
    this->cursor(this->x(), val);
}

void fb::console::reset_x()
{
    this->x(0);
}

void fb::console::reset_y()
{
    this->y(0);
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
    this->_x = 0;
    this->_y += (this->_additional_y + 1);
    this->_additional_y = 0;
    return *this;
}

fb::console& fb::console::cursor(uint16_t x, uint16_t y)
{
    this->_x = x;
    this->_y = y;
    return *this;
}

void fb::console::cursor(uint16_t* x, uint16_t* y) const
{
    *x = this->_x;
    *y = this->_y;
}

fb::console& fb::console::get()
{
    static std::unique_ptr<console> ist;
    static std::once_flag           flag;
    std::call_once(flag, [] { ist.reset(new console()); });
    return *ist;
}