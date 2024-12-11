#include<windows.h>
#include<stdio.h>
#include<gl/GL.h>
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#pragma comment(lib,"opengl32.lib")

//Global variables Declaration
HWND ghwnd = NULL;
DWORD dwStyle = NULL;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };
bool gbFullScreen = false;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
bool gbActivateWindow = false;
FILE *gfptr;

LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) {

	//function Declaration
	int initialize(void);
	void display(void);		//1st for DB

	//variable declartion
	bool bDone = false;
	int iRet;
	WNDCLASSEX wndclass;
	MSG msg;
	HWND hwnd;
	TCHAR szAppName[] = TEXT("DubBuffRendered");

	if (fopen_s(&gfptr, "DubLog.txt", "w") != 0) { //0 for success non-0 for failure

		MessageBox(NULL, TEXT("Cannot Create Log File!!.."), TEXT("GLErr"), MB_OK);
		exit(0);
	}
	else {
		fprintf(gfptr, "Log File Created Successfully!!..\n\n");
	}

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = MyCallBack;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClassEx(&wndclass);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("GL-DoubleBuffer"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
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
		fprintf(gfptr, "ChosePixelFormat Failed!!..\n");
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
		fprintf(gfptr, "Initialization Successful!!..\n");
	}

	ShowWindow(hwnd, iCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	//GAME LOOP
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
				//call to Update()
			}
			//call to Display in Double Buffering
			display();	//2nd for DB
		}
	}
	return((int)msg.wParam);
}

LRESULT CALLBACK MyCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

	//function declaration.
	void ToggleFullScreen(void);
	void resize(int, int);
	// No Declaration of Display	// 3rd for DB
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

		//no WM_PAINT instead will call to WM_ERASEBKGND //4th For DB
	case WM_ERASEBKGND:	//5th for DB
		return(0);

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_KEYDOWN:
		switch (wParam) {

		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		case 'F':
		case 'f':
			ToggleFullScreen();
			break;
		}
		break;

	case WM_DESTROY:
		uninitialize();
		PostQuitMessage(0);
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullScreen() {

	MONITORINFO mi = { sizeof(MONITORINFO) };

	if (gbFullScreen == false) {

		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if (dwStyle & WS_OVERLAPPEDWINDOW) {

			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi)) {

				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

				SetWindowPos(ghwnd,
					HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);
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
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
		gbFullScreen = false;
	}
}


int initialize(void) {

	//variables & Functions
	void resize(int, int);
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;

	memset((void*)&pfd, NULL, sizeof(PIXELFORMATDESCRIPTOR));

	//Sachaaa Bharnaaa
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; //6th for DB
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cBlueBits = 8;
	pfd.cGreenBits = 8;
	pfd.cAlphaBits = 8;



	ghdc = GetDC(ghwnd);

	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);

	if (iPixelFormatIndex == 0) {
		return(-1);
	}

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE) {
		return(-2);
	}

	ghrc = wglCreateContext(ghdc);
	if (ghdc == NULL) {
		return(-3);
	}

	if (wglMakeCurrent(ghdc, ghrc) == FALSE) {
		return(-4);
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//Warm Up To Resize
	resize(WIN_WIDTH, WIN_HEIGHT);
	return(0);
}

void resize(int width, int height) {

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

}

void display(void) {

	glClear(GL_COLOR_BUFFER_BIT);
	//code For Triangle
	glMatrixMode(GL_MODELVIEW);

	glBegin(GL_TRIANGLES);


	//appex with RED
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(0.0f, 1.0f);

	//left bottom with GREEN
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(-1.0f, -1.0f);

	//right bottom with Blue
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex2f(1.0f, -1.0f);

	glEnd();

	SwapBuffers(ghdc);	//7th For DB
}

void uninitialize(void) {

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
		fprintf(gfptr, "\nFile Closed Successfully!!..\n");
		fclose(gfptr);
		gfptr = NULL;
	}

}


