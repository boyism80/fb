#include "console.h"

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

//bool SetConsoleTitle(const std::string& title)
//{
//	auto hwnd = ::GetConsoleWindow();
//	if(hwnd == nullptr)
//		return false;
//
//	::SetWindowTextA(hwnd, title.c_str());
//	return true;
//}