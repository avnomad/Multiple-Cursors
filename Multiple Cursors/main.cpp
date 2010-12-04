#include <iostream>
using std::cout;
using std::cin;
using std::endl;
using std::cerr;
using std::clog;
using std::left;

#include <cstdlib>
using std::system;
using std::exit;

#include <map>
using std::map;

#include <windows.h>

#define escape(A) #A
#define length(A) (sizeof(A)/sizeof((A)[0]))

struct Point
{
	LONG x;
	LONG y;

	Point(){}
	Point(LONG X,LONG Y)
		:x(X),y(Y){}
}; // end struct Point

map<DWORD,Point> cursorPositions;

LRESULT CALLBACK windowProcedure(HWND window,UINT message,WPARAM argW,LPARAM argL);

int main()
{
	// register a window class
	WNDCLASS mainClass;

	mainClass.style = CS_HREDRAW | CS_VREDRAW;
	mainClass.lpfnWndProc = windowProcedure;
	mainClass.cbClsExtra = 0;
	mainClass.cbWndExtra = 0;
	mainClass.hInstance = GetModuleHandle(NULL);
	mainClass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	mainClass.hCursor = LoadCursor(NULL,IDC_ARROW);
	mainClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	mainClass.lpszMenuName = NULL;
	mainClass.lpszClassName = TEXT("mainClass");

	RegisterClass(&mainClass);

	// temp
	//Point p;
	//p.x = 100;
	//p.y = 100;
	//cursorPositions[0] = p;
	//p.x = 200;
	//p.y = 200;
	//cursorPositions[1] = p;

	// create, show, update window
	HWND mainWindow = CreateWindow(TEXT("mainClass"),TEXT("Multiple Cursors"),WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,NULL,GetModuleHandle(NULL),NULL);
	ShowWindow(mainWindow,SW_SHOWNORMAL);
	UpdateWindow(mainWindow);

	// register for raw input
	RAWINPUTDEVICE devices[1];
	devices[0].usUsagePage = 1;
	devices[0].usUsage = 2;
	devices[0].dwFlags = RIDEV_DEVNOTIFY|RIDEV_INPUTSINK;
	devices[0].hwndTarget = mainWindow;
	RegisterRawInputDevices(devices,length(devices),sizeof(devices[0]));

	// enter message loop
	MSG message;
	while(GetMessage(&message,NULL,0,0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	} // end while

	//system("pause");
	return message.wParam;
} // end function main

LRESULT CALLBACK windowProcedure(HWND window,UINT message,WPARAM argW,LPARAM argL)
{
	PAINTSTRUCT ps;
	HDC dc;
	RID_DEVICE_INFO deviceInfo;
	UINT size;
	RECT r;

	switch(message)
	{
	case WM_INPUT_DEVICE_CHANGE:	// maybe use handles instead of ids?
		if(argW == GIDC_ARRIVAL)
		{
			size = sizeof(deviceInfo);
			GetRawInputDeviceInfo((HANDLE)argL,RIDI_DEVICEINFO,&deviceInfo,&size);
			if(deviceInfo.dwType == RIM_TYPEMOUSE)
			{
				if(cursorPositions.find(deviceInfo.mouse.dwId) == cursorPositions.end())	// if not already in
				{
					GetClientRect(window,&r);
					cursorPositions[deviceInfo.mouse.dwId] = Point(r.right/2,r.bottom/2);
				} // end if
				clog << "mouse connected with ID = " << deviceInfo.mouse.dwId << endl;
			}
			else
				clog << "connected device is not a mouse!" << endl;
		} // end if
		if(argW == GIDC_REMOVAL)
		{
			size = sizeof(deviceInfo);
			GetRawInputDeviceInfo((HANDLE)argL,RIDI_DEVICEINFO,&deviceInfo,&size);
			if(deviceInfo.dwType == RIM_TYPEMOUSE)
			{
				cursorPositions.erase(deviceInfo.mouse.dwId);	// erases it if it exists.
				clog << "mouse disconnected with ID = " << deviceInfo.mouse.dwId << endl;
			}
			else
				clog << "disconnected device is not a mouse!" << endl;
		} // end if
		InvalidateRect(window,NULL,TRUE);
		return 0;
	case WM_PAINT:
		{
		dc = BeginPaint(window,&ps);
		SelectObject(dc,CreatePen(PS_SOLID,3,RGB(255,192,0)));	// gold pen

		auto begin = cursorPositions.begin();
		auto end = cursorPositions.end();
		while(begin != end)
		{
			Rectangle(dc,begin->second.x,begin->second.y,begin->second.x+20,begin->second.y+20);
			++begin;
		} // end while

		DeleteObject(SelectObject(dc,GetStockObject(WHITE_PEN)));	// delete gold pen
		EndPaint(window,&ps);
		return 0;
		}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	} // end switch
	return DefWindowProc(window,message,argW,argL);
} // end function windowProcedure
