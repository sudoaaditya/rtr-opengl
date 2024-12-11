#include<windows.h>
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);
void OptFullScreen(void);
//GlobalVariables For Full Screen
//DEVMODE dMode = { sizeof(DEVMODE) };
bool bIsFullScreen = false;
HWND gHwnd = NULL;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) {

	WNDCLASSEX wndclass;
	MSG msg;
	HWND hwnd;
	TCHAR szAppName[] = TEXT("TFS");

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = MyCallBack;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClassEx(&wndclass);

	hwnd = CreateWindow(szAppName,
		TEXT("CDS-FullScreen"),
		WS_OVERLAPPEDWINDOW,
		100,
		100,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	gHwnd = hwnd;

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0)) {

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return((int)msg.wParam);
}

LRESULT CALLBACK MyCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

	
	
	switch (iMsg) {

	case WM_KEYDOWN:
		switch (wParam) {

		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		case 'F':
		case 'f':
			OptFullScreen();
			break;

		}
		break;
		
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void OptFullScreen(void) {
	TCHAR str[255];
	
	/*	if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dMode)) {


		//wsprintf(str, TEXT("dmPelsWidth : %d\ndmPelsHeight : %d\ndmDisplayFlags : %d\n"), 
		//dMode.dmPelsWidth, dMode.dmPelsHeight, dMode.dmDisplayFlags);
		MessageBox(gHwnd, str, NULL, MB_OK);
		}
		
	if (ChangeDisplaySettings(&dMode, CDS_FULLSCREEN | CDS_UPDATEREGISTRY  | CDS_GLOBAL ) == DISP_CHANGE_SUCCESSFUL) {
		MessageBox(gHwnd, TEXT("HERE"), TEXT("ack"), MB_OK);
	}
	*/
	DEVMODE dMode;
	dMode.dmSize = sizeof(DEVMODE);

	EnumDisplaySettings(NULL, 0, &dMode);
	if (bIsFullScreen == false)
	{
		ChangeDisplaySettings(&dMode, CDS_FULLSCREEN);
		bIsFullScreen = true;
	}
	else {
		ChangeDisplaySettings(&dMode, CDS_RESET);
		bIsFullScreen = false;

	}
}

                  