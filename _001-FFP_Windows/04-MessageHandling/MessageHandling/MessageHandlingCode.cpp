#include<windows.h>

LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLne, int iCmdShow) {

	//Variable Declarations
	WNDCLASSEX wndclass;
	MSG msg;
	TCHAR szAppName[] = TEXT("MsgHand");
	HWND hwnd;

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = MyCallBack;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClassEx(&wndclass);

	hwnd = CreateWindow(szAppName,
		TEXT("Message Handling"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
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

	switch (iMsg) {

	case WM_CREATE:
		MessageBox(hwnd, TEXT("WM_CREATE is Called"), TEXT("CreateMsg"), MB_OK);
		break;

	case WM_KEYDOWN:
		switch (wParam) {

		case VK_ESCAPE:
			MessageBox(hwnd, TEXT("VK_ESCAPE is Pressed"), TEXT("EscMsg"), MB_OK);
			DestroyWindow(hwnd);
			break;

		case 0x46:
			MessageBox(hwnd, TEXT("F is Pressed"), TEXT("FMsg"), MB_OK);
			break;
		}
		break;

	case WM_LBUTTONDOWN:
		MessageBox(hwnd, TEXT("Left Mouse Buttion is Clicked"), TEXT("LBMsg"), MB_OK);
		break;

	case WM_RBUTTONDOWN:
		MessageBox(hwnd, TEXT("Right Mouse Button is Clicked"), TEXT("RBMsg"), MB_OK);
		break;

	case WM_DESTROY:
		MessageBox(hwnd, TEXT("WM_Destroy is called"), TEXT("DestroyMsg"), MB_OK);
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}