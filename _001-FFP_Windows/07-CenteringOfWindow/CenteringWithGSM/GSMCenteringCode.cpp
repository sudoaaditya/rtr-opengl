#include<windows.h>

LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszScmdLine, int iCmdShow) {

	WNDCLASSEX wndclass;
	MSG msg;
	HWND hwnd;
	TCHAR szAppName[] = TEXT("GSM-Centering"), str[255];
	int monWidth, monHeight;
	int winWidth = 800, winHeight = 600;
	int xPar = 0;
	int yPar = 0;
	
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

	monWidth = GetSystemMetrics(SM_CXMAXIMIZED);
	monHeight = GetSystemMetrics(SM_CYMAXIMIZED);
	/*
	wsprintf(str, TEXT("Width : %d\nHeight : %d\n"), monWidth, monHeight);
	MessageBox(NULL, str, TEXT("AckHW"), MB_OK);
	*/

	xPar = (monWidth / 2) - (winWidth / 2);
	yPar = (monHeight / 2) - (winHeight / 2);

	hwnd = CreateWindow(szAppName,
		TEXT("Toggle FullScreen"),
		WS_OVERLAPPEDWINDOW,
		xPar,
		yPar,
		800,
		600,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0)) {

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return((int)msg.wParam);


}

LRESULT CALLBACK MyCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

	void ToggleFullScreen(void);

	switch (iMsg) {

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}
