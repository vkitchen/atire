/*
	CANVAS.C
	--------
*/
#include <windows.h>
#include "resources/resource.h"
#include "canvas.h"
//#include "bitmapinfor256.h"
//#include "pallette.h"
#include "memory_file_line.h"
#include "memory_file_line_iterator.h"

#define VK_SCROLLLOCK (0x91)

/*
	ANT_CANVAS::ANT_CANVAS()
	------------------------
*/
ANT_canvas::ANT_canvas(HINSTANCE hInstance)
{
this->hInstance = hInstance;
file = new ANT_memory_file_line;
}

/*
	ANT_CANVAS::~ANT_CANVAS()
	-------------------------
*/
ANT_canvas::~ANT_canvas()
{
delete file;
}

/*
	ANT_CANVAS::LOAD_FILE()
	-----------------------
*/
long long ANT_canvas::load_file(void)
{
char chosen_filter[1024];
char chosen_filename[MAX_PATH];
OPENFILENAME parameters;

parameters.lStructSize = sizeof(parameters);
parameters.hwndOwner = window;
parameters.hInstance = hInstance;
parameters.lpstrFilter = "C/C++ files\0*.c;*.cxx;*.cpp\0\0\0";
parameters.lpstrCustomFilter = chosen_filter;
parameters.nMaxCustFilter = sizeof(chosen_filter) - 1;
parameters.nFilterIndex = 1;
parameters.lpstrFile = chosen_filename;
parameters.nMaxFile = sizeof(chosen_filename) - 1;
parameters.lpstrFileTitle = NULL;
parameters.nMaxFileTitle = 0;
parameters.lpstrInitialDir = NULL;
parameters.lpstrTitle = "Open...";
parameters.Flags = OFN_LONGNAMES;
parameters.nFileOffset = 0;
parameters.nFileExtension = 0;
parameters.lpstrDefExt = NULL;
parameters.lCustData = 0;
parameters.lpfnHook = NULL;
parameters.lpTemplateName = 0;

#if (_WIN32_WINNT >= 0x0500)
	parameters.pvReserved = NULL;
	parameters.dwReserved = 0;
	parameters.FlagsEx = 0;
#endif

if ((GetOpenFileName(&parameters)) != 0)
	return file->read_file(parameters.lpstrFile);
else
	return 0;
}

/*
	ANT_CANVAS::MENU()
	------------------
*/
void ANT_canvas::menu(WORD clicked)
{
SCROLLINFO scrollbar;
long long lines;

switch (clicked)
	{
	/*
		FILE MENU
	*/
	case ID_FILE_EXIT:
		PostQuitMessage(0);
		break;
	case ID_FILE_OPEN_FILE:
		lines = load_file();
		set_scroll_position(1, file->get_current_line_number(), file->get_page_size(), file->get_lines_in_file());
		break;
	}
}

/*
	ANT_CANVAS::SET_SCROLL_POSITION()
	---------------------------------
*/
long long ANT_canvas::set_scroll_position(long long zero, long long position, long long page_length, long long end_of_file)
{
SCROLLINFO scrollbar;

SetScrollRange(window, SB_VERT, zero, end_of_file, true);
scrollbar.cbSize = sizeof(scrollbar);
scrollbar.fMask = SIF_RANGE | SIF_TRACKPOS | SIF_PAGE | SIF_POS; 
scrollbar.nMin = zero;
scrollbar.nMax = end_of_file;
scrollbar.nPage = page_length;
scrollbar.nPos = position;
scrollbar.nTrackPos = 0;
SetScrollInfo(window, SB_VERT, &scrollbar, true);
InvalidateRect(window, NULL, true);

return position;
}

/*
	ANT_CANVAS::RENDER()
	--------------------
*/
void ANT_canvas::render(void)
{
HDC hDC;
PAINTSTRUCT paintStruct;
TEXTMETRIC text_metrics;
RECT window_size;
HGDIOBJ hFont;
long long vertical_position, vertical_spacing;
long long left_margin_gap = 5;	
long long bottom_of_window;
char *current_line;
ANT_memory_file_line_iterator iterator(file);

GetClientRect(window, &window_size);
bottom_of_window = window_size.bottom;

hFont = GetStockObject(OEM_FIXED_FONT);

hDC = BeginPaint(window, &paintStruct);
SelectObject(hDC, hFont);
//SetBkColor(hDC, RGB(0xFF, 0xFF, 0xFF));
SetTextColor(hDC, RGB(0x00, 0x00, 0x00));
GetTextMetrics(hDC, &text_metrics);
vertical_spacing = text_metrics.tmHeight + text_metrics.tmExternalLeading;

vertical_position = 0;
current_line = iterator.first();
if (current_line != NULL)
	while (vertical_position < bottom_of_window)
		{
		TextOut(hDC, left_margin_gap, vertical_position, current_line, strlen(current_line));
		vertical_position += vertical_spacing;
		if ((current_line = iterator.next()) == NULL)
			break;
		}

EndPaint(window, &paintStruct);
}


/*
	ANT_CANVAS::SET_PAGE_SIZE()
	---------------------------
*/
long long ANT_canvas::set_page_size(long long pixels)
{
TEXTMETRIC text_metrics;
HDC hDC = GetDC(window);

SelectObject(hDC, GetStockObject(OEM_FIXED_FONT));
GetTextMetrics(hDC, &text_metrics);
file->set_page_size( pixels / (text_metrics.tmHeight + text_metrics.tmExternalLeading));

ReleaseDC(window, hDC);
return 0;
}

/*
	ANT_CANVAS::WINDOWS_CALLBACK()
	------------------------------
*/
LRESULT ANT_canvas::windows_callback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
switch(message)
	{
	case WM_CREATE:
		set_page_size(((CREATESTRUCT *)lParam)->cy);
		return 0;

	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		return 0;

	case WM_SYSKEYUP:
	case WM_KEYUP:
		return 0;

	case WM_VSCROLL:
		{
		long long new_line, new_position;

		switch LOWORD(wParam)
			{
			case SB_LINEUP:
				new_line = file->line_up();
				break;
			case SB_LINEDOWN:
				new_line = file->line_down();
				break;
			case SB_PAGEUP:
				new_line = file->page_up();
				break;
			case SB_PAGEDOWN:
				new_line = file->page_down();
				break;
			case SB_BOTTOM:
				new_line = file->file_end();
				break;
			case SB_TOP:
				new_line = file->file_start();
				break;
			case SB_ENDSCROLL:			// user has finished scrolling, do nothing
				break;
			case SB_THUMBPOSITION:		// user is tracking the scroll bar
			case SB_THUMBTRACK:
				new_position = HIWORD(wParam);
				new_line = file->goto_line(new_position);
				break;
			}
		set_scroll_position(1, file->get_current_line_number(), file->get_page_size(), file->get_lines_in_file());
		InvalidateRect(window, NULL, true);
		return 0;
		}
	case WM_PAINT:
		render();
		return 0;

	case WM_TIMER:
		return 0;

	case WM_SIZE:
		set_page_size(HIWORD(lParam));
		return 0;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL)			// ESC key
			{
			}
		else
			menu(LOWORD(wParam));
		return 0;
	}
return (DefWindowProc(hwnd,message,wParam,lParam));
}

/*
	WINDOWS_CALLBACK()
	------------------
*/
static LRESULT CALLBACK windows_callback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
ANT_canvas *object = NULL;

if (message == WM_CREATE)
	{
	object = (ANT_canvas *)((CREATESTRUCT *)lParam)->lpCreateParams;
	object->window = hwnd;
	return object->windows_callback(hwnd, message, wParam, lParam);
	}
else if ((object = (ANT_canvas *)GetWindowLongPtr(hwnd, GWLP_USERDATA)) != NULL)
	return object->windows_callback(hwnd, message, wParam, lParam);
else
	return (DefWindowProc(hwnd,message,wParam,lParam));
}

/*
	ANT_CANVAS::CREATE_WINDOW()
	---------------------------
*/
long ANT_canvas::create_window(char *window_title)
{
HWND window;
WNDCLASSEX windowClass;

this->hInstance = hInstance;
windowClass.cbSize = sizeof(WNDCLASSEX);
windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
windowClass.lpfnWndProc = ::windows_callback;
windowClass.cbClsExtra = 0;
windowClass.cbWndExtra = 0;
windowClass.hInstance = 0;
windowClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_ATIRE_EDIT));
windowClass.hCursor = NULL;
windowClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
windowClass.lpszMenuName = NULL;
windowClass.lpszClassName = "ATIRE/Edit";
windowClass.hIconSm = NULL;

RegisterClassEx(&windowClass);

window = CreateWindowEx(NULL,			// extended style
	"ATIRE/Edit",					// class name
	window_title,					// window name
	WS_EX_OVERLAPPEDWINDOW | WS_SYSMENU | WS_VISIBLE | WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VSCROLL,
	120, 120,			// x/y coords
	WIDTH_IN_PIXELS,	// width
	2 * HEIGHT_IN_PIXELS,	// height
	NULL,				// handle to parent
	LoadMenu(0, MAKEINTRESOURCE(IDR_MENU)),				// Menu
	0,					// application instance
	this);				// no extra parameter's

SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)this);

UpdateWindow(window);

return 0;
}

