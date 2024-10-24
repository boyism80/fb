#include <fb/console.h>

#ifdef _WIN32
bool SetConsoleIcon(int id)
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

bool fb::console::line(uint16_t width, char content, char side)
{
    if (width < 3)
        return false;

    int  offset      = 0;
    char buffer[256] = {
        0,
    };

    buffer[offset] = side;
    offset++;
    std::memset(buffer + offset, content, width - 1);
    offset += (width - 1);
    buffer[offset] = side;

    uint16_t before_x, before_y;
    this->cursor(&before_x, &before_y);

    this->put(buffer);
    this->cursor(before_x, before_y);
    return true;
}

fb::console& fb::console::clear(uint16_t x, uint16_t y)
{
    Term::cout << Term::cursor_move(y, x - 1) << std::string(this->width() - x + 1, ' ') << std::flush;
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
    if (height < 3)
        return false;

    auto rows  = 0;
    auto pivot = this->y();
    this->y(pivot + rows++);
    this->line(width, '-', '+');
    while (rows < height - 1)
    {
        this->y(pivot + rows++);
        this->line(width, ' ', '+');
    }
    this->y(pivot + rows++);
    this->line(width, '-', '+');

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
    auto size = Term::screen_size();
    return size.columns();
}

uint16_t fb::console::height() const
{
    auto size = Term::screen_size();
    return size.rows();
}

fb::console& fb::console::next()
{
    this->cursor(0, this->_y + this->_additional_y + 1);
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
    std::call_once(flag, [] {
        ist.reset(new console());
    });
    return *ist;
}