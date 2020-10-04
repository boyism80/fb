#include "console.h"

#ifdef _WIN32

console* console::_ist;

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

console::console() : 
    _stdout(GetStdHandle(STD_OUTPUT_HANDLE))
{
    // get size
    CONSOLE_SCREEN_BUFFER_INFO		screen;
    GetConsoleScreenBufferInfo(this->_stdout, &screen);
    this->_width = screen.dwSize.X;
    this->_height = screen.dwSize.Y;

    CONSOLE_CURSOR_INFO     cursor;
    GetConsoleCursorInfo(this->_stdout, &cursor);
    cursor.bVisible = false;
    SetConsoleCursorInfo(this->_stdout, &cursor);

}

console::~console()
{
}

bool console::line(uint16_t x, uint16_t y, uint16_t width, char content, char side)
{
    if(width < 3)
        return false;

    int  offset = 0;
    char buffer[256] = {0,};

    buffer[offset] = side;                      offset++;
    memset(buffer+offset, content, width-1);    offset += (width-1);
    buffer[offset] = side;

    this->puts(buffer, x, y);
    return true;
}

bool console::puts(const std::string& text, uint16_t x, uint16_t y)
{
    COORD					coord	= {(short)x, (short)y};
    DWORD					written;
    return WriteConsoleOutputCharacterA(this->_stdout, text.c_str(), text.length(), coord, &written) ? true : false;
}

bool console::clear(uint16_t row, uint16_t width)
{
    return false;
}

bool console::box(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
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

uint16_t console::width() const
{
    return this->_width;
}

uint16_t console::height() const
{
    return this->_height;
}

console& console::get()
{
    if(_ist == nullptr)
        _ist = new console();

    return *_ist;
}

void console::release()
{
    if(_ist != nullptr)
        delete _ist;
}

#endif