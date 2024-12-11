#include<windows.h>

LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);
//Special For Centering Window
int winWidth = 800, winHeight = 600, monWidth, monHeight;
int xPar = 100, yPar = 100;
MONITORINFO mi;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLne, int iCmdShow) {

	//Variable Declarations
	WNDCLASSEX wndclass;
	MSG msg;
	TCHAR szAppName[] = TEXT("MsgHand");
	HWND hwnd = NULL;

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = MyCallBack;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClassEx(&wndclass);
	//calling fun to get monitor details;
	//BOOL flag = FALSE;
//	POINT p = { 100,100 };

	hwnd = CreateWindow(szAppName,
		TEXT("Centring"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL);

	//ShowWindow(hwnd, iCmdShow);
	//UpdateWindow(hwnd);


	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return((int)msg.wParam);
}

LRESULT CALLBACK MyCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

	TCHAR str[] = TEXT("I'M Always Gonna Be At Center Of Your Screen!!");
	HDC hdc;
	RECT rc;
	PAINTSTRUCT ps;

	switch (iMsg) {

	case WM_CREATE:
		mi = { sizeof(MONITORINFO) };

		GetMonitorInfo(MonitorFromWindow(hwnd, MONITORINFOF_PRIMARY), &mi);

		//wsprintf(str, TEXT("Left : %ld \n Top : %ld\n Right : %ld\n Bottom : %ld"), mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right, mi.rcMonitor.bottom);
		//MessageBox(NULL, str, TEXT("MIAck"), MB_OK);


		monWidth = (mi.rcMonitor.right - mi.rcMonitor.left);
		monHeight = (mi.rcMonitor.bottom - mi.rcMonitor.top);

		//wsprintf(str, TEXT("Width : %ld \n Height : %ld"), monWidth, monHeight);
		//MessageBox(NULL, str, TEXT("MIAck"), MB_OK);

		xPar = (monWidth / 2) - (winWidth / 2);
		yPar = (monHeight / 2) - (winHeight / 2);

		//wsprintf(str, TEXT("Xpar : %ld \n YPar : %ld"), xPar, yPar);
		//MessageBox(NULL, str, TEXT("MIAck"), MB_OK);

		SetWindowPos(hwnd, HWND_TOP, xPar, yPar, winWidth, winHeight, NULL);
		break;

	case WM_PAINT:
		GetClientRect(hwnd, &rc);
		BeginPaint(hwnd, &ps);
		hdc = ps.hdc;

		SetBkColor(hdc, RGB(0,0,0));
		SetTextColor(hdc, RGB(0, 255, 0));

		DrawText(hdc, str, -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

		EndPaint(hwnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

