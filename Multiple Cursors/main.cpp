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

map<LPARAM,Point> cursorPositions;

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

	// create, show, update window
	HWND mainWindow = CreateWindow(TEXT("mainClass"),TEXT("Multiple Cursors"),WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,NULL,GetModuleHandle(NULL),NULL);
	ShowWindow(mainWindow,SW_SHOWNORMAL);
	UpdateWindow(mainWindow);

	// register for raw input
	RAWINPUTDEVICE devices[1];
	devices[0].usUsagePage = 1;
	devices[0].usUsage = 2;
	devices[0].dwFlags = RIDEV_DEVNOTIFY/*|RIDEV_INPUTSINK*/;
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
	char *data;
	RAWINPUT *input;
	Point *point;

	switch(message)
	{
	case WM_SETFOCUS:
		ShowCursor(FALSE);
		return 0;
	case WM_KILLFOCUS:
		ShowCursor(TRUE);
		return 0;
	case WM_INPUT_DEVICE_CHANGE:
		if(argW == GIDC_ARRIVAL)
		{
			if(cursorPositions.find(argL) == cursorPositions.end())	// if not already in
			{
				GetClientRect(window,&r);
				cursorPositions[argL] = Point(r.right/2,r.bottom/2);
			} // end if
			clog << "mouse connected with HANDLE = " << argL << endl;
		} // end if
		if(argW == GIDC_REMOVAL)
		{
			cursorPositions.erase(argL);	// erases it if it exists.
			clog << "mouse disconnected with HANDLE = " << argL << endl;
		} // end if
		InvalidateRect(window,NULL,TRUE);
		return 0;
	case WM_INPUT:
		GetRawInputData((HRAWINPUT)argL,RID_INPUT,NULL,&size,sizeof(RAWINPUTHEADER));
		data = new char[size];
		GetRawInputData((HRAWINPUT)argL,RID_INPUT,data,&size,sizeof(RAWINPUTHEADER));
		input = (RAWINPUT*)data;	// ...
		point = &cursorPositions[(LPARAM)input->header.hDevice];
		if((input->data.mouse.usFlags & 0x1) == MOUSE_MOVE_ABSOLUTE)
		{
			point->x = input->data.mouse.lLastX;
			point->y = input->data.mouse.lLastY;
		} // end if
		if((input->data.mouse.usFlags & 0x1) == MOUSE_MOVE_RELATIVE)
		{
			point->x += input->data.mouse.lLastX;
			point->y += input->data.mouse.lLastY;
		} // end if
		GetClientRect(window,&r);
		if(point->x < r.left)
			point->x = r.left;
		if(point->x > r.right)
			point->x = r.right;
		if(point->y < r.top)
			point->y = r.top;
		if(point->y > r.bottom)
			point->y = r.bottom;
		//clog << "input from mouse with HANDLE = " << (LPARAM)input->header.hDevice << " position = " 
		//	<< input->data.mouse.lLastX << ',' << input->data.mouse.lLastY << " flags = 0x" 
		//	<< std::hex << input->data.mouse.usFlags << std::dec << endl;
		delete[] data;
		InvalidateRect(window,NULL,TRUE);
		return 0;
	case WM_PAINT:
		{
		dc = BeginPaint(window,&ps);
		SelectObject(dc,CreatePen(PS_SOLID,1,RGB(255,192,0)));	// gold pen

		auto begin = cursorPositions.begin();
		auto end = cursorPositions.end();
		while(begin != end)
		{
			auto begin2 = begin;
			++begin2;
			while(begin2 != end)
			{
				MoveToEx(dc,begin->second.x,begin->second.y,NULL);
				LineTo(dc,begin2->second.x,begin2->second.y);
				begin2++;
			} // end while
			//Ellipse(dc,begin->second.x,begin->second.y,begin->second.x+20,begin->second.y+20);
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
