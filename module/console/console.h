#ifndef __CONSOLE_H__
#define	__CONSOLE_H__

#ifdef  _WIN32
#include <Windows.h>
#include <string>

#define	CONSOLE_TITLE	"Private kingdom of the wind - http://cshyeon.com"

bool SetConsoleIcon(int id);
#endif

class console
{
private:
    static console*         _ist;

private:
    HANDLE                  _stdout;
    uint16_t                _width, _height;

private:
    console();
    ~console();

public:
    bool                    puts(const std::string& text, uint16_t x, uint16_t y);
    bool                    clear(uint16_t row, uint16_t width);
    bool                    line(uint16_t x, uint16_t y, uint16_t width, char content, char side = '+');
    bool                    box(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    uint16_t                width() const;
    uint16_t                height() const;

public:
    static console&         get();
    static void             release();
};

#endif // !__CONSOLE_H__