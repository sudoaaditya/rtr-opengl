
#include<windows.h>
#include<gl/GL.h>
#include<stdio.h>
#pragma comment(lib,"opengl32.lib")
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

//global Variables
WINDOWPLACEMENT wpPrev;
DWORD dwStyle;
HWND ghwnd = NULL;
bool gbFullScreen = false;
HDC ghdc;
HGLRC ghrc;
bool gbActivateWindow = false;
FILE *gfptr = NULL;

LRESULT CALLBACK MyCallback(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrvInstance, LPSTR ilszCmdLine, int iCmdShow) {

	//function declaration
	int initialize(void);
	void display(void);

	//variables
	HWND hwnd;
	WNDCLASSEX wndclass;
	MSG msg;
	TCHAR szApp[] = TEXT("BAsicPrimitive");
	int iRet = 0;
	bool bDone = false;

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpszClassName = szApp;
	wndclass.lpszMenuName = NULL;
	wndclass.lpfnWndProc = MyCallback;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClassEx(&wndclass);

	if (fopen_s(&gfptr, "PrimitiveLog.txt", "w+") != 0) {
		MessageBox(NULL, TEXT("Cannot Create Log"), TEXT("ErrMsg"), MB_OK);
		exit(0);
	}
	else {
		fprintf(gfptr, "Log Created Successfully!!\n\n");
	}

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szApp,
		TEXT("Lines Strip"),
		WS_OVERLAPPEDWINDOW |
		WS_CLIPCHILDREN |
		WS_CLIPSIBLINGS |
		WS_VISIBLE,
		100,
		100,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;

	iRet = initialize();
	if (iRet == -1) {
		fprintf(gfptr, "ChoosePixelFormat Failed!!..\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -2) {
		fprintf(gfptr, "SetPixelFormat Failed!!..\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -3) {
		fprintf(gfptr, "wglCreateContext Failed!!..\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -4) {
		fprintf(gfptr, "wglMakeCurrent Failed!!..\n");
		DestroyWindow(hwnd);
	}
	else {
		fprintf(gfptr, "Initialization Successful!!\n");
	}

	ShowWindow(hwnd,iCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	//game loop
	while (bDone == false) {

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

			if (msg.message == WM_QUIT) {
				bDone = true;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else {
			if (gbActivateWindow == true) {
				//here Update must called
			}
			display();
		}
	}

	return((int)msg.wParam);
}

LRESULT CALLBACK MyCallback(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

	//function declarartions
	void ToggleFuLLScreen(void);
	void resize(int, int);
	void display(void);
	void uninitialize(void);

	switch (iMsg) {
	case WM_SETFOCUS:
		gbActivateWindow = true;
		break;
	
	case WM_KILLFOCUS:
		gbActivateWindow = false;
		break;
	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_KEYDOWN:
		switch (wParam) {

		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		case 0x46:
			ToggleFuLLScreen();
			break;
		}
		break;

	case WM_ERASEBKGND:
		return(0);

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
		
	case WM_DESTROY:
		uninitialize();
		PostQuitMessage(0);
		break;

	default:
		DefWindowProc(hwnd, iMsg, wParam, lParam);
		break;
	}
	return(0);
}

void ToggleFuLLScreen(void) {

	//variables
	MONITORINFO mi;
	wpPrev = { sizeof(WINDOWPLACEMENT) };

	if (gbFullScreen == false) {

		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if (dwStyle&WS_OVERLAPPEDWINDOW) {
			
			mi = { sizeof(MONITORINFO) };

			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(
				MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi)) {

				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

				SetWindowPos(ghwnd,
					HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_FRAMECHANGED | SWP_NOZORDER);
			}
		}
		ShowCursor(FALSE);
		gbFullScreen = true;
	}
	else {

		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		
		SetWindowPlacement(ghwnd, &wpPrev);

		SetWindowPos(ghwnd,
			HWND_TOP,
			0, 0, 0, 0,
			SWP_NOZORDER | SWP_NOMOVE |
			SWP_FRAMECHANGED | SWP_NOSIZE |
			SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
		gbFullScreen = false;
	}
}

int initialize(void) {

	//variables decl
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;

	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	//structure bharane
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cBlueBits = 8;
	pfd.cGreenBits = 8;
	pfd.cAlphaBits = 8;

	ghdc = GetDC(ghwnd);

	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0){
		return(-1);
	}
	else {
		fprintf(gfptr, "ChoosePixelFormat Successful = %d\n", iPixelFormatIndex);
	}

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE) {
		return(-2);
	}
	else {
		fprintf(gfptr, "ChoosePixelFormat Successful!!\n");
	}

	ghrc = wglCreateContext(ghdc);	//bridging api to make superman's clothes
	if (ghrc == NULL) {
		return(-3);
	}
	else {
		fprintf(gfptr, "wglCreateContext Successful!!\n");
	}

	if (wglMakeCurrent(ghdc, ghrc) == FALSE) {
		return(-4);
	}
	else {
		fprintf(gfptr, "wglMakeCurrent Successful!!\n");
	}

	//set Buffer color to black.
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	return(0);

}

void resize(int width,int height) {

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

}

void display(void) {

	glClear(GL_COLOR_BUFFER_BIT);

	SwapBuffers(ghdc);
}

void uninitialize(void) {

	//if in fullscreen then it is said to be sesume window & the proceed.
	if (gbFullScreen == true) {

		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(ghwnd, &wpPrev);

		SetWindowPos(ghwnd,
			HWND_TOP,
			0, 0, 0, 0,
			SWP_NOZORDER | SWP_NOMOVE |
			SWP_FRAMECHANGED | SWP_NOSIZE |
			SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
	}

	if (wglGetCurrentContext() == ghrc) {
		wglMakeCurrent(NULL, NULL);
	}

	if (ghrc) {
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}

	if (ghdc) {
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (gfptr) {

		fprintf(gfptr, "\nUninitailization SuccessFul!!\n");
		fclose(gfptr);
		gfptr = NULL;
	}
}
