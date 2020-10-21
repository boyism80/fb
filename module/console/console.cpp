#include "console.h"

fb::console* fb::console::_ist;

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

fb::console::console() : _x(0), _y(0)
{
#ifdef _WIN32
    this->_stdout = GetStdHandle(STD_OUTPUT_HANDLE);

    // get size
    CONSOLE_SCREEN_BUFFER_INFO		screen;
    GetConsoleScreenBufferInfo(this->_stdout, &screen);
    this->_width = screen.dwSize.X;
    this->_height = screen.dwSize.Y;

    CONSOLE_CURSOR_INFO     cursor;
    GetConsoleCursorInfo(this->_stdout, &cursor);
    cursor.bVisible = false;
    SetConsoleCursorInfo(this->_stdout, &cursor);
#else
    initscr();
    getmaxyx(stdscr, this->_height, this->_width);
    noecho();
#endif
}

fb::console::~console()
{
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

    this->move(x, y).puts(buffer);
    return true;
}

fb::console& fb::console::move(uint16_t x, uint16_t y)
{
    this->_x = x;
    this->_y = y;
    return *this;
}

uint32_t fb::console::puts(const char* format, ...)
{
    va_list					args;

    va_start(args, format);
    auto                    combined = this->format(format, &args);
    va_end(args);

    if(this->_x.has_value() == false)
        this->_x = 0;

    if(this->_y.has_value() == false)
        this->_y = this->_current_line++;

#ifdef _WIN32
    COORD					coord	= {(short)this->_x.value(), (short)this->_y.value()};
    DWORD					written;
    auto                    result = WriteConsoleOutputCharacterA(this->_stdout, combined.c_str(), combined.length(), coord, &written) == 0 ? 0 : combined.length();

#else
    mvprintw(this->_y.value(), this->_x.value(), CP949(combined).c_str());
    refresh();

    auto                    result = combined.length();
#endif

    this->_x.reset();
    this->_y.reset();

    return result;
}

bool fb::console::clear(uint16_t x, uint16_t y)
{
    static int prev = 0;
    static char buffer[256];

    if(prev > x)
    {
        auto len = this->width() - x;
        std::memset(buffer, ' ', len);
        buffer[len] = 0;
        this->move(x, y).puts(buffer);
    }

    prev = x;
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

uint16_t fb::console::width() const
{
    return this->_width;
}

uint16_t fb::console::height() const
{
    return this->_height;
}

uint32_t fb::console::current_line() const
{
    return this->_current_line;
}

fb::console& fb::console::current_line(uint32_t row)
{
    this->_current_line = row;
    return *this;
}

fb::console& fb::console::next()
{
    this->_current_line++;
    return *this;
}

fb::console& fb::console::get()
{
    if(_ist == nullptr)
        _ist = new console();

    return *_ist;
}

void fb::console::release()
{
    if(_ist != nullptr)
        delete _ist;

#ifdef __linux__
    endwin();
#endif
}