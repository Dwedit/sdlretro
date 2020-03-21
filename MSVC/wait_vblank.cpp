#include <Windows.h>

void win32_wait_vblank()
{
	HWND hwnd = 0;
	HDC dc;
	dc = GetDC(hwnd);
	GetPixel(dc, 0, 0);
	ReleaseDC(hwnd, dc);
}